#include <prepro/lexer.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

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

}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: lexdump <file>\n";
    return 1;
  } 
  std::ifstream file(argv[1]);

  if (!file) {
    std::cerr << "Cannot open file\n";
    return 1;
  }
  std::string src{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
 
  prepro::Lexer lexer(src);
  std::vector<prepro::Token> tokens = lexer.tokenize();

  for (const auto &token : tokens) {
    std::cout << token.line           << ':'
              << token.column         << "  "
              << token.type           << "  \""
              << escape(token.lexeme) << "\"\n";
  }
}