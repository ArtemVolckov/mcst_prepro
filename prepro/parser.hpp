#pragma once

#include <prepro/ast.hpp>
#include <prepro/lexer.hpp>

#include <vector>
#include <string_view>
#include <memory>

namespace prepro {

class Parser {
  const std::vector<Token> &tokens_;
  size_t pos_ = 0;

  bool is_at_end() const noexcept {
    return pos_ >= tokens_.size();
  }

  const Token& peek();
  const Token& advance();

  [[noreturn]]
  void error(const Token& token, std::string_view message) const;

  std::unique_ptr<ScopeNode> parse_scope();
  std::unique_ptr<DefBlockNode> parse_defblock();
  std::unique_ptr<BlockNode> parse_block();
  std::unique_ptr<DefineNode> parse_define();

  std::vector<std::unique_ptr<ASTNode>> parse_macro();

public:
  Parser(const std::vector<Token> &tokens) : tokens_(tokens) {}
  std::unique_ptr<ScopeNode> parse();
};

}