#include <prepro/lexer.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <iterator>

namespace {

std::string escape(const std::string_view &lexeme) {
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
    std::cerr << "Использование: lexdump <файл>\n";
    return 1;
  } 
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "Не удалось открыть файл\n";
    return 1;
  }
  std::string src{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
 
  prepro::Lexer lexer(src);
  std::vector<prepro::Token> tokens = lexer.tokenize();

  for (const auto &token : tokens) {
    std::cout << token.line_           << ':'
              << token.column_         << "  "
              << token.type_           << "  \""
              << escape(token.lexeme_) << "\"\n";
  }
}