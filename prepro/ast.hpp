#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <utility>

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
    
  ASTNode(NodeType type) : type_(type) {}
  virtual ~ASTNode() = 0;

  ASTNode(const ASTNode&) = delete;
  ASTNode& operator=(const ASTNode&) = delete;

  ASTNode(ASTNode&&) = default;
  ASTNode& operator=(ASTNode&&) = default;
};

inline ASTNode::~ASTNode() = default;

struct TextNode : public ASTNode {
  std::string_view text_;
  TextNode(std::string_view text) : ASTNode(NodeType::TEXT), text_(text) {}
};

struct IdNode : public ASTNode {
  std::string_view id_;
  IdNode(std::string_view id) : ASTNode(NodeType::ID), id_(id) {}
};

struct ScopeNode : public ASTNode {
  std::vector<std::unique_ptr<ASTNode>> macro_;
  ScopeNode(std::vector<std::unique_ptr<ASTNode>> macro) : ASTNode(NodeType::SCOPE), macro_(std::move(macro)) {}
};

struct DefBlockNode : public ASTNode {
  std::string_view id_;                       
  std::vector<std::string_view> args_;       
  std::vector<std::unique_ptr<ASTNode>> macro_;

  DefBlockNode(std::string_view id, std::vector<std::string_view> args, std::vector<std::unique_ptr<ASTNode>> macro) 
    : ASTNode(NodeType::DEF_BLOCK), id_(id), args_(std::move(args)), macro_(std::move(macro)) {}
};

struct BlockNode : public ASTNode {
  std::string_view id_;
  /* каждый аргумент — это макроблок (ScopeNode) */
  std::vector<std::unique_ptr<ASTNode>> args_; 
  BlockNode(std::string_view id, std::vector<std::unique_ptr<ASTNode>> args) 
    : ASTNode(NodeType::BLOCK), id_(id), args_(std::move(args)) {}
};

struct DefineNode : public ASTNode {
  std::string_view id_;
  std::string_view value_;
  DefineNode(std::string_view id, std::string_view value) : ASTNode(NodeType::DEFINE), id_(id), value_(value) {}
};

}