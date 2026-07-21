#include <prepro/lexer.hpp>

#include <vector>
#include <string_view>
#include <ostream>

namespace prepro {

char Lexer::peek() const noexcept {
  if (is_eof()) return '\0';
  return src_[pos_]; 
}

void Lexer::advance() noexcept {
  if (is_eof()) return;
  if (src_[pos_] == '\n') {
    ++line_;
    column_ = 1;
  } else ++column_;
  ++pos_;
}

bool Lexer::check(std::string_view str) const noexcept {
  if (pos_ + str.length() > src_.length()) return false;
  return (str == src_.substr(pos_, str.length()));
}

bool Lexer::match(char c) noexcept {
  if (is_eof()) return false;
  if (src_[pos_] != c) return false;
  advance();
  return true;
}

bool Lexer::match(std::string_view str) noexcept {
  if (!check(str)) return false;

  for (size_t i = 0; i < str.length(); ++i)
    advance();

  return true;
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;
  
  while (!is_eof())
    tokens.push_back(next());

  return tokens;
}

Token Lexer::next() {
  size_t start_column = column_;
  size_t start_line   = line_;
  size_t start_pos    = pos_;

  if (match("<:")) return {TokenType::DIRECTIVE_OPEN,  src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  if (match(":>")) return {TokenType::DIRECTIVE_CLOSE, src_.substr(start_pos, pos_ - start_pos), start_line, start_column};

  if (match('\n')) return {TokenType::NEWLINE, src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  if (match('('))  return {TokenType::LPAREN,  src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  if (match(')'))  return {TokenType::RPAREN,  src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  if (match(','))  return {TokenType::COMMA,   src_.substr(start_pos, pos_ - start_pos), start_line, start_column};

  char c = peek();

  if (is_space_char(c)) {
    advance();
    while (is_space_char(peek())) advance();
    return {TokenType::SPACE, src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  }
  if (is_id_start(c)) {
    advance();
    while (is_id_body(peek())) advance();
    return {TokenType::ID, src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
  }

  /* TEXT */
  advance();

  while (!is_eof()) {
    c = peek();

    if (is_space_char(c) || is_id_start(c) || c == '\n' || c == '(' || c == ')' || c == ',') break;
    if (check("<:") || check(":>")) break;

    advance();
  }
  return {TokenType::TEXT, src_.substr(start_pos, pos_ - start_pos), start_line, start_column};
}

std::ostream &operator<<(std::ostream &os, TokenType type) {
  switch (type) {
    case TokenType::ID:              return os << "ID";
    case TokenType::TEXT:            return os << "TEXT";
    case TokenType::DIRECTIVE_OPEN:  return os << "DIRECTIVE_OPEN";
    case TokenType::DIRECTIVE_CLOSE: return os << "DIRECTIVE_CLOSE";
    case TokenType::LPAREN:          return os << "LPAREN";
    case TokenType::RPAREN:          return os << "RPAREN";
    case TokenType::COMMA:           return os << "COMMA";
    case TokenType::NEWLINE:         return os << "NEWLINE";
    case TokenType::SPACE:           return os << "SPACE";
  }
  return os << "<unknown>";
}

}