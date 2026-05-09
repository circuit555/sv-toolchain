// SPDX-License-Identifier: MIT

export module svt.model.ast;

import std;
import svt.model.token;

namespace svt::model {

export enum class PortDirection : std::uint8_t { kInput, kOutput };

export enum class NetType : std::uint8_t { kWire, kLogic };

export enum class LiteralKind : std::uint8_t { kInteger, kReal, kString };

struct Declaration {
  std::string_view name;
};

export struct Expression;
// NOTE(): we have to use a pointer here as we are using a recursive-descent
// parser e.g., `UnaryExpression` (see below) can itself contain a
// `UnaryExpression` as operand, hence it can not use `Expression` directly
// as member because it would cause a recursive-descent parser to enter an
// infinite loop. To avoid this, we have to use indirection i.e., a pointer so
// compiler is able to allocate memory for the `UnaryExpression` as pointer size
// is known at compile time.
export using ExpressionPtr = std::unique_ptr<Expression>;

export struct IdentifierExpression : Declaration {};

export struct SystemIdentifierExpression : Declaration {};

export struct LiteralExpression {
  LiteralKind kind{};
  std::string_view value;
};

export struct UnaryExpression {
  std::string_view operator_lexeme;
  ExpressionPtr operand;
};

export struct BinaryExpression {
  std::string_view operator_lexeme;
  ExpressionPtr left;
  ExpressionPtr right;
};

export struct ConditionalExpression {
  ExpressionPtr condition;
  ExpressionPtr true_expression;
  ExpressionPtr false_expression;
};

export struct IndexExpression {
  ExpressionPtr base;
  ExpressionPtr index;
};

export struct RangeSelectExpression {
  ExpressionPtr base;
  ExpressionPtr left;
  ExpressionPtr right;
};

export struct ConcatenationExpression {
  std::vector<ExpressionPtr> expressions;
};

export struct ReplicationExpression {
  ExpressionPtr count;
  std::vector<ExpressionPtr> expressions;
};

export struct CallExpression {
  ExpressionPtr callee;
  std::vector<ExpressionPtr> arguments;
};

export struct AssignmentPatternExpression {
  std::vector<ExpressionPtr> expressions;
};

export struct UnsupportedExpression {
  std::span<Token const> tokens;
};

export using ExpressionNode =
    std::variant<std::monostate, IdentifierExpression,
                 SystemIdentifierExpression, LiteralExpression, UnaryExpression,
                 BinaryExpression, ConditionalExpression, IndexExpression,
                 RangeSelectExpression, ConcatenationExpression,
                 ReplicationExpression, CallExpression,
                 AssignmentPatternExpression, UnsupportedExpression>;

export struct Expression {
  ExpressionNode node;
  std::span<Token const> tokens;
};

export struct Statement;
export using StatementPtr = std::unique_ptr<Statement>;

export enum class ProceduralBlockKind : std::uint8_t {
  kInitial,
  kAlways,
  kAlwaysFf,
  kAlwaysComb,
  kAlwaysLatch,
  kFinal
};

export enum class AssignmentKind : std::uint8_t { kBlocking, kNonblocking };

export enum class CaseKind : std::uint8_t { kCase, kCaseX, kCaseZ };

export enum class LoopKind : std::uint8_t {
  kFor,
  kWhile,
  kRepeat,
  kForeach,
  kForever
};

export enum class TimingControlKind : std::uint8_t { kEvent, kDelay };

export enum class WaitKind : std::uint8_t { kWait, kWaitOrder, kWaitFork };

export enum class ForkJoinKind : std::uint8_t { kJoin, kJoinAny, kJoinNone };

export enum class ProceduralContinuousAssignKind : std::uint8_t {
  kAssign,
  kDeassign,
  kForce,
  kRelease
};

export struct BeginEndBlockStatement {
  std::vector<StatementPtr> statements;
};

export struct AssignmentStatement {
  AssignmentKind kind{};
  ExpressionPtr left_hand_side;
  ExpressionPtr right_hand_side;
};

export struct IfElseStatement {
  ExpressionPtr condition;
  StatementPtr then_statement;
  StatementPtr else_statement;
};

export struct CaseStatement {
  CaseKind kind{};
  ExpressionPtr expression;
  std::span<Token const> items;
};

export struct LoopStatement {
  LoopKind kind{};
  std::span<Token const> control;
  StatementPtr body;
};

export struct TimingControlStatement {
  TimingControlKind kind{};
  std::span<Token const> control;
  StatementPtr statement;
};

export struct WaitStatement {
  WaitKind kind{};
  std::span<Token const> control;
  StatementPtr statement;
};

export struct ForkJoinStatement {
  ForkJoinKind kind{};
  std::vector<StatementPtr> statements;
};

export struct ProceduralContinuousAssignStatement {
  ProceduralContinuousAssignKind kind{};
  ExpressionPtr left_hand_side;
  ExpressionPtr right_hand_side;
};

export struct SystemTaskCallStatement {
  std::string_view name;
  std::vector<ExpressionPtr> arguments;
};

export struct UnsupportedStatement {
  std::span<Token const> tokens;
};

export using StatementNode =
    std::variant<std::monostate, BeginEndBlockStatement, AssignmentStatement,
                 IfElseStatement, CaseStatement, LoopStatement,
                 TimingControlStatement, WaitStatement, ForkJoinStatement,
                 ProceduralContinuousAssignStatement, SystemTaskCallStatement,
                 UnsupportedStatement>;

export struct Statement {
  StatementNode node;
  std::span<Token const> tokens;
};

export struct PackedRangeDimension {
  ExpressionPtr left;
  ExpressionPtr right;
  std::span<Token const> tokens;
};

export struct PackedSizeDimension {
  ExpressionPtr size;
  std::span<Token const> tokens;
};

export using PackedDimension =
    std::variant<PackedRangeDimension, PackedSizeDimension>;

export struct PortDeclaration : Declaration {
  PortDirection direction{};
};

export struct NetDeclaration : Declaration {
  NetType type;
  std::span<Token const> type_specifier;
  std::vector<PackedDimension> packed_dimensions;
};

export struct ParameterTypeDeclaration : Declaration {
  std::span<Token const> default_type;
};

export struct ParameterValueDeclaration : Declaration {
  std::span<Token const> type_specifier;
  ExpressionPtr default_value;
};

export using ParameterDeclaration =
    std::variant<ParameterTypeDeclaration, ParameterValueDeclaration>;

export struct ContinuousAssign {
  ExpressionPtr left_hand_side;
  ExpressionPtr right_hand_side;
};

export struct AlwaysBlock {
  ProceduralBlockKind kind{ProceduralBlockKind::kAlways};
  StatementPtr statement;
};

export struct InitialBlock {
  StatementPtr statement;
};

export struct FinalBlock {
  StatementPtr statement;
};

export struct GenerateIfExpression {
  ExpressionPtr condition;
  std::span<Token const> tokens;
};

export struct GenerateForExpression {
  ExpressionPtr initialization;
  ExpressionPtr condition;
  ExpressionPtr step;
  std::span<Token const> tokens;
};

export using GenerateExpression =
    std::variant<GenerateIfExpression, GenerateForExpression>;

export struct GenerateBlock {
  std::span<Token const> body;
  std::vector<GenerateExpression> expressions;
};

export struct ModuleInstantiation {
  std::string_view module_name;
  std::string_view instance_name;
  std::span<Token const> parameter_overrides;
  std::span<Token const> port_connections;
};

export struct UnsupportedModuleItem {
  std::string_view kind;
  std::span<Token const> tokens;
};

export using ModuleItem =
    std::variant<NetDeclaration, ContinuousAssign, AlwaysBlock, InitialBlock,
                 FinalBlock, GenerateBlock, ModuleInstantiation,
                 UnsupportedModuleItem>;

export struct ModuleDeclaration : Declaration {
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<ModuleItem> items;
};

export struct UnsupportedDesignElement {
  std::string_view kind;
  std::span<Token const> tokens;
};

/// @brief Top-level SystemVerilog design element.
export using DesignElement =
    std::variant<ModuleDeclaration, UnsupportedDesignElement>;

}  // namespace svt::model
