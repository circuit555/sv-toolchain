

export module svt.model.ast;

import std;

namespace svt::model {

enum class BinaryOperation : std::uint8_t { kPlus, kMinus, kMultiply, kDivide };

enum class ParameterType : std::uint8_t { kInt };

enum class PortDirection : std::uint8_t { kInput, kOutput };

enum class NetType : std::uint8_t { kWire, kLogic };

export struct AstNode;

export using AstNodePtr = std::unique_ptr<AstNode>;

struct IdentifierExpression {
  std::string name;
};

struct NumberExpression {
  int value;
};

struct BinaryExpression {
  BinaryOperation operation;
  AstNodePtr left_operand_ptr;
  AstNodePtr right_operand_ptr;
};

struct ParameterDeclaration {
  std::string name;
  ParameterType type;
  AstNodePtr default_value;
};

struct PortDeclaration {
  std::string name;
  PortDirection direction;
};

struct NetDeclaration {
  std::string name;
  NetType type;
  AstNodePtr msb;
  AstNodePtr lsb;
};

struct ModuleDeclaration {
  std::string name;
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<AstNodePtr> items;
};

export struct TranslationUnit {
  std::vector<AstNodePtr> declarations;
};

using AstNodeVariant =
    std::variant<IdentifierExpression, NumberExpression, BinaryExpression,
                 ParameterDeclaration, PortDeclaration, NetDeclaration,
                 ModuleDeclaration>;

export struct AstNode {
  AstNodeVariant node;
};

}  // namespace svt::model
