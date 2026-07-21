#include <prepro/lexer.hpp>
#include <prepro/parser.hpp>
#include <prepro/ast.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

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
  if (!node) return;

  std::string indent_str(indent, ' ');

  switch (node->type_) {
    case prepro::NodeType::TEXT: {
      auto text_node = static_cast<const prepro::TextNode*>(node);
      std::cout << indent_str << "TEXT  \"" << escape(text_node->text_) << "\"\n";
      break;
    }
    case prepro::NodeType::ID: {
      auto id_node = static_cast<const prepro::IdNode*>(node);
      std::cout << indent_str << "ID    " << id_node->id_ << "\n";
      break;
    }
    case prepro::NodeType::SCOPE: {
      auto scope_node = static_cast<const prepro::ScopeNode*>(node);
      std::cout << indent_str << "SCOPE (\n";
      
      for (const auto& child : scope_node->macro_) {
        dump_ast(child.get(), indent + 2);
      }
      std::cout << indent_str << ")\n";
      break;
    }
    case prepro::NodeType::DEFINE: {
      auto define_node = static_cast<const prepro::DefineNode*>(node);
      std::cout << indent_str << "DEFINE " << define_node->id_ 
                << " = \"" << escape(define_node->value_) << "\"\n";
      break;
    }
    default:
      std::cout << indent_str << "UNKNOWN_NODE\n";
      break;
  }
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: parsedump <file>\n";
    return 1;
  } 
  std::ifstream file(argv[1]);

  if (!file) {
    std::cerr << "Cannot open file\n";
    return 1;
  }
  std::string src{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
 
  try {
    prepro::Lexer lexer(src);
    std::vector<prepro::Token> tokens = lexer.tokenize();

    prepro::Parser parser(tokens);
    std::unique_ptr<prepro::ScopeNode> root = parser.parse();

    dump_ast(root.get());

  } 
  catch (const std::exception& e) {
    std::cerr << "Error during execution: " << e.what() << "\n";
    return 1;
  }
  return 0;
}