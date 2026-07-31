#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <utility>
#include <cstddef>

namespace prepro {

enum class NodeType {
  ID,
  TEXT,
  SCOPE,
  DEF_BLOCK,
  BLOCK,
  DEFINE
};

struct ASTNode {
  NodeType type_;
  size_t line_;
  size_t column_;
    
  ASTNode(NodeType type, size_t line, size_t column) 
    : type_(type), line_(line), column_(column) {}
  
  virtual ~ASTNode() = 0;
  ASTNode(const ASTNode&) = delete;
  ASTNode& operator=(const ASTNode&) = delete;
  ASTNode(ASTNode&&) noexcept = default;
  ASTNode& operator=(ASTNode&&) noexcept = default;
};

inline ASTNode::~ASTNode() = default;

struct TextNode : public ASTNode {
  std::string_view text_;

  TextNode(const std::string_view &text, size_t line, size_t column) 
    : ASTNode(NodeType::TEXT, line, column), text_(text) {}
};

struct IdNode : public ASTNode {
  std::string_view id_;

  IdNode(const std::string_view &id, size_t line, size_t column) 
    : ASTNode(NodeType::ID, line, column), id_(id) {}
};

struct ScopeNode : public ASTNode {
  std::vector<std::unique_ptr<ASTNode>> macro_;

  ScopeNode(std::vector<std::unique_ptr<ASTNode>> &&macro, size_t line, size_t column) 
    : ASTNode(NodeType::SCOPE, line, column), macro_(std::move(macro)) {}
};

struct DefBlockNode : public ASTNode {
  std::string_view id_;                       
  std::vector<std::string_view> args_;       
  std::vector<std::unique_ptr<ASTNode>> macro_;

  DefBlockNode(const std::string_view &id, std::vector<std::string_view> &&args, 
    std::vector<std::unique_ptr<ASTNode>> &&macro, size_t line, size_t column) 
    : ASTNode(NodeType::DEF_BLOCK, line, column), id_(id), args_(std::move(args)), macro_(std::move(macro)) {}
};

struct BlockNode : public ASTNode {
  std::string_view id_;
  // каждый аргумент — это макроблок (ScopeNode)
  std::vector<std::unique_ptr<ScopeNode>> args_;

  BlockNode(const std::string_view &id, std::vector<std::unique_ptr<ScopeNode>> &&args, size_t line, size_t column) 
    : ASTNode(NodeType::BLOCK, line, column), id_(id), args_(std::move(args)) {}
};

struct DefineNode : public ASTNode {
  std::string_view id_;
  std::string_view value_;

  DefineNode(const std::string_view &id, const std::string_view &value, size_t line, size_t column) 
    : ASTNode(NodeType::DEFINE, line, column), id_(id), value_(value) {}
};

}