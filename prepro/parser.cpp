#include <prepro/parser.hpp>

#include <vector>
#include <string_view>
#include <memory>

namespace prepro {

std::unique_ptr<ScopeNode> Parser::parse() {
  return std::make_unique<ScopeNode>(parse_macro());
}

std::unique_ptr<ScopeNode> Parser::parse_scope() {
  return std::make_unique<ScopeNode>(parse_macro());
}

std::unique_ptr<DefBlockNode> Parser::parse_defblock() {
  throw std::logic_error("parse_defblock() is not implemented");
}

std::unique_ptr<BlockNode> Parser::parse_block() {
  throw std::logic_error("parse_block() is not implemented");
}

std::unique_ptr<DefineNode> Parser::parse_define() {
  const Token *token = &advance();

  if (token->type == TokenType::SPACE) token = &advance();
  if (token->type != TokenType::ID) error(*token, "expected identifier");

  std::string_view id = token->lexeme;
  token = &advance();

  if (token->type == TokenType::SPACE) token = &advance();
  if (token->type == TokenType::DIRECTIVE_CLOSE) return std::make_unique<DefineNode>(id, std::string_view{});

  std::string_view value = token->lexeme;

  while (!is_at_end()) {
    token = &peek();

    if (token->type == TokenType::DIRECTIVE_CLOSE) {
        advance(); 
        break;
    }
    token = &advance();
    value = {value.data(), static_cast<size_t>((token->lexeme.data() + token->lexeme.size()) - value.data())};
  }
  return std::make_unique<DefineNode>(id, value);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_macro() {
  std::vector<std::unique_ptr<ASTNode>> macro;

  while (!is_at_end()) {
    const Token *token = &advance();

    if (token->type == TokenType::DIRECTIVE_CLOSE) return macro;

    if (token->type == TokenType::ID) {
      macro.push_back(std::make_unique<IdNode>(token->lexeme));
      continue;
    }
    if (token->type == TokenType::DIRECTIVE_OPEN) {
      token = &advance();

      if (token->type == TokenType::SPACE) token = &advance();
      if (token->type != TokenType::ID) error (*token, "expected directive name");

      if (token->lexeme == "scope") macro.push_back(parse_scope());
      else if (token->lexeme == "defblock") macro.push_back(parse_defblock());
      else if (token->lexeme == "block") macro.push_back(parse_block());
      else if (token->lexeme == "define") macro.push_back(parse_define());
      else error(*token, "unknown directive");

      continue;
    }

    /* TEXT */
    std::string_view text = token->lexeme;

    while (!is_at_end()) {
      token = &peek();

      if (token->type == TokenType::ID || token->type == TokenType::DIRECTIVE_OPEN || token->type == TokenType::DIRECTIVE_CLOSE) break;
      token = &advance();
      text = {text.data(), static_cast<size_t>((token->lexeme.data() + token->lexeme.size()) - text.data())};
    }
    macro.push_back(std::make_unique<TextNode>(text));
  }
  return macro;
}

bool Parser::is_at_end() const {
  return pos_ >= tokens_.size();
}

const Token &Parser::peek() {
  return tokens_[pos_];
}

const Token &Parser::advance() {
  if (!is_at_end()) ++pos_;
  return tokens_[pos_-1];
}

[[noreturn]]
void Parser::error(const Token& token, std::string_view message) const {
  throw std::runtime_error(
    "Parser error at " +
    std::to_string(token.line) +
    ":" +
    std::to_string(token.column) +
    ": " +
    std::string(message)
  );
}

}