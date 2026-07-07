#pragma once

#include <prepro/AST.hpp>
#include <string>
#include <string_view>
#include <vector>

class Parser {
private:
  std::string_view input;
  size_t position = 0;
  
  bool is_eof() const { return position >= input.length(); }
 
  /* получение следующего символа */
  bool get_symbol(char &symbol) {
    if (is_eof())
      return false;
    symbol = input[position];
    position++;
    return true;
  }
  /* сравнение строк с текущей позиции */
  bool match(std::string_view substr) const {
    if (position + substr.length() > input.length()) {
      return false;
    }
    return input.substr(position, substr.length()) == substr;
  }
  /* сравнение строк с текущей позиции и смещение позиции */
  bool match_and_consume(std::string_view substr) {
    if (match(substr)) {
      position += substr.length();
      return true;
    }
    return false;
  }
  /* пропуск пробелов и табуляций */
  void skip_spaces() {
    while (!is_eof() && (input[position] == ' ' || input[position] == '\t'))
      position++;
  }
  void parse_directive(std::vector<ASTNode> &ast);
  void parse_define(std::vector<ASTNode> &ast);
  void parse_defblock(std::vector<ASTNode> &ast);
  void parse_block(std::vector<ASTNode> &ast);
  void parse_scope(std::vector<ASTNode> &ast);

public:
  Parser(std::string_view text) : input(text) {}
  std::vector<ASTNode> parse();
};