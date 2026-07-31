#include <prepro/lexer.hpp>
#include <prepro/parser.hpp>
#include <prepro/ast.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <cstddef>
#include <iterator>
#include <exception>

namespace {

std::string escape(std::string_view lexeme) {
  std::string res;

  for (char c : lexeme) {
    switch (c) {
      case '\n': res += "\\n"; break;
      case '\r': res += "\\r"; break;
      case '\t': res += "\\t"; break;
      default:   res += c;     break;
    }
  }
  return res;
}

void dump_ast(const prepro::ASTNode *node, int indent = 0) {
  if (!node) {
    return;
  }
  std::string indent_str(indent, ' ');

  switch (node->type_) {
    case prepro::NodeType::TEXT: {
      auto text_node = static_cast<const prepro::TextNode*>(node);
      std::cout << indent_str << "TEXT  \"" << escape(text_node->text_) << "\"";
      break;
    }
    case prepro::NodeType::ID: {
      auto id_node = static_cast<const prepro::IdNode*>(node);
      std::cout << indent_str << "ID    " << id_node->id_;
      break;
    }
    case prepro::NodeType::SCOPE: {
      auto scope_node = static_cast<const prepro::ScopeNode*>(node);
      std::cout << indent_str << "SCOPE {";

      for (const auto &child : scope_node->macro_) {
        std::cout << "\n";
        dump_ast(child.get(), indent + 2);
      }
      std::cout << "\n" << indent_str << "}";
      break;
    }
    case prepro::NodeType::DEF_BLOCK: {
      auto def_block_node = static_cast<const prepro::DefBlockNode*>(node);
      std::cout << indent_str << "DEFBLOCK " << def_block_node->id_ << "(";

      for (size_t i = 0; i < def_block_node->args_.size(); ++i) {
        if (i) {
          std::cout << ", ";
        }
        std::cout << def_block_node->args_[i];
      }
      std::cout << ") {";

      for (const auto &child : def_block_node->macro_) {
        std::cout << "\n";
        dump_ast(child.get(), indent + 2);
      }
      std::cout << "\n" << indent_str << "}";
      break;
    }
    case prepro::NodeType::BLOCK: {
      auto block_node = static_cast<const prepro::BlockNode*>(node);
      std::cout << indent_str << "BLOCK " << block_node->id_ << " {";

      for (size_t i = 0; i < block_node->args_.size(); ++i) {
        std::cout << "\n" << indent_str << "  ARG " << i << ":\n";
        dump_ast(block_node->args_[i].get(), indent + 4);
      }
      std::cout << "\n" << indent_str << "}";
      break;
    }
    case prepro::NodeType::DEFINE: {
      auto define_node = static_cast<const prepro::DefineNode*>(node);
      std::cout << indent_str << "DEFINE " << define_node->id_ << " = \"" << escape(define_node->value_) << "\"";
      break;
    }
    default:
      std::cout << indent_str << "НЕИЗВЕСТНЫЙ_УЗЕЛ";
      break;
  }
}

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Использование: parsedump <файл>\n";
    return 1;
  } 
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "Не удалось открыть файл\n";
    return 1;
  }
  std::string src{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
 
  try {
    prepro::Lexer lexer(src);
    prepro::Parser parser(lexer.tokenize());
    std::unique_ptr<prepro::ScopeNode> root = parser.parse();

    dump_ast(root.get());
    std::cout << "\n";
  } 
  catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}