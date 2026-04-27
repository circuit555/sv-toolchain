// SPDX-License-Identifier: MIT

export module svt.model.ast;

import std;
import svt.model.token;

namespace svt::model {

enum class BinaryOperation : std::uint8_t { kPlus, kMinus, kMultiply, kDivide };

export enum class PortDirection : std::uint8_t { kInput, kOutput };

export enum class NetType : std::uint8_t { kWire, kLogic };

struct Declaration {
  std::string_view name;
};

class IdentifierExpression;
class NumberExpression;
class BinaryExpression;
export class PortDeclaration;
export class NetDeclaration;
export class ContinuousAssign;
export class ModuleDeclaration;

export struct ParameterTypeDeclaration : Declaration {
  std::span<Token const> default_type;
};

export struct ParameterValueDeclaration : Declaration {
  std::span<Token const> type_specifier;
  std::span<Token const> default_value;
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
  std::span<Token const> type_specifier;
};

struct ContinuousAssign {
  std::span<Token const> left_hand_side;
  std::span<Token const> right_hand_side;
};

export using ModuleItem = std::variant<NetDeclaration, ContinuousAssign>;

export struct ModuleDeclaration : Declaration {
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<ModuleItem> items;
};

}  // namespace svt::model
