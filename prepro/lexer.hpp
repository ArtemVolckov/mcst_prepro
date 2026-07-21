#pragma once

#include <vector>
#include <string_view>
#include <ostream>

namespace prepro {

enum class TokenType {
  ID,              // C id         
  TEXT,            // остальной текст 

  DIRECTIVE_OPEN,  // <:              
  DIRECTIVE_CLOSE, // :>              
  LPAREN,          // (               
  RPAREN,          // )               
  COMMA,           // ,
  NEWLINE,         // \n
  SPACE            // ' ', \t, \r
};

struct Token {
  TokenType type;
  std::string_view lexeme;
  size_t line;
  size_t column;
};

class Lexer {
  std::string_view src_;
  size_t pos_    = 0;
  size_t line_   = 1;
  size_t column_ = 1;

  bool is_eof() const noexcept { return pos_ >= src_.length(); }

  char peek() const noexcept;
  void advance() noexcept;

  bool check(std::string_view str) const noexcept;
  bool match(char c) noexcept;
  bool match(std::string_view str) noexcept;

  static constexpr bool is_space_char(char c) noexcept {
    return (c == ' ' || c == '\t' || c == '\r');
  }
  static constexpr bool is_id_start(char c) noexcept {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
  }
  static constexpr bool is_id_body(char c) noexcept {
    return (is_id_start(c) || (c >= '0' && c <= '9') || c == '_');
  }

  Token next();

public:
  Lexer(std::string_view src) : src_(src) {}
  std::vector<Token> tokenize();
};

std::ostream &operator<<(std::ostream &os, TokenType type);

}