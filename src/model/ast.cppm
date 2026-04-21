

export module svt.model.ast;

import std;

namespace svt::model {

enum class BinaryOperation : std::uint8_t { kPlus, kMinus, kMultiply, kDivide };

enum class PortDirection : std::uint8_t { kInput, kOutput };

enum class NetType : std::uint8_t { kWire, kLogic };

class IdentifierExpression;
class NumberExpression;
class BinaryExpression;
export class ParameterDeclaration;
export class PortDeclaration;
class NetDeclaration;
export class ModuleDeclaration;

/// @brief Type-erased AST node that stores one concrete node variant.
export using AstNode =
    std::variant<IdentifierExpression, NumberExpression, BinaryExpression,
                 ParameterDeclaration, PortDeclaration, NetDeclaration,
                 ModuleDeclaration>;

/// @brief Owning pointer to an AST node.
export using AstNodePointer = AstNode*;

struct IdentifierExpression {
  std::string_view name;
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
  std::string_view name;
  std::vector<std::string_view> type_specifier;
  std::vector<std::string_view> default_value;
  bool is_type_parameter{false};
};

struct PortDeclaration {
  std::string_view name;
  PortDirection direction;
};

struct NetDeclaration {
  std::string_view name;
  NetType type;
  AstNodePointer msb;
  AstNodePointer lsb;
};

export struct ModuleDeclaration {
  std::string_view name;
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<AstNodePointer> items;
};

}  // namespace svt::model
