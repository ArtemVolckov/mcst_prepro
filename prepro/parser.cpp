#include <prepro/ast.hpp>
#include <prepro/lexer.hpp>
#include <prepro/parser.hpp>

#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <cstddef>
#include <utility>
#include <stdexcept>

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
  return std::make_unique<ScopeNode>(parse_macro(MacroStop::EndOfFile), 1, 1);
}

std::unique_ptr<ScopeNode> Parser::parse_scope(size_t line, size_t column) {
  if (previous().type_ == TokenType::NEWLINE) {
    return std::make_unique<ScopeNode>(parse_macro(MacroStop::DIRECTIVE_CLOSE), line, column);
  }
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::NEWLINE) {
    error(*token, "ожидался перенос строки после 'scope'");
  }
  return std::make_unique<ScopeNode>(parse_macro(MacroStop::DIRECTIVE_CLOSE), line, column);
}

std::unique_ptr<DefBlockNode> Parser::parse_defblock(size_t line, size_t column) {
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "ожидался идентификатор после 'defblock'");
  }
  const std::string_view &id = token->lexeme_;
  
  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::LPAREN) {
    error(*token, "ожидалась '('");
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
      error(*token, "ожидался перенос строки после списка аргументов");
    }
    return std::make_unique<DefBlockNode>(id, std::move(args), parse_macro(MacroStop::DIRECTIVE_CLOSE), line, column);
  }

  for (;;) {
    if (token->type_ == TokenType::SPACE) {
      token = &advance();
    }
    if (token->type_ != TokenType::ID) {
      error(*token, "ожидалось имя аргумента");
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
      error(*token, "ожидалась ',' или ')'");
    }
    token = &advance();
  }
  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::NEWLINE) {
    error(*token, "ожидался перенос строки после списка аргументов");
  }
  return std::make_unique<DefBlockNode>(id, std::move(args), parse_macro(MacroStop::DIRECTIVE_CLOSE), line, column);
}

std::unique_ptr<BlockNode> Parser::parse_block(size_t line, size_t column) {
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "ожидался идентификатор после 'block'");
  }
  const std::string_view &id = token->lexeme_;

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::LPAREN) {
    error(*token, "ожидалась '('");
  }
  std::vector<std::unique_ptr<ScopeNode>> args;

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
      error(*token, "ожидалось ':>' после списка аргументов");
    }
    return std::make_unique<BlockNode>(id, std::move(args), line, column);
  }
  if (previous(2).type_ == TokenType::SPACE) {
    pos_ -= 2;
  }
  else {
    --pos_;
  }

  for (;;) {
    const Token &arg_begin = peek();
    args.push_back(std::make_unique<ScopeNode>(parse_macro(MacroStop::BLOCK_ARGUMENT), arg_begin.line_, arg_begin.column_));
    if (previous().type_ == TokenType::RPAREN) {
      break;
    }
  }

  token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::DIRECTIVE_CLOSE) {
    error(*token, "ожидалось ':>' после списка аргументов");
  }
  return std::make_unique<BlockNode>(id, std::move(args), line, column);
}

std::unique_ptr<DefineNode> Parser::parse_define(size_t line, size_t column) {
  const Token *token = &advance();
  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ != TokenType::ID) {
    error(*token, "ожидался идентификатор после 'define'");
  }
  const std::string_view &id = token->lexeme_;

  if (is_at_end()) {
    error(*token, "ожидалась строка подстановки или ':>'");
  }
  token = &advance();

  if (token->type_ == TokenType::SPACE) {
    token = &advance();
  }
  if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
    return std::make_unique<DefineNode>(id, "", line, column);
  }
  if (token->type_ == TokenType::DIRECTIVE_OPEN) {
    error(*token, "'<:' не допускается внутри директивы define");
  }
  std::string_view value = token->lexeme_;

  while (!is_at_end()) {
    token = &advance();
    if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
      break;
    }
    if (token->type_ == TokenType::DIRECTIVE_OPEN) {
      error(*token, "'<:' не допускается внутри директивы define");
    }
    value = {value.data(), static_cast<size_t>((token->lexeme_.data() + token->lexeme_.size()) - value.data())};
  }
  if (token->type_ != TokenType::DIRECTIVE_CLOSE) {
    error(*token, "ожидалось закрывающее ':>'");
  }
  // trim
  if (previous(2).type_ == TokenType::SPACE) {
    token = &previous(2);
    value = {value.data(), static_cast<size_t>(token->lexeme_.data() - value.data())};
  }
  return std::make_unique<DefineNode>(id, value, line, column);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse_macro(MacroStop stop) {
  std::vector<std::unique_ptr<ASTNode>> macro;
  if (is_at_end()) {
    if (stop == MacroStop::DIRECTIVE_CLOSE) {
      error(previous(), "ожидалось закрывающее ':>'");
    }
    if (stop == MacroStop::BLOCK_ARGUMENT) {
      error(previous(), "ожидалась ',' или ')' для разделения/закрытия аргументов блока");
    }
    return macro;
  }
  const Token *token;

  while (!is_at_end()) {
    token = &advance();
    if (token->type_ == TokenType::ID) {
      macro.push_back(std::make_unique<IdNode>(token->lexeme_, token->line_, token->column_));
      continue;
    }
    if (token->type_ == TokenType::DIRECTIVE_OPEN) {
      size_t line = token->line_;
      size_t column = token->column_;
      token = &advance();
      if (token->type_ == TokenType::SPACE) {
        token = &advance();
      }
      if (token->type_ == TokenType::NEWLINE) {
        macro.push_back(parse_scope(line, column));
        continue;
      }
      if (token->type_ != TokenType::ID) {
        error (*token, "ожидалось имя директивы или перенос строки после '<:'");
      }

      if (token->lexeme_ == "scope") {
        macro.push_back(parse_scope(line, column));
      }
      else if (token->lexeme_ == "defblock") {
        macro.push_back(parse_defblock(line, column));
      }
      else if (token->lexeme_ == "block") {
        macro.push_back(parse_block(line, column));
      }
      else if (token->lexeme_ == "define") {
        macro.push_back(parse_define(line, column));
      }
      else {
        error(*token, "неизвестная директива");
      }
      continue;
    }
    if (token->type_ == TokenType::DIRECTIVE_CLOSE) {
      if (stop != MacroStop::DIRECTIVE_CLOSE) {
        error(*token, "неожиданное ':>'");
      }
      size_t back = 2;
      if (previous(back).type_ == TokenType::SPACE) {
        ++back;
      }
      if (previous(back).type_ != TokenType::NEWLINE) {
        error(*token, "ожидался перенос строки перед ':>'");
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
      error(*token, "'(' не допускается внутри аргумента блока");
    }

    // TEXT
    std::string_view text = token->lexeme_;
    size_t line = token->line_;
    size_t column = token->column_;

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
    macro.push_back(std::make_unique<TextNode>(text, line, column));
  }
  if (stop == MacroStop::DIRECTIVE_CLOSE) {
    error(*token, "ожидалось закрывающее ':>'");
  }
  if (stop == MacroStop::BLOCK_ARGUMENT) {
    error(*token, "ожидалась ',' или ')' для разделения/закрытия аргументов блока");
  }
  return macro;
}

[[noreturn]]
void Parser::error(const Token &token, const std::string_view &message) const {
  throw std::runtime_error(
    "Ошибка парсера на " +
    std::to_string(token.line_) +
    ":" +
    std::to_string(token.column_) +
    ": " +
    std::string(message)
  );
}

}