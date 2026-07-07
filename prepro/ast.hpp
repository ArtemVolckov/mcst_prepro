#pragma once

#include <string>
#include <vector>
#include <variant>

struct RawTextData {
  std::string text;
};

#if 0
struct ScopeData {

};

struct DefBlockData {

};

struct BlockData {

};
#endif

struct DefineData {
  std::string name;
  std::string value;
};

struct ASTNode {
  std::variant<RawTextData, DefineData> data;
};