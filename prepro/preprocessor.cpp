#include <prepro/ast.hpp>
#include <prepro/preprocessor.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>
#include <cstddef>
#include <stdexcept>

namespace prepro {

void Environment::define(const std::string_view &id, const std::string &value) {
  if (defines_.contains(id)) {
    error("повторное определение define '" + std::string(id) + "'");
  }
  defines_.emplace(id, value);
}

void Environment::define_block(const DefBlockNode &block) {
  if (defblocks_.contains(block.id_)) {
    error("повторное определение defblock '" + std::string(block.id_) + "'");
  }
  defblocks_.emplace(block.id_, &block);
}

void Environment::reg(const std::string &reg) {
  for (const auto &r : regs_) {
    if (r == reg) {
      error("повторное определение регистра '" + reg + "'");
    }
  }
  regs_.push_back(reg);
}

const std::string *Environment::find_define(const std::string_view &id) const {
  auto it = defines_.find(id);
  if (it != defines_.end()) {
    return &it->second;
  }
  if (parent_) {
    return parent_->find_define(id);
  }
  return nullptr;
}

const DefBlockNode *Environment::find_block(const std::string_view &id) const {
  auto it = defblocks_.find(id);
  if (it != defblocks_.end()) {
    return it->second;
  }
  if (parent_) {
    return parent_->find_block(id);
  }
  return nullptr;
}

// пока возвращаем первый свободный регистр
std::optional<std::string> Environment::take_reg() {
  if (regs_.empty()) {
    return std::nullopt;
  }
  std::string reg = std::move(regs_.front());
  regs_.erase(regs_.begin());
  return reg;
}

std::string Preprocessor::process(const ScopeNode &root) const {
  Environment global;
  std::string res;
  process_macro(root.macro_, global, res);
  return res;
}

// дочернее окружение - создается
void Preprocessor::process_scope(const ScopeNode &scope, const Environment &env, std::string &out) const {
  Environment local(env); 
  process_macro(scope.macro_, local, out);
}

// дочернее окружение - не создается
void Preprocessor::process_macro(const std::vector<std::unique_ptr<ASTNode>> &macro, Environment &env, std::string &out) const {
  for (const auto &node : macro) {
    process_node(*node, env, out);
  }
}

void Preprocessor::process_node(const ASTNode &node, Environment &env, std::string &out) const {
  switch (node.type_) {
    case NodeType::TEXT: {
      const auto &text = static_cast<const TextNode &>(node);
      out += text.text_;
      return;
    }
    case NodeType::ID: {
      const auto &id = static_cast<const IdNode &>(node);
      if (const auto *value = env.find_define(id.id_)) {
        out += *value;
        return;
      }
      out += id.id_;
      return;
    }
    case NodeType::SCOPE: {
      const auto &scope = static_cast<const ScopeNode &>(node);
      process_scope(scope, env, out);
      return;
    }
    case NodeType::DEF_BLOCK: {
      const auto &block = static_cast<const DefBlockNode &>(node);
      try {
        env.define_block(block);
      } catch (const std::runtime_error &e) {
        error(block, e.what());
      }
      return;
    }
    case NodeType::BLOCK: {
      const auto &block = static_cast<const BlockNode &>(node);
      process_block(block, env, out);
      return;
    }
    case NodeType::DEFINE: {
      const auto &define = static_cast<const DefineNode &>(node);
      try {
        env.define(define.id_, std::string(define.value_));
      } catch (const std::runtime_error &e) {
        error(define, e.what());
      }
      return;
    }
    case NodeType::REG: {
      const auto &reg = static_cast<const RegNode &>(node);
      process_reg(reg, env);
      return;
    }
    case NodeType::REGS: {
      const auto &regs = static_cast<const RegsNode &>(node);
      process_regs(regs, env);
      return;
    }
  }
}

void Preprocessor::process_block(const BlockNode &block, Environment &env, std::string &out) const {
  const DefBlockNode *rule = env.find_block(block.id_);
  if (!rule) {
    error(block, "неизвестный макроблок '" + std::string(block.id_) + "'");
  }
  if (block.args_.size() != rule->args_.size()) {
    error(block, "неверное количество аргументов для макроблока '" + std::string(block.id_) + "'");
  }
  Environment local(env);

  for (size_t i = 0; i < block.args_.size(); ++i) {
    std::string value;
    if (block.args_[i]) {
      process_scope(*block.args_[i], env, value);
    }
    local.define(rule->args_[i], value);
  }
  process_macro(rule->macro_, local, out);
}

void Preprocessor::process_regs(const RegsNode &regs, Environment &env) const {
  for (const auto &range : regs.ranges_) {
    if (!range.last_) {
      try {
        env.reg(std::string(range.first_));
      } catch (const std::runtime_error &e) {
        error(regs, e.what());
      }
      continue;
    }
    std::string reg(range.first_);

    size_t pos = reg.size();
    while (pos > 0 && std::isdigit(reg[pos - 1])) {
      --pos;
    }
    if (pos == reg.size()) {
      error(regs, "у диапазона регистров отсутствует младший номер");
    }
    std::string prefix = reg.substr(0, pos);
    size_t first = std::stoul(reg.substr(pos));

    for (size_t i = first; i <= *range.last_; ++i) {
      try {
        env.reg(prefix + std::to_string(i));
      } catch (const std::runtime_error &e) {
        error(regs, e.what());
      }
    }
  }
}

void Preprocessor::process_reg(const RegNode &reg, Environment &env) const {
  for (const auto &binding : reg.bindings_) {
    if (binding.reg_) {
      const std::string_view &value = *binding.reg_;

      // Полный регистр 
      if (!value.empty() && std::isdigit(static_cast<unsigned char>(value.back()))) {
        env.define(binding.id_, std::string(value));
        continue;
      }
    }
    // первый свободный регистр
    auto value = env.take_reg();
    if (!value) {
      error(reg, "не осталось свободных регистров");
    }
    env.define(binding.id_, *value);
  }
}

[[noreturn]]
void Preprocessor::error(const ASTNode &node, const std::string_view &message) const {
  throw std::runtime_error(
    "Ошибка препроцессора на " +
    std::to_string(node.line_) +
    ":" +
    std::to_string(node.column_) +
    ": " +
    std::string(message)
  );
}

[[noreturn]]
void Environment::error(const std::string_view &message) const {
  throw std::runtime_error(std::string(message));
}

}