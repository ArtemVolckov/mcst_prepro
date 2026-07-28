#include <prepro/lexer.hpp>
#include <prepro/parser.hpp>
#include <prepro/ast.hpp>
#include <prepro/preprocessor.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: pp <file>\n";
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
    prepro::Parser parser(lexer.tokenize());
    std::unique_ptr<prepro::ScopeNode> root = parser.parse();
    prepro::Preprocessor preprocessor;
    std::cout << preprocessor.process(*root);
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}