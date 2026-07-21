#include <prepro/lexer.hpp>

#include <iostream>
#include <cassert>
#include <vector>
#include <string_view>

#define RUN_TEST(test_func) run_test(#test_func, test_func)

using namespace prepro;

template <typename T>
void run_test(std::string_view name, T test) {
  std::cout << "[ RUN      ] " << name << '\n';
  test();
  std::cout << "[       OK ] " << name << '\n';
}

void check_token_lexeme(const Token &token, TokenType type, std::string_view lexeme) {
  assert(token.type == type);
  assert(token.lexeme == lexeme);
}

void check_line_column(const Token &token, size_t line, size_t column) {
  assert(token.line == line);
  assert(token.column == column);
}

void test_basic_tokens() {
  std::string_view src = "<::>(),";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 5);

  check_token_lexeme(tokens[0], TokenType::DIRECTIVE_OPEN, "<:");
  check_token_lexeme(tokens[1], TokenType::DIRECTIVE_CLOSE, ":>");
  check_token_lexeme(tokens[2], TokenType::LPAREN, "(");
  check_token_lexeme(tokens[3], TokenType::RPAREN, ")");
  check_token_lexeme(tokens[4], TokenType::COMMA, ",");
}

void test_spaces() {
  std::string_view src = " \r\ta   ";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 3);

  check_token_lexeme(tokens[0], TokenType::SPACE, " \r\t");
  check_token_lexeme(tokens[1], TokenType::ID, "a");
  check_token_lexeme(tokens[2], TokenType::SPACE, "   ");
}

void test_empty() {
  std::string_view src = "";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.empty());
}

void test_newlines() {
  std::string_view src = "\n\n\n";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 3);

  check_token_lexeme(tokens[0], TokenType::NEWLINE, "\n");
  check_line_column(tokens[0], 1, 1);

  check_token_lexeme(tokens[1], TokenType::NEWLINE, "\n");
  check_line_column(tokens[1], 2, 1);

  check_token_lexeme(tokens[2], TokenType::NEWLINE, "\n");
  check_line_column(tokens[2], 3, 1);
}

void test_id() {
  Lexer lexer1("hello");
  std::vector<Token> tokens1 = lexer1.tokenize();

  assert(tokens1.size() == 1);
  check_token_lexeme(tokens1[0], TokenType::ID, "hello");

  Lexer lexer2("abc123_def");
  std::vector<Token> tokens2 = lexer2.tokenize();
  
  assert(tokens2.size() == 1);
  check_token_lexeme(tokens2[0], TokenType::ID, "abc123_def");
  
  Lexer lexer3("_abc");
  std::vector<Token> tokens3 = lexer3.tokenize();
  
  assert(tokens3.size() == 2);
  check_token_lexeme(tokens3[0], TokenType::TEXT, "_");
  check_token_lexeme(tokens3[1], TokenType::ID, "abc");
}

void test_text() {
  std::string_view src = "+=-;";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 1);
  check_token_lexeme(tokens[0], TokenType::TEXT, "+=-;");
}

void test_mixed() {
  std::string_view src = "x = <: ()\n  y";
  Lexer lexer(src);
  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 11);

  check_token_lexeme(tokens[0], TokenType::ID, "x");
  check_line_column(tokens[0], 1, 1);

  check_token_lexeme(tokens[1], TokenType::SPACE, " ");
  check_line_column(tokens[1], 1, 2);

  check_token_lexeme(tokens[2], TokenType::TEXT, "=");
  check_line_column(tokens[2], 1, 3);

  check_token_lexeme(tokens[3], TokenType::SPACE, " ");
  check_line_column(tokens[3], 1, 4);

  check_token_lexeme(tokens[4], TokenType::DIRECTIVE_OPEN, "<:");
  check_line_column(tokens[4], 1, 5);

  check_token_lexeme(tokens[5], TokenType::SPACE, " ");
  check_line_column(tokens[5], 1, 7);

  check_token_lexeme(tokens[6], TokenType::LPAREN, "(");
  check_line_column(tokens[6], 1, 8);

  check_token_lexeme(tokens[7], TokenType::RPAREN, ")");
  check_line_column(tokens[7], 1, 9);

  check_token_lexeme(tokens[8], TokenType::NEWLINE, "\n");
  check_line_column(tokens[8], 1, 10);

  check_token_lexeme(tokens[9], TokenType::SPACE, "  ");
  check_line_column(tokens[9], 2, 1);

  check_token_lexeme(tokens[10], TokenType::ID, "y");
  check_line_column(tokens[10], 2, 3);
}

int main() {
  std::cout << "--- LEXER UNIT TESTS ---\n\n";

  RUN_TEST(test_basic_tokens);
  RUN_TEST(test_spaces);
  RUN_TEST(test_empty);
  RUN_TEST(test_newlines);
  RUN_TEST(test_id);
  RUN_TEST(test_text);
  RUN_TEST(test_mixed);

  std::cout << "\n--- ALL TESTS PASSED ---\n";
}
