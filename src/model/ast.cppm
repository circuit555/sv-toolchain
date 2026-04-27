// SPDX-License-Identifier: MIT

export module svt.model.ast;

import std;

namespace svt::model {

enum class BinaryOperation : std::uint8_t { kPlus, kMinus, kMultiply, kDivide };

export enum class PortDirection : std::uint8_t { kInput, kOutput };

enum class NetType : std::uint8_t { kWire, kLogic };

struct Declaration {
  std::string_view name;
};

class IdentifierExpression;
class NumberExpression;
class BinaryExpression;
export class PortDeclaration;
class NetDeclaration;
export class ContinuousAssign;
export class ModuleDeclaration;

export struct ParameterTypeDeclaration : Declaration {
  std::vector<std::string_view> default_type;
};

export struct ParameterValueDeclaration : Declaration {
  std::vector<std::string_view> type_specifier;
  std::vector<std::string_view> default_value;
};

export using ParameterDeclaration =
    std::variant<ParameterTypeDeclaration, ParameterValueDeclaration>;

/// @brief Type-erased AST node that stores one concrete node variant.
export using AstNode =
    std::variant<IdentifierExpression, NumberExpression, BinaryExpression,
                 ParameterDeclaration, PortDeclaration, NetDeclaration,
                 ModuleDeclaration>;

/// @brief Owning pointer to an AST node.
export using AstNodePointer = AstNode*;

struct IdentifierExpression : Declaration {};

struct NumberExpression {
  int value;
};

struct BinaryExpression {
  BinaryOperation operation;
  AstNodePointer left_operand_ptr;
  AstNodePointer right_operand_ptr;
};

struct PortDeclaration : Declaration {
  PortDirection direction;
};

struct NetDeclaration : Declaration {
  NetType type;
  AstNodePointer msb;
  AstNodePointer lsb;
};

struct ContinuousAssign {
  std::vector<std::string_view> left_hand_side;
  std::vector<std::string_view> right_hand_side;
};

export using ModuleItem = std::variant<ContinuousAssign>;

export struct ModuleDeclaration : Declaration {
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<ModuleItem> items;
};

}  // namespace svt::model
