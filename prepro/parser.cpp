#include <prepro/parser.hpp>

#include <vector>
#include <string_view>
#include <memory>

namespace prepro {

const Token &Parser::peek() const noexcept {
  return tokens_[pos_];
}

const Token &Parser::advance() noexcept {
  if (!is_at_end()) {
    ++pos_;
  }
  return tokens_[pos_ - 1];
}

const Token &Parser::previous(size_t n) const noexcept {
  return tokens_[pos_ - n];
}

std::unique_ptr<ScopeNode> Parser::parse() {
  return std::make_unique<ScopeNode>(parse_macro(false));
}

std::unique_ptr<ScopeNode> Parser::parse_scope() {
  return std::make_unique<ScopeNode>(parse_macro(true));
}

std::unique_ptr<DefBlockNode> Parser::parse_defblock() {
  throw std::logic_error("parse_defblock() is not implemented");
}

std::unique_ptr<BlockNode> Parser::parse_block() {
  throw std::logic_error("parse_block() is not implemented");
}

std::unique_ptr<DefineNode> Parser::parse_define() {
  const Token *token = &advance();

  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "expected id");
  }

  const std::string_view &id = token->lexeme_;
  token = &advance();

  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
    return std::make_unique<DefineNode>(id, "");
  }

  std::string_view value = token->lexeme_;

  while (!is_at_end()) {
    token = &peek();

    if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
        advance(); 
        break;
    }
    token = &advance();
    value = {value.data(), static_cast<size_t>((token->lexeme_.data() + token->lexeme_.size()) - value.data())};
  }
  return std::make_unique<DefineNode>(id, value);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_macro(bool expect_close) {
  std::vector<std::unique_ptr<ASTNode>> macro;

  while (!is_at_end()) {
    const Token *token = &advance();

    if (token->type_ == TokenType::ID) {
      macro.push_back(std::make_unique<IdNode>(token->lexeme_));
      continue;
    }
    if (token->type_ == TokenType::DIRECTIVE_OPEN) {
      token = &advance();

      if (token->type_ == TokenType::SPACE) {
        token = &advance();
      }
      if (token->type_ == TokenType::NEWLINE) {
        macro.push_back(parse_scope());
        continue;
      }
      if (token->type_ != TokenType::ID) {
        error (*token, "expected directive name or newline after '<:'");
      }

      if (token->lexeme_ == "scope") {
        macro.push_back(parse_scope());
      }
      else if (token->lexeme_ == "defblock") {
        macro.push_back(parse_defblock());
      }
      else if (token->lexeme_ == "block") {
        macro.push_back(parse_block());
      }
      else if (token->lexeme_ == "define") {
        macro.push_back(parse_define());
      }
      else {
        error(*token, "unknown directive");
      }
      continue;
    }
    if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
      if (!expect_close) {
        error(*token, "unexpected :>");
      }
      size_t back = 2;
      if (previous(back).type_ == TokenType::SPACE) {
        ++back;
      }
      if (previous(back).type_ != TokenType::NEWLINE) {
        error(*token, "expected newline before :>");
      }
      return macro;
    }

    /* TEXT */
    std::string_view text = token->lexeme_;

    while (!is_at_end()) {
      token = &peek();

      if (token->type_ == TokenType::ID || 
        token->type_   == TokenType::DIRECTIVE_OPEN || 
        token->type_   == TokenType::DIRECTIVE_CLOSE) {
        break;
      }
      token = &advance();
      text = {text.data(), static_cast<size_t>((token->lexeme_.data() + token->lexeme_.size()) - text.data())};
    }
    macro.push_back(std::make_unique<TextNode>(text));
  }
  return macro;
}

[[noreturn]]
void Parser::error(const Token &token, const std::string_view &message) const {
  throw std::runtime_error(
    "Parser error at " +
    std::to_string(token.line_) +
    ":" +
    std::to_string(token.column_) +
    ": " +
    std::string(message)
  );
}

}