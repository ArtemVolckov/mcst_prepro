#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>

namespace prepro { 

struct ASTNode;
struct BlockNode;
struct DefBlockNode;
struct ScopeNode; 

}

namespace prepro {

class Environment {
public:
  Environment() = default;
  Environment(const Environment &parent) : parent_(&parent) {}

  void define(const std::string_view &id, const std::string &value);
  void define_block(const DefBlockNode &block);

  const std::string *find_define(const std::string_view &id) const;
  const DefBlockNode *find_block(const std::string_view &id) const;

private:
  const Environment *parent_ = nullptr;

  std::unordered_map<std::string_view, std::string> defines_;
  std::unordered_map<std::string_view, const DefBlockNode*> defblocks_;
  
  [[noreturn]]
  void error(const std::string_view &message) const;
};

class Preprocessor {
public:
  std::string process(const ScopeNode &root) const;

private:
  void process_scope(const ScopeNode &scope, const Environment &env, std::string &out) const;
  void process_macro(const std::vector<std::unique_ptr<ASTNode>> &macro, Environment &env, std::string &out) const;
  void process_node(const ASTNode &node, Environment &env, std::string &out) const;
  void process_block(const BlockNode &block, Environment &env, std::string &out) const;

  [[noreturn]]
  void error(const ASTNode &node, const std::string_view &message) const;
};

}