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
  return std::make_unique<ScopeNode>(parse_macro(MacroStop::EndOfFile));
}

std::unique_ptr<ScopeNode> Parser::parse_scope() {
  if (previous().type_ == TokenType::NEWLINE) {
    return std::make_unique<ScopeNode>(parse_macro(MacroStop::DIRECTIVE_CLOSE));
  }
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::NEWLINE) {
    error(*token, "expected newline after 'scope'");
  }
  return std::make_unique<ScopeNode>(parse_macro(MacroStop::DIRECTIVE_CLOSE));
}

std::unique_ptr<DefBlockNode> Parser::parse_defblock() {
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "expected id after 'defblock'");
  }
  const std::string_view &id = token->lexeme_;
  
  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::LPAREN) {
    error(*token, "expected '('");
  }
  std::vector<std::string_view> args;

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ == TokenType::RPAREN) {
    token = &advance();
    if (token->type_ == TokenType::SPACE) {
      token = &advance();
    }
    if (token->type_ != TokenType::NEWLINE) {
      error(*token, "expected newline after after list of arguments");
    }
    return std::make_unique<DefBlockNode>(id, std::move(args), parse_macro(MacroStop::DIRECTIVE_CLOSE));
  }

  for (;;) {
    if (token->type_ == TokenType::SPACE) {
      token = &advance();
    }
    if (token->type_ != TokenType::ID) {
      error(*token, "expected argument name");
    }
    args.push_back(token->lexeme_);

    token = &advance();
    if (token->type_ == TokenType::SPACE) {
      token = &advance();
    }
    if (token->type_ == TokenType::RPAREN) {
      break;
    }
    if (token->type_ != TokenType::COMMA) {
      error(*token, "expected ',' or ')'");
    }
    token = &advance();
  }
  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::NEWLINE) {
    error(*token, "expected newline after list of arguments");
  }
  return std::make_unique<DefBlockNode>(id, std::move(args), parse_macro(MacroStop::DIRECTIVE_CLOSE));
}

std::unique_ptr<BlockNode> Parser::parse_block() {
  const Token* token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "expected id after 'block'");
  }
  const std::string_view& id = token->lexeme_;

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::LPAREN) {
    error(*token, "expected '('");
  }
  std::vector<std::unique_ptr<ASTNode>> args;

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ == TokenType::RPAREN) {
    token = &advance();
    if (token->type_ == TokenType::SPACE) {
      token = &advance();
    }
    if (token->type_ != TokenType::DIRECTIVE_CLOSE) {
      error(*token, "expected ':>' after list of arguments");
    }
    return std::make_unique<BlockNode>(id, std::move(args));
  }
  if (previous(2).type_ == TokenType::SPACE) {
    pos_ -= 2;
  }
  else {
    --pos_;
  }

  for (;;) {
    args.push_back(std::make_unique<ScopeNode>(parse_macro(MacroStop::BLOCK_ARGUMENT)));
    if (previous().type_ == TokenType::RPAREN) {
      break;
    }
  }

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::DIRECTIVE_CLOSE) {
    error(*token, "expected ':>' after list of arguments");
  }
  return std::make_unique<BlockNode>(id, std::move(args));
}

std::unique_ptr<DefineNode> Parser::parse_define() {
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "expected id after 'define'");
  }
  const std::string_view &id = token->lexeme_;

  if (is_at_end()) {
    error(*token, "expected replacement string or ':>'");
  }
  token = &advance();

  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
    return std::make_unique<DefineNode>(id, "");
  }
  if (token->type_ == TokenType::DIRECTIVE_OPEN) {
    error(*token, "'<:' is not allowed inside define directive");
  }
  std::string_view value = token->lexeme_;

  while (!is_at_end()) {
    token = &advance();
    if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
      break;
    }
    if (token->type_ == TokenType::DIRECTIVE_OPEN) {
      error(*token, "'<:' is not allowed inside define directive");
    }
    value = {value.data(), static_cast<size_t>((token->lexeme_.data() + token->lexeme_.size()) - value.data())};
  }
  if (token->type_ != TokenType::DIRECTIVE_CLOSE) {
    error(*token, "expected closing ':>'");
  }
  /* trim */
  if (previous(2).type_ == TokenType::SPACE) {
    token = &previous(2);
    value = {value.data(), static_cast<size_t>(token->lexeme_.data() - value.data())};
  }
  return std::make_unique<DefineNode>(id, value);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_macro(MacroStop stop) {
  std::vector<std::unique_ptr<ASTNode>> macro;
  if (is_at_end()) {
    if (stop == MacroStop::DIRECTIVE_CLOSE) {
      error(previous(), "expected closing ':>'");
    }
    if (stop == MacroStop::BLOCK_ARGUMENT) {
      error(previous(), "expected ',' or ')' to close/separate block arguments");
    }
    return macro;
  }
  const Token *token;

  while (!is_at_end()) {
    token = &advance();
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
      if (stop != MacroStop::DIRECTIVE_CLOSE) {
        error(*token, "unexpected ':>'");
      }
      size_t back = 2;
      if (previous(back).type_ == TokenType::SPACE) {
        ++back;
      }
      if (previous(back).type_ != TokenType::NEWLINE) {
        error(*token, "expected newline before ':>'");
      }
      return macro;
    }
    if ((token->type_ == TokenType::RPAREN ||
      token->type_    == TokenType::COMMA) &&
      stop == MacroStop::BLOCK_ARGUMENT) {
      return macro;
    } 
    if (token->type_ == TokenType::LPAREN &&
      stop == MacroStop::BLOCK_ARGUMENT) {
      error(*token, "'(' is not allowed inside block argument");
    }

    /* TEXT */
    std::string_view text = token->lexeme_;

    while (!is_at_end()) {
      token = &peek();
      if ((token->type_ == TokenType::ID || 
        token->type_    == TokenType::DIRECTIVE_OPEN || 
        token->type_    == TokenType::DIRECTIVE_CLOSE) ||
        ((token->type_  == TokenType::LPAREN ||
        token->type_    == TokenType::RPAREN ||
        token->type_    == TokenType::COMMA) &&
        stop == MacroStop::BLOCK_ARGUMENT)) {
        break;
      }
      token = &advance();
      text = {text.data(), static_cast<size_t>((token->lexeme_.data() + token->lexeme_.size()) - text.data())};
    }
    macro.push_back(std::make_unique<TextNode>(text));
  }
  if (stop == MacroStop::DIRECTIVE_CLOSE) {
    error(*token, "expected closing ':>'");
  }
  if (stop == MacroStop::BLOCK_ARGUMENT) {
    error(*token, "expected ',' or ')' to close/separate block arguments");
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