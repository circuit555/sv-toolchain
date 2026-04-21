

export module svt.model.ast;

import std;

namespace svt::model {

enum class BinaryOperation : std::uint8_t { kPlus, kMinus, kMultiply, kDivide };

enum class ParameterType : std::uint8_t { kInt };

enum class PortDirection : std::uint8_t { kInput, kOutput };

enum class NetType : std::uint8_t { kWire, kLogic };

/// @brief Forward declaration for the generic AST node wrapper.
export struct AstNode;

/// @brief Owning pointer to an AST node.
export using AstNodePointer = std::unique_ptr<AstNode>;

struct IdentifierExpression {
  std::string name;
};

struct NumberExpression {
  int value;
};

struct BinaryExpression {
  BinaryOperation operation;
  AstNodePointer left_operand_ptr;
  AstNodePointer right_operand_ptr;
};

struct ParameterDeclaration {
  std::string name;
  ParameterType type;
  AstNodePointer default_value;
};

struct PortDeclaration {
  std::string name;
  PortDirection direction;
};

struct NetDeclaration {
  std::string name;
  NetType type;
  AstNodePointer msb;
  AstNodePointer lsb;
};

struct ModuleDeclaration {
  std::string name;
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<AstNodePointer> items;
};

/// @brief Root AST object containing top-level declarations.
export struct TranslationUnit {
  /// Top-level declarations parsed from the source.
  std::vector<AstNodePointer> declarations;
};

using AstNodeVariant =
    std::variant<IdentifierExpression, NumberExpression, BinaryExpression,
                 ParameterDeclaration, PortDeclaration, NetDeclaration,
                 ModuleDeclaration>;

/// @brief Type-erased AST node that stores one concrete node variant.
export struct AstNode {
  /// Concrete AST node payload.
  AstNodeVariant node;
};

}  // namespace svt::model
