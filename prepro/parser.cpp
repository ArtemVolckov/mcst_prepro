#include <prepro/Parser.hpp>

std::vector<ASTNode> Parser::parse() {
  std::vector<ASTNode> ast;

  while (!is_eof()) {
    size_t next_position = input.find("<:", position);

    if (next_position == std::string_view::npos)
      next_position = input.length();
    if (next_position != position) {
      std::string text(input.substr(position, next_position - position));
      ast.push_back(ASTNode{ RawTextData{text} });
      position = next_position;
    }
    if (!is_eof())
      parse_directive(ast);
  }
  return ast;
}

void Parser::parse_directive(std::vector<ASTNode> &ast) {
  match_and_consume("<:");
  skip_spaces();

  if (match_and_consume("define")) {
    parse_define(ast);
  }
  else if (match_and_consume("defblock")) {
    parse_defblock(ast);
  }
  else if (match_and_consume("block")) {
    parse_block(ast);
  }
  /*else {
    throw
  }*/
}

void Parser::parse_define(std::vector<ASTNode> &ast) {
  //
}

void Parser::parse_defblock(std::vector<ASTNode> &ast) {
  //
}

void Parser::parse_block(std::vector<ASTNode> &ast) {
  //
}

void Parser::parse_scope(std::vector<ASTNode> &ast) {
  //
}