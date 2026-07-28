#include <prepro/ast.hpp>
#include <prepro/preprocessor.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>

namespace prepro {

void Environment::define(const std::string_view &id, const std::string &value) {
  if (defines_.contains(id)) {
    error("duplicate define '" + std::string(id) + "'");
  }
  defines_.emplace(id, value);
}

void Environment::define_block(const DefBlockNode &block) {
  if (defblocks_.contains(block.id_)) {
    error("duplicate defblock '" + std::string(block.id_) + "'");
  }
  defblocks_.emplace(block.id_, &block);
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

std::string Preprocessor::process(const ScopeNode &root) const {
  Environment global;
  std::string res;
  process_macro(root.macro_, global, res);
  return res;
}

void Preprocessor::process_scope(const ScopeNode &scope, const Environment &env, std::string &out) const {
  Environment local(env); 
  process_macro(scope.macro_, local, out);
}

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
    case NodeType::DEFINE: {
      const auto &define = static_cast<const DefineNode &>(node);
      env.define(define.id_, std::string(define.value_));
      return;
    }
    case NodeType::DEF_BLOCK: {
      const auto &block = static_cast<const DefBlockNode &>(node);
      env.define_block(block);
      return;
    }
    case NodeType::BLOCK: {
      const auto &block = static_cast<const BlockNode &>(node);
      process_block(block, env, out);
      return;
    }
  }
}

void Preprocessor::process_block(const BlockNode &block, Environment &env, std::string &out) const {
  const DefBlockNode *rule = env.find_block(block.id_);
  if (!rule) {
    error("unknown block '" + std::string(block.id_) + "'");
  }
  if (block.args_.size() != rule->args_.size()) {
    error("wrong number of arguments for block '" + std::string(block.id_) + "'");
  }
  Environment local(env);

  for (size_t i = 0; i < block.args_.size(); ++i) {
    std::string value;
    process_scope(static_cast<const ScopeNode &>(*block.args_[i]), env, value);
    local.define(rule->args_[i], value);
  }
  process_macro(rule->macro_, local, out);
}

[[noreturn]]
void Preprocessor::error(const std::string_view &message) const {
  throw std::runtime_error(std::string(message));
}

[[noreturn]]
void Environment::error(const std::string_view &message) const {
  throw std::runtime_error(std::string(message));
}

}