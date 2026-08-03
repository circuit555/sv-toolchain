// SPDX-License-Identifier: MIT

export module svt.model.ast;

import std;
import svt.model.token;

namespace svt::model {

export enum class PortDirection : std::uint8_t {
  kInput,
  kOutput,
  kInout,
  kRef
};

export enum class NetType : std::uint8_t {
  kWire, kLogic, kTri, kTri0, kTri1, kTriand, kTrior, kTrireg, kUwire,
  kWand, kWor, kSupply0, kSupply1, kInterconnect
};

export enum class VariableType : std::uint8_t {
  kReg,
  kInt,
  kInteger,
  kShortint,
  kLongint,
  kByte,
  kBit,
  kReal,
  kTime,
  kShortreal,
  kChandle,
  kRealtime,
  kEvent,
  kString
};

export enum class ParameterKind : std::uint8_t { kParameter, kLocalparam };

export enum class ModuleSourceKind : std::uint8_t { kModule, kMacromodule };

export enum class TypeDeclarationKind : std::uint8_t {
  kTypedef,
  kEnum,
  kStruct,
  kUnion,
  kNettype
};

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

export struct StreamingConcatenationExpression {
  std::string_view direction;
  std::span<Token const> slice_size;
  std::span<Token const> elements;
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
  struct Entry {
    std::span<Token const> key;
    ExpressionPtr value;
  };
  std::vector<ExpressionPtr> expressions;
  std::vector<Entry> entries;
};

export struct CastExpression {
  std::span<Token const> type_specifier;
  ExpressionPtr expression;
};

export struct MemberAccessExpression {
  ExpressionPtr base;
  std::string_view separator;
  std::string_view member;
};

export struct UnsupportedExpression {
  std::span<Token const> tokens;
};

export using ExpressionNode =
    std::variant<std::monostate, IdentifierExpression,
                 SystemIdentifierExpression, LiteralExpression, UnaryExpression,
                 BinaryExpression, ConditionalExpression, IndexExpression,
                 RangeSelectExpression, ConcatenationExpression,
                 StreamingConcatenationExpression,
                 ReplicationExpression, CallExpression,
                 AssignmentPatternExpression, CastExpression,
                 MemberAccessExpression,
                 UnsupportedExpression>;

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

export struct CaseItem {
  std::vector<ExpressionPtr> labels;
  StatementPtr statement;
  bool is_default{};
};

export struct CaseStatement {
  CaseKind kind{};
  ExpressionPtr expression;
  std::vector<CaseItem> items;
};

export struct WhileLoopControl {
  ExpressionPtr condition;
};

export struct RepeatLoopControl {
  ExpressionPtr count;
};

export struct ForLoopControl {
  ExpressionPtr initializer;
  ExpressionPtr condition;
  ExpressionPtr step;
};

export struct ForeachLoopControl {
  ExpressionPtr array_expression;
  std::vector<std::string_view> loop_variables;
};

export using LoopControl =
    std::variant<std::monostate, WhileLoopControl, RepeatLoopControl,
                 ForLoopControl, ForeachLoopControl>;

export struct LoopStatement {
  LoopKind kind{};
  LoopControl control;
  StatementPtr body;
};

export struct TimeLiteral {
  std::string_view magnitude;
  std::string_view unit;
  std::span<Token const> tokens;
};

export struct DelayControl {
  ExpressionPtr expression;
  std::optional<TimeLiteral> semantic_time;
};

export struct EventControl {
  std::vector<ExpressionPtr> events;
};

export using TimingControl = std::variant<DelayControl, EventControl>;

export struct TimingControlStatement {
  TimingControlKind kind{};
  TimingControl control;
  StatementPtr statement;
};

export struct WaitExpressionControl {
  ExpressionPtr expression;
};

export struct WaitOrderControl {
  std::vector<ExpressionPtr> events;
};

export using WaitControl =
    std::variant<std::monostate, WaitExpressionControl, WaitOrderControl>;

export struct WaitStatement {
  WaitKind kind{};
  WaitControl control;
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

export struct ProceduralDeclarationStatement {
  std::span<Token const> declaration;
};

export struct UnsupportedStatement {
  std::span<Token const> tokens;
};

export struct TokenPreservingStatement {
  std::string_view kind;
  std::span<Token const> tokens;
};

export struct ReturnStatement {
  ExpressionPtr expression;
};

export struct BreakStatement {
  std::string_view label;
};

export struct ContinueStatement {
  std::string_view label;
};

export struct DisableStatement {
  std::span<Token const> target;
};

export struct ExpectStatement {
  std::span<Token const> condition;
  std::span<Token const> action;
};

export using StatementNode =
    std::variant<std::monostate, BeginEndBlockStatement, AssignmentStatement,
                 IfElseStatement, CaseStatement, LoopStatement,
                 TimingControlStatement, WaitStatement, ForkJoinStatement,
                 ProceduralContinuousAssignStatement, SystemTaskCallStatement,
                 ProceduralDeclarationStatement,
                 ReturnStatement, BreakStatement, ContinueStatement,
                 DisableStatement, ExpectStatement, TokenPreservingStatement,
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

export enum class ModulePortKind : std::uint8_t {
  kImplicit,
  kExplicitNamed,
  kEmpty
};

export struct ModulePort : Declaration {
  ModulePortKind kind{};
  std::optional<PortDirection> direction;
  std::span<Token const> attributes;
  std::span<Token const> type_specifier;
  std::vector<std::span<Token const>> packed_dimensions;
  std::span<Token const> unpacked_dimensions;
  std::span<Token const> default_value;
  std::span<Token const> interface_type;
  std::span<Token const> tokens;
};

export struct NetDeclaration : Declaration {
  NetType type;
  std::vector<std::string_view> names;
  std::span<Token const> type_specifier;
  std::vector<PackedDimension> packed_dimensions;
};

export struct VariableDeclarator : Declaration {
  std::span<Token const> unpacked_dimensions;
  std::span<Token const> initializer;
  std::span<Token const> tokens;
};

export struct VariableDeclaration : Declaration {
  VariableType type;
  std::span<Token const> attributes;
  std::span<Token const> type_specifier;
  std::vector<PackedDimension> packed_dimensions;
  std::vector<VariableDeclarator> declarators;
  std::span<Token const> tokens;
};

export struct TypeDeclaration : Declaration {
  TypeDeclarationKind kind{TypeDeclarationKind::kTypedef};
  bool packed{false};
  bool tagged{false};
  bool forward{false};
  std::span<Token const> type_specifier;
  std::span<Token const> body;
  std::span<Token const> resolution_function;
  std::span<Token const> tokens;
};

export struct StructuredVariableDeclaration : Declaration {
  TypeDeclarationKind kind{TypeDeclarationKind::kStruct};
  bool packed{false};
  bool tagged{false};
  std::span<Token const> body;
  std::vector<VariableDeclarator> declarators;
  std::span<Token const> tokens;
};

export struct UserDefinedNetDeclaration : Declaration {
  std::span<Token const> type_specifier;
  std::span<Token const> unpacked_dimensions;
  std::span<Token const> tokens;
};

export struct ParameterTypeDeclaration : Declaration {
  ParameterKind kind{ParameterKind::kParameter};
  std::span<Token const> default_type;
};

export struct ParameterValueDeclaration : Declaration {
  ParameterKind kind{ParameterKind::kParameter};
  std::span<Token const> type_specifier;
  ExpressionPtr default_value;
};

export using ParameterDeclaration =
    std::variant<ParameterTypeDeclaration, ParameterValueDeclaration>;

export struct ContinuousAssign {
  ExpressionPtr left_hand_side;
  ExpressionPtr right_hand_side;
};

export struct ModuleInstantiation {
  std::string_view module_name;
  std::string_view instance_name;
  std::span<Token const> instance_dimensions;
  std::span<Token const> parameter_overrides;
  std::span<Token const> port_connections;
};

export struct ClassDeclaration : Declaration {
  enum class MemberKind : std::uint8_t {
    kField, kMethod, kConstraint, kType, kOther
  };
  struct Member {
    MemberKind kind{MemberKind::kOther};
    std::string_view name;
    std::span<Token const> tokens;
  };
  std::string_view lifetime;
  std::vector<ParameterDeclaration> parameters;
  std::vector<Member> members;
  std::span<Token const> extends;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct SubroutineDeclaration : Declaration {
  bool task{false};
  bool extern_declaration{false};
  std::string_view lifetime;
  std::span<Token const> return_type;
  std::span<Token const> ports;
  std::vector<std::span<Token const>> default_arguments;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct DpiDeclaration {
  bool export_declaration{false};
  std::string_view language;
  std::span<Token const> declaration;
  std::span<Token const> tokens;
};

export struct SpecifyBlock {
  enum class ItemKind : std::uint8_t { kSpecparam, kPath, kOther };
  struct Item {
    ItemKind kind{ItemKind::kOther};
    std::span<Token const> tokens;
  };
  std::vector<Item> structured_items;
  std::span<Token const> items;
  std::span<Token const> tokens;
};

export struct AssertionDeclaration {
  bool sequence{false};
  std::string_view name;
  std::span<Token const> header;
  std::span<Token const> ports;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct AssertionStatement {
  std::string_view kind;
  std::span<Token const> expression;
  std::span<Token const> action;
  std::span<Token const> tokens;
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

export struct GenerateItem;
export using GenerateItemPtr = std::unique_ptr<GenerateItem>;

export struct GenvarIdentifier {
  std::string_view name;
  ExpressionPtr initializer;
  std::span<Token const> tokens;
};

export struct GenvarDeclaration {
  std::vector<GenvarIdentifier> identifiers;
};

export struct GenerateFor {
  std::string_view loop_variable;
  ExpressionPtr initialization;
  ExpressionPtr condition;
  ExpressionPtr step;
  std::vector<GenerateItemPtr> body;
  std::string_view block_name;
};

export struct GenerateIf {
  ExpressionPtr condition;
  std::vector<GenerateItemPtr> then_body;
  std::vector<GenerateItemPtr> else_body;
  std::string_view then_block_name;
  std::string_view else_block_name;
};

export struct GenerateCaseItem {
  std::vector<ExpressionPtr> expressions;
  bool is_default{false};
  std::vector<GenerateItemPtr> body;
  std::span<Token const> tokens;
};

export struct GenerateCase {
  ExpressionPtr expression;
  std::vector<GenerateCaseItem> items;
};

export struct GenerateRegion {
  std::vector<GenerateItemPtr> items;
};

export struct UnsupportedGenerateItem {
  std::string_view kind;
};

export struct NullGenerateItem {};

export using GenerateItemNode =
    std::variant<GenvarDeclaration, GenerateFor, GenerateIf, GenerateCase,
                 GenerateRegion, ContinuousAssign, NetDeclaration,
                 ModuleInstantiation, NullGenerateItem, UnsupportedGenerateItem>;

export struct GenerateItem {
  GenerateItemNode node;
  std::span<Token const> tokens;
};

export enum class TimeDeclarationKind : std::uint8_t {
  kTimeUnit,
  kTimePrecision
};

export struct TimeDeclaration {
  TimeDeclarationKind kind{};
  std::span<Token const> time_value;
  std::span<Token const> precision_value;
  std::optional<TimeLiteral> semantic_time_value;
  std::optional<TimeLiteral> semantic_precision_value;
  std::span<Token const> tokens;
};

export struct ModportDeclaration : Declaration {
  std::span<Token const> ports;
  std::span<Token const> tokens;
};

export struct InterfaceSubroutineDeclaration {
  bool task{false};
  bool extern_declaration{false};
  std::span<Token const> tokens;
};

export struct DefaultClockingDeclaration {
  std::span<Token const> tokens;
};

export struct ClockingDeclaration {
  enum class ItemDirection : std::uint8_t { kInput, kOutput, kInout, kOther };
  struct Item {
    ItemDirection direction{ItemDirection::kOther};
    std::string_view name;
    std::span<Token const> skew;
    std::span<Token const> tokens;
  };
  bool global{false};
  bool default_clocking{false};
  std::string_view name;
  std::vector<Item> items;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct DefaultDisableIffDeclaration {
  std::span<Token const> tokens;
};

export struct CheckerDeclaration : Declaration {
  enum class ItemKind : std::uint8_t {
    kDeclaration, kClocking, kDefaultDisable, kAssertion, kGenerate, kOther
  };
  struct Item {
    ItemKind kind{ItemKind::kOther};
    std::string_view name;
    std::span<Token const> tokens;
  };
  std::vector<ModulePort> ports;
  std::vector<Item> items;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct TokenPreservingDeclaration {
  std::string_view kind;
  std::span<Token const> tokens;
};

export struct CovergroupDeclaration : Declaration {
  enum class ItemKind : std::uint8_t {
    kCoverpoint, kCross, kOption, kBin, kSample, kOther
  };
  struct Bin {
    std::string_view kind;
    std::string_view name;
    std::span<Token const> tokens;
  };
  struct Item {
    ItemKind kind{ItemKind::kOther};
    std::string_view name;
    std::vector<Bin> bins;
    std::span<Token const> expression;
    std::span<Token const> iff_condition;
    std::span<Token const> with_clause;
    bool transition{false};
    std::span<Token const> tokens;
  };
  std::vector<Item> items;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct ConfigDeclaration : Declaration {
  enum class ItemKind : std::uint8_t {
    kDesign, kDefaultLiblist, kCellUse, kInstanceLiblist, kOther
  };
  struct Item {
    ItemKind kind{ItemKind::kOther};
    std::span<Token const> subject;
    std::span<Token const> libraries;
    std::span<Token const> tokens;
  };
  std::vector<Item> items;
  std::span<Token const> body;
  std::span<Token const> tokens;
};

export struct InterfaceItemDeclaration {
  std::string_view kind;
  std::span<Token const> tokens;
};

export using InterfaceItem =
    std::variant<TimeDeclaration, ModulePort, VariableDeclaration,
                 TypeDeclaration, StructuredVariableDeclaration,
                 ModportDeclaration, InterfaceSubroutineDeclaration,
                 DefaultClockingDeclaration, ClockingDeclaration,
                 DefaultDisableIffDeclaration, InterfaceItemDeclaration>;

export struct InterfaceDeclaration : Declaration {
  std::string_view lifetime;
  std::vector<ParameterDeclaration> parameters;
  std::vector<ModulePort> ports;
  std::vector<InterfaceItem> items;
  std::span<Token const> tokens;
};

export struct PackageScopeName {
  std::string_view scope;
  std::string_view name;
  std::span<Token const> tokens;
};

export struct ImportDeclaration {
  std::vector<PackageScopeName> names;
  std::span<Token const> tokens;
};

export struct ExportDeclaration {
  std::vector<PackageScopeName> names;
  std::span<Token const> tokens;
};

export using PackageImportDeclaration = ImportDeclaration;
export using PackageExportDeclaration = ExportDeclaration;

// TODO(): why do we need unsupported stuff still?
export struct UnsupportedPackageItem {
  std::string_view kind;
  std::span<Token const> tokens;
};

export using PackageItem =
    std::variant<TimeDeclaration, ParameterDeclaration, TypeDeclaration,
                 ImportDeclaration, ExportDeclaration, TokenPreservingDeclaration,
                 UnsupportedPackageItem>;

export struct PackageDeclaration : Declaration {
  // TODO(): maybe std::optional<std::string_view> lifetime{};
  std::string_view lifetime;
  std::vector<PackageItem> items;
  std::span<Token const> tokens;
};

export struct UnsupportedModuleItem {
  std::string_view kind;
  std::span<Token const> tokens;
};

export using ModuleItem =
    std::variant<NetDeclaration, VariableDeclaration, ParameterDeclaration,
                 TypeDeclaration, StructuredVariableDeclaration,
                 UserDefinedNetDeclaration, ContinuousAssign, AlwaysBlock,
                 InitialBlock, FinalBlock, GenerateItem, ModuleInstantiation,
                 TimeDeclaration, ImportDeclaration, ClassDeclaration,
                 SubroutineDeclaration, SpecifyBlock, AssertionDeclaration,
                 AssertionStatement, ClockingDeclaration,
                 DefaultClockingDeclaration, DefaultDisableIffDeclaration,
                 DpiDeclaration,
                 CheckerDeclaration,
                 TokenPreservingDeclaration,
                 CovergroupDeclaration,
                 ConfigDeclaration,
                 UnsupportedModuleItem>;

export struct ModuleDeclaration : Declaration {
  ModuleSourceKind source_kind{ModuleSourceKind::kModule};
  std::string_view lifetime;
  std::vector<ParameterDeclaration> parameters;
  std::vector<ImportDeclaration> imports;
  std::vector<ModulePort> ports;
  std::vector<ModuleItem> items;
};

export struct ProgramDeclaration : Declaration {
  std::string_view lifetime;
  std::vector<ModulePort> ports;
  std::vector<ModuleItem> items;
  std::span<Token const> tokens;
};

export struct PrimitiveDeclaration : Declaration {
  std::vector<ModulePort> ports;
  std::span<Token const> table;
  std::span<Token const> initial_statement;
  std::span<Token const> tokens;
};

export struct UnsupportedDesignElement {
  std::string_view kind;
  std::span<Token const> tokens;
};

/// @brief Top-level SystemVerilog design element.
export using DesignElement =
    std::variant<ModuleDeclaration, ProgramDeclaration, PrimitiveDeclaration,
                 PackageDeclaration, InterfaceDeclaration, ClassDeclaration,
                 SubroutineDeclaration, TypeDeclaration, TimeDeclaration,
                 ImportDeclaration, ExportDeclaration, CheckerDeclaration,
                 TokenPreservingDeclaration, DpiDeclaration,
                 ConfigDeclaration,
                 UnsupportedDesignElement>;

}  // namespace svt::model
