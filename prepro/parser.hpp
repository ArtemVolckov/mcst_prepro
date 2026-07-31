#pragma once

#include <prepro/ast.hpp>
#include <prepro/lexer.hpp>

#include <vector>
#include <string_view>
#include <memory>
#include <utility>
#include <cstddef>

namespace prepro {

enum class MacroStop {
  EndOfFile,
  DIRECTIVE_CLOSE,
  BLOCK_ARGUMENT
};

class Parser {
public:
  Parser(std::vector<Token> &&tokens) : tokens_(std::move(tokens)) {}
  std::unique_ptr<ScopeNode> parse();

private:
  std::vector<Token> tokens_;
  size_t pos_ = 0;

  bool is_at_end() const noexcept {
    return pos_ >= tokens_.size();
  }
  const Token &peek() const noexcept;
  const Token &previous(size_t n = 1) const noexcept;
  const Token &advance() noexcept;

  std::unique_ptr<ScopeNode> parse_scope(size_t line, size_t column);
  std::unique_ptr<DefBlockNode> parse_defblock(size_t line, size_t column);
  std::unique_ptr<BlockNode> parse_block(size_t line, size_t column);
  std::unique_ptr<DefineNode> parse_define(size_t line, size_t column);

  std::vector<std::unique_ptr<ASTNode>> parse_macro(MacroStop stop);
  
  [[noreturn]]
  void error(const Token &token, const std::string_view &message) const;
};

}