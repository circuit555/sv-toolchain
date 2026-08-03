// SPDX-License-Identifier: MIT

#include <catch2/catch_all.hpp>

import std;
import svt.model.ast;
import svt.core.parser;

namespace {
using Parser = svt::core::Parser;
using ModuleDeclaration = svt::model::ModuleDeclaration;
using ModulePortKind = svt::model::ModulePortKind;
using PortDirection = svt::model::PortDirection;
using ParameterTypeDeclaration = svt::model::ParameterTypeDeclaration;
using ParameterValueDeclaration = svt::model::ParameterValueDeclaration;
using ParameterKind = svt::model::ParameterKind;
using ContinuousAssign = svt::model::ContinuousAssign;
using AlwaysBlock = svt::model::AlwaysBlock;
using InitialBlock = svt::model::InitialBlock;
using FinalBlock = svt::model::FinalBlock;
using GenvarDeclaration = svt::model::GenvarDeclaration;
using GenerateFor = svt::model::GenerateFor;
using GenerateIf = svt::model::GenerateIf;
using GenerateCase = svt::model::GenerateCase;
using GenerateRegion = svt::model::GenerateRegion;
using GenerateItem = svt::model::GenerateItem;
using ModuleInstantiation = svt::model::ModuleInstantiation;
using NetDeclaration = svt::model::NetDeclaration;
using NetType = svt::model::NetType;
using VariableDeclaration = svt::model::VariableDeclaration;
using VariableType = svt::model::VariableType;
using TypeDeclaration = svt::model::TypeDeclaration;
using TypeDeclarationKind = svt::model::TypeDeclarationKind;
using StructuredVariableDeclaration = svt::model::StructuredVariableDeclaration;
using UserDefinedNetDeclaration = svt::model::UserDefinedNetDeclaration;
using InterfaceDeclaration = svt::model::InterfaceDeclaration;
using ModportDeclaration = svt::model::ModportDeclaration;
using InterfaceSubroutineDeclaration =
    svt::model::InterfaceSubroutineDeclaration;
using DefaultClockingDeclaration = svt::model::DefaultClockingDeclaration;
using UnsupportedModuleItem = svt::model::UnsupportedModuleItem;
using UnsupportedDesignElement = svt::model::UnsupportedDesignElement;
using TimeDeclaration = svt::model::TimeDeclaration;
using TimeDeclarationKind = svt::model::TimeDeclarationKind;
using PackageDeclaration = svt::model::PackageDeclaration;
using PackageImportDeclaration = svt::model::PackageImportDeclaration;
using PackageExportDeclaration = svt::model::PackageExportDeclaration;
using ImportDeclaration = svt::model::ImportDeclaration;
using ExportDeclaration = svt::model::ExportDeclaration;
using UnsupportedPackageItem = svt::model::UnsupportedPackageItem;
using ParameterDeclaration = svt::model::ParameterDeclaration;
using Expression = svt::model::Expression;
using IdentifierExpression = svt::model::IdentifierExpression;
using LiteralExpression = svt::model::LiteralExpression;
using BinaryExpression = svt::model::BinaryExpression;
using IndexExpression = svt::model::IndexExpression;
using RangeSelectExpression = svt::model::RangeSelectExpression;
using ConcatenationExpression = svt::model::ConcatenationExpression;
using UnsupportedExpression = svt::model::UnsupportedExpression;
using PackedRangeDimension = svt::model::PackedRangeDimension;
using PackedSizeDimension = svt::model::PackedSizeDimension;
using Statement = svt::model::Statement;
using BeginEndBlockStatement = svt::model::BeginEndBlockStatement;
using AssignmentStatement = svt::model::AssignmentStatement;
using IfElseStatement = svt::model::IfElseStatement;
using CaseStatement = svt::model::CaseStatement;
using WhileLoopControl = svt::model::WhileLoopControl;
using RepeatLoopControl = svt::model::RepeatLoopControl;
using ForLoopControl = svt::model::ForLoopControl;
using ForeachLoopControl = svt::model::ForeachLoopControl;
using LoopStatement = svt::model::LoopStatement;
using DelayControl = svt::model::DelayControl;
using EventControl = svt::model::EventControl;
using TimingControlStatement = svt::model::TimingControlStatement;
using WaitExpressionControl = svt::model::WaitExpressionControl;
using WaitOrderControl = svt::model::WaitOrderControl;
using WaitStatement = svt::model::WaitStatement;
using ForkJoinStatement = svt::model::ForkJoinStatement;
using ProceduralContinuousAssignStatement =
    svt::model::ProceduralContinuousAssignStatement;
using SystemTaskCallStatement = svt::model::SystemTaskCallStatement;
using ProceduralBlockKind = svt::model::ProceduralBlockKind;
using AssignmentKind = svt::model::AssignmentKind;
using CaseKind = svt::model::CaseKind;
using LoopKind = svt::model::LoopKind;
using TimingControlKind = svt::model::TimingControlKind;
using WaitKind = svt::model::WaitKind;
using ForkJoinKind = svt::model::ForkJoinKind;
using ProceduralContinuousAssignKind =
    svt::model::ProceduralContinuousAssignKind;
using ProgramDeclaration = svt::model::ProgramDeclaration;
using PrimitiveDeclaration = svt::model::PrimitiveDeclaration;
using ModuleSourceKind = svt::model::ModuleSourceKind;
using ClassDeclaration = svt::model::ClassDeclaration;
using SubroutineDeclaration = svt::model::SubroutineDeclaration;
using SpecifyBlock = svt::model::SpecifyBlock;
using AssertionDeclaration = svt::model::AssertionDeclaration;
using AssertionStatement = svt::model::AssertionStatement;
using ClockingDeclaration = svt::model::ClockingDeclaration;
using DefaultDisableIffDeclaration = svt::model::DefaultDisableIffDeclaration;
using CheckerDeclaration = svt::model::CheckerDeclaration;
using TokenPreservingDeclaration = svt::model::TokenPreservingDeclaration;
using TokenPreservingStatement = svt::model::TokenPreservingStatement;
using ReturnStatement = svt::model::ReturnStatement;
using BreakStatement = svt::model::BreakStatement;
using ContinueStatement = svt::model::ContinueStatement;
using DisableStatement = svt::model::DisableStatement;
using ExpectStatement = svt::model::ExpectStatement;
using ProceduralDeclarationStatement = svt::model::ProceduralDeclarationStatement;
using CovergroupDeclaration = svt::model::CovergroupDeclaration;
using ConfigDeclaration = svt::model::ConfigDeclaration;
using CastExpression = svt::model::CastExpression;
using DpiDeclaration = svt::model::DpiDeclaration;
using MemberAccessExpression = svt::model::MemberAccessExpression;
using NullGenerateItem = svt::model::NullGenerateItem;

auto Lexemes(auto const& tokens) -> std::vector<std::string_view> {
  std::vector<std::string_view> result{};
  for (auto const& token : tokens) {
    result.push_back(token.lexeme);
  }
  return result;
}

template <typename T>
auto ExprAs(Expression const& expression) -> T const& {
  return std::get<T>(expression.node);
}

template <typename T>
auto StmtAs(Statement const& statement) -> T const& {
  return std::get<T>(statement.node);
}

template <typename T>
auto GenItemAs(svt::model::GenerateItem const& item) -> T const& {
  return std::get<T>(item.node);
}

template <typename T>
auto ParameterAs(svt::model::ModuleItem const& item) -> T const& {
  return std::get<T>(std::get<ParameterDeclaration>(item));
}

auto GenerateRegionBodyTokens(GenerateItem const& item)
    -> decltype(item.tokens) {
  if (item.tokens.size() < 2UZ) {
    return decltype(item.tokens){};
  }
  return {std::next(item.tokens.begin()), std::prev(item.tokens.end())};
}

auto ReadFixture(std::filesystem::path const& fixture_path) -> std::string {
  auto const test_path{std::filesystem::path{__FILE__}.parent_path()};
  std::ifstream file_stream{test_path / fixture_path,
                            std::ios::binary | std::ios::ate};
  REQUIRE(file_stream.is_open());

  std::string source{};
  source.resize(file_stream.tellg());
  file_stream.seekg(0);
  file_stream.read(source.data(), static_cast<std::streamsize>(source.size()));
  return source;
}
}  // namespace

TEST_CASE("Parse program and primitive declarations", "[parser]") {
  std::string src = R"(
    program automatic p(input clk);
      timeunit 1ns;
      integer value;
      initial value = 1;
    endprogram : p
    primitive udp(output y, input a);
      table
        0 : 1;
      endtable
      initial y = 0;
    endprimitive : udp
  )";

  Parser parser{std::move(src)};
  auto translation_unit = parser.Parse();
  REQUIRE(translation_unit.size() == 2);

  auto const& program = std::get<ProgramDeclaration>(translation_unit.at(0));
  REQUIRE(program.name == "p");
  REQUIRE(program.lifetime == "automatic");
  REQUIRE(program.ports.size() == 1);
  REQUIRE(program.items.size() == 3);

  auto const& primitive =
      std::get<PrimitiveDeclaration>(translation_unit.at(1));
  REQUIRE(primitive.name == "udp");
  REQUIRE(primitive.ports.size() == 2);
  REQUIRE_FALSE(primitive.table.empty());
  REQUIRE_FALSE(primitive.initial_statement.empty());

  Parser macro_parser{"macromodule legacy; endmodule : legacy"};
  auto macro_translation_unit = macro_parser.Parse();
  auto const& macromodule = std::get<svt::model::ModuleDeclaration>(
      macro_translation_unit.front());
  REQUIRE(macromodule.source_kind == ModuleSourceKind::kMacromodule);
}

TEST_CASE("Parse subroutine declarations", "[parser]") {
  Parser parser{std::string{R"(
    function int top(input int value); endfunction : top
    module m;
      task automatic work(input int value); endtask
      extern function void declared(input int value);
    endmodule
  )"}};
  auto translation_unit = parser.Parse();
  auto const& function_declaration =
      std::get<SubroutineDeclaration>(translation_unit.at(0));
  REQUIRE_FALSE(function_declaration.task);
  REQUIRE(function_declaration.name == "top");
  REQUIRE(function_declaration.body.empty());
  auto const& module = std::get<ModuleDeclaration>(translation_unit.at(1));
  REQUIRE(std::holds_alternative<SubroutineDeclaration>(module.items.at(0)));
  auto const& task = std::get<SubroutineDeclaration>(module.items.at(0));
  REQUIRE(task.task);
  REQUIRE(task.lifetime == "automatic");
  REQUIRE(std::get<SubroutineDeclaration>(module.items.at(1))
              .extern_declaration);
}

TEST_CASE("Parse specify blocks as isolated module items", "[parser]") {
  Parser parser{std::string{"module m; specify specparam t = 1; endspecify endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& specify = std::get<SpecifyBlock>(module.items.front());
  REQUIRE_FALSE(specify.items.empty());
  REQUIRE(specify.structured_items.size() == 1);
  REQUIRE(specify.structured_items.front().kind == SpecifyBlock::ItemKind::kSpecparam);
  REQUIRE(specify.tokens.front().lexeme == "specify");
}

TEST_CASE("Classify specify paths", "[parser]") {
  Parser parser{std::string{"module m; specify (a *> y) = (1:2:3); endspecify endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& specify = std::get<SpecifyBlock>(
      std::get<ModuleDeclaration>(translation_unit.front()).items.front());
  REQUIRE(specify.structured_items.size() == 1);
  REQUIRE(specify.structured_items.front().kind == SpecifyBlock::ItemKind::kPath);
}

TEST_CASE("Parse assertion declarations and statements", "[parser]") {
  Parser parser{std::string{"module m; property p; a |-> b; endproperty sequence s; a ##1 b; endsequence assert property (p); endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& property = std::get<AssertionDeclaration>(module.items.at(0));
  REQUIRE_FALSE(property.sequence);
  REQUIRE(property.name == "p");
  auto const& sequence = std::get<AssertionDeclaration>(module.items.at(1));
  REQUIRE(sequence.sequence);
  REQUIRE(sequence.name == "s");
  auto const& assertion = std::get<AssertionStatement>(module.items.at(2));
  REQUIRE(assertion.kind == "assert");
  REQUIRE_FALSE(assertion.expression.empty());
}

TEST_CASE("Parse clocking and default directives", "[parser]") {
  Parser parser{std::string{"module m; clocking cb @(posedge clk); input #1 a; output b; endclocking default clocking cb; default disable iff (reset); endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& clocking = std::get<ClockingDeclaration>(module.items.at(0));
  REQUIRE(clocking.name == "cb");
  REQUIRE_FALSE(clocking.body.empty());
  REQUIRE(clocking.items.size() == 2);
  REQUIRE(clocking.items.at(0).direction == ClockingDeclaration::ItemDirection::kInput);
  REQUIRE(clocking.items.at(0).name == "a");
  REQUIRE(clocking.items.at(0).skew.size() == 2);
  REQUIRE(clocking.items.at(1).direction == ClockingDeclaration::ItemDirection::kOutput);
  REQUIRE(std::get<ClockingDeclaration>(module.items.at(1)).default_clocking);
  REQUIRE(std::holds_alternative<DefaultDisableIffDeclaration>(module.items.at(2)));
}

TEST_CASE("Parse checker declarations", "[parser]") {
  Parser parser{std::string{"checker c(input clk); default clocking cb; assert property (p); endchecker : c"}};
  auto translation_unit = parser.Parse();
  auto const& checker = std::get<CheckerDeclaration>(translation_unit.front());
  REQUIRE(checker.name == "c");
  REQUIRE(checker.ports.size() == 1);
  REQUIRE_FALSE(checker.body.empty());
  REQUIRE(checker.items.size() == 2);
  REQUIRE(checker.items.at(0).kind == CheckerDeclaration::ItemKind::kDefaultDisable);
  REQUIRE(checker.items.at(1).kind == CheckerDeclaration::ItemKind::kAssertion);
}

TEST_CASE("Parse null generate items", "[parser]") {
  Parser parser{std::string{"module m; generate ; if (1) ; endgenerate endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& region = GenItemAs<GenerateRegion>(
      std::get<GenerateItem>(module.items.front()));
  REQUIRE(std::holds_alternative<NullGenerateItem>(region.items.front()->node));
}

TEST_CASE("Parse module instance arrays", "[parser]") {
  Parser parser{std::string{"module m; child inst[3:1][2:0](.a(), .b()); endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& instance = std::get<ModuleInstantiation>(module.items.front());
  REQUIRE(Lexemes(instance.instance_dimensions) ==
          std::vector<std::string_view>{"[", "3", ":", "1", "]", "[", "2", ":", "0", "]"});
}

TEST_CASE("Parse extended net types and declarators", "[parser]") {
  Parser parser{std::string{"module m; wor u, v; tri0 [3:0] bus; endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& wor = std::get<NetDeclaration>(module.items.at(0));
  REQUIRE(wor.type == NetType::kWor);
  REQUIRE(wor.names == std::vector<std::string_view>{"u", "v"});
  REQUIRE(std::get<NetDeclaration>(module.items.at(1)).type == NetType::kTri0);
}

TEST_CASE("Preserve primitive gate instances", "[parser]") {
  Parser parser{std::string{"module m; rcmos #1step (q, r, s, t); pullup (strong1) p1(a), p2(b); endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  REQUIRE(std::get<TokenPreservingDeclaration>(module.items.at(0)).kind == "rcmos");
  REQUIRE(std::get<TokenPreservingDeclaration>(module.items.at(1)).kind == "pullup");
}

TEST_CASE("Parse bind alias defparam and let declarations", "[parser]") {
  Parser parser{std::string{"bind target checker_inst ci(); module m; alias a = b; defparam m.W = 1; let inc(x) = x + 1; endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& bind = std::get<TokenPreservingDeclaration>(translation_unit.at(0));
  REQUIRE(bind.kind == "bind");
  auto const& module = std::get<ModuleDeclaration>(translation_unit.at(1));
  REQUIRE(std::get<TokenPreservingDeclaration>(module.items.at(0)).kind == "alias");
  REQUIRE(std::get<TokenPreservingDeclaration>(module.items.at(1)).kind == "defparam");
  REQUIRE(std::get<TokenPreservingDeclaration>(module.items.at(2)).kind == "let");
}

TEST_CASE("Parse token-preserving procedural controls", "[parser]") {
  Parser parser{std::string{"module m; initial begin return; break; continue; disable fork; end endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& initial = std::get<InitialBlock>(module.items.front());
  auto const& block = StmtAs<BeginEndBlockStatement>(*initial.statement);
  REQUIRE(StmtAs<ReturnStatement>(*block.statements.at(0)).expression == nullptr);
  REQUIRE(StmtAs<BreakStatement>(*block.statements.at(1)).label.empty());
  REQUIRE(StmtAs<ContinueStatement>(*block.statements.at(2)).label.empty());
  REQUIRE(StmtAs<DisableStatement>(*block.statements.at(3)).target.front().lexeme == "fork");
}

TEST_CASE("Parse structured procedural control statements", "[parser]") {
  Parser parser{std::string{"module m; initial begin return count; break: done; continue: next; expect (a) else b; end endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& block = StmtAs<BeginEndBlockStatement>(*std::get<InitialBlock>(module.items.front()).statement);
  REQUIRE(StmtAs<ReturnStatement>(*block.statements.at(0)).expression != nullptr);
  REQUIRE(StmtAs<BreakStatement>(*block.statements.at(1)).label == "done");
  REQUIRE(StmtAs<ContinueStatement>(*block.statements.at(2)).label == "next");
  auto const& expect = StmtAs<ExpectStatement>(*block.statements.at(3));
  REQUIRE(expect.condition.front().lexeme == "(");
  REQUIRE(expect.condition.size() >= 3);
}

TEST_CASE("Parse procedural declarations inside blocks", "[parser]") {
  Parser parser{std::string{"module m; initial begin int i = 1; logic ready; i = 2; end endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& block = StmtAs<BeginEndBlockStatement>(
      *std::get<InitialBlock>(module.items.front()).statement);
  REQUIRE(std::holds_alternative<ProceduralDeclarationStatement>(block.statements.at(0)->node));
  REQUIRE(std::holds_alternative<ProceduralDeclarationStatement>(block.statements.at(1)->node));
  REQUIRE(std::holds_alternative<AssignmentStatement>(block.statements.at(2)->node));
}

TEST_CASE("Parse covergroup declarations", "[parser]") {
  Parser parser{std::string{"module m; covergroup cg @(posedge clk); cp: coverpoint data; endgroup : cg endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& covergroup = std::get<CovergroupDeclaration>(module.items.front());
  REQUIRE(covergroup.name == "cg");
  REQUIRE_FALSE(covergroup.body.empty());
  REQUIRE(covergroup.items.size() == 1);
  REQUIRE(covergroup.items.front().kind == CovergroupDeclaration::ItemKind::kCoverpoint);
  REQUIRE(covergroup.items.front().name == "cp");
}

TEST_CASE("Parse covergroup conditions and transition bins", "[parser]") {
  Parser parser{std::string{"module m; covergroup cg; c: coverpoint x iff (en) { bins t = (1 => 2); bins w = x with (item > 0); } endgroup endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& covergroup = std::get<CovergroupDeclaration>(module.items.front());
  REQUIRE(covergroup.items.size() == 1);
  auto const& item = covergroup.items.front();
  REQUIRE(item.name == "c");
  REQUIRE_FALSE(item.expression.empty());
  REQUIRE_FALSE(item.iff_condition.empty());
  REQUIRE(item.transition);
  REQUIRE_FALSE(item.with_clause.empty());
  REQUIRE(item.bins.size() == 2);
}

TEST_CASE("Parse inside and matches expression operators", "[parser]") {
  Parser parser{std::string{"module m; initial begin x = a inside {1, 2}; y = a matches b; end endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& block = StmtAs<BeginEndBlockStatement>(
      *std::get<InitialBlock>(module.items.front()).statement);
  auto const& inside = StmtAs<AssignmentStatement>(*block.statements.at(0));
  REQUIRE(std::get<BinaryExpression>(inside.right_hand_side->node).operator_lexeme == "inside");
  auto const& matches = StmtAs<AssignmentStatement>(*block.statements.at(1));
  REQUIRE(std::get<BinaryExpression>(matches.right_hand_side->node).operator_lexeme == "matches");
}

TEST_CASE("Model semantic delay time literals", "[parser]") {
  Parser parser{std::string{"module m; initial #5ns x = 1; endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& timing = StmtAs<TimingControlStatement>(
      *std::get<InitialBlock>(module.items.front()).statement);
  auto const& delay = std::get<DelayControl>(timing.control);
  REQUIRE(delay.semantic_time.has_value());
  REQUIRE(delay.semantic_time->magnitude == "5");
  REQUIRE(delay.semantic_time->unit == "ns");
}

TEST_CASE("Parse config declarations", "[parser]") {
  Parser parser{std::string{"config cfg; design top; default liblist work; cell top use work.top; endconfig : cfg"}};
  auto translation_unit = parser.Parse();
  auto const& config = std::get<ConfigDeclaration>(translation_unit.front());
  REQUIRE(config.name == "cfg");
  REQUIRE_FALSE(config.body.empty());
  REQUIRE(config.items.size() == 3);
  REQUIRE(config.items.at(0).kind == ConfigDeclaration::ItemKind::kDesign);
  REQUIRE(config.items.at(1).kind == ConfigDeclaration::ItemKind::kDefaultLiblist);
  REQUIRE(config.items.at(2).kind == ConfigDeclaration::ItemKind::kCellUse);
  REQUIRE(config.items.at(0).subject.front().lexeme == "top");
  REQUIRE(config.items.at(1).libraries.front().lexeme == "work");
  REQUIRE(config.items.at(2).subject.front().lexeme == "top");
  REQUIRE(config.items.at(2).libraries.size() == 3);
  REQUIRE(config.items.at(2).libraries.at(0).lexeme == "work");
  REQUIRE(config.items.at(2).libraries.at(1).lexeme == ".");
  REQUIRE(config.items.at(2).libraries.at(2).lexeme == "top");
}

TEST_CASE("Model semantic time literals", "[parser]") {
  Parser parser{std::string{"timeunit 10ns / 1ps;"}};
  auto translation_unit = parser.Parse();
  auto const& time = std::get<TimeDeclaration>(translation_unit.front());
  REQUIRE(time.semantic_time_value->magnitude == "10");
  REQUIRE(time.semantic_time_value->unit == "ns");
  REQUIRE(time.semantic_precision_value->magnitude == "1");
  REQUIRE(time.semantic_precision_value->unit == "ps");
}

TEST_CASE("Parse expression casts", "[parser]") {
  Parser parser{std::string{"module m; assign y = int'(x + 1); endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& module = std::get<ModuleDeclaration>(translation_unit.front());
  auto const& assign = std::get<ContinuousAssign>(module.items.front());
  auto const& cast = ExprAs<CastExpression>(*assign.right_hand_side);
  REQUIRE(Lexemes(cast.type_specifier) == std::vector<std::string_view>{"int"});
  REQUIRE(Lexemes(cast.expression->tokens) ==
          std::vector<std::string_view>{"x", "+", "1"});
}

TEST_CASE("Parse DPI declarations and subroutine defaults", "[parser]") {
  Parser parser{std::string{"import \"DPI-C\" function void dpi(input int x); export \"DPI-C\" function void dpo(input int x); function void f(input int x = 1); endfunction"}};
  auto translation_unit = parser.Parse();
  REQUIRE(std::get<DpiDeclaration>(translation_unit.at(0)).language == "DPI-C");
  REQUIRE(std::get<DpiDeclaration>(translation_unit.at(1)).export_declaration);
  auto const& function = std::get<SubroutineDeclaration>(translation_unit.at(2));
  REQUIRE(Lexemes(function.ports) ==
          std::vector<std::string_view>{"(", "input", "int", "x", "=", "1"});
  REQUIRE(function.default_arguments.size() == 1);
  REQUIRE(Lexemes(function.default_arguments.front()) ==
          std::vector<std::string_view>{"1"});
}

TEST_CASE("Parse member and scoped expression access", "[parser]") {
  Parser parser{std::string{"module m; assign y = obj.field + pkg::value; endmodule"}};
  auto translation_unit = parser.Parse();
  auto const& assign = std::get<ContinuousAssign>(
      std::get<ModuleDeclaration>(translation_unit.front()).items.front());
  auto const& binary = ExprAs<BinaryExpression>(*assign.right_hand_side);
  auto const& member = ExprAs<MemberAccessExpression>(*binary.left);
  REQUIRE(member.member == "field");
  auto const& scoped = ExprAs<MemberAccessExpression>(*binary.right);
  REQUIRE(scoped.separator == "::");
  REQUIRE(scoped.member == "value");
}

TEST_CASE("Parse all.sv regression fixture", "[parser][regression]") {
  auto const fixture_path{std::filesystem::path{__FILE__}.parent_path() /
                          "all.sv"};
  std::ifstream file_stream{fixture_path, std::ios::binary | std::ios::ate};
  REQUIRE(file_stream.is_open());
  std::string source{};
  source.resize(file_stream.tellg());
  file_stream.seekg(0);
  file_stream.read(source.data(), static_cast<std::streamsize>(source.size()));
  Parser parser{std::move(source)};
  auto translation_unit = parser.Parse();
  REQUIRE(translation_unit.size() > 1);
}

TEST_CASE("Parse generic module parameters", "[parser]") {
  std::string src = R"(
    module foo #(
      parameter logic [7:0] MASK = WIDTH - 1, ALT_MASK = 2,
      parameter type T = logic [3:0], U = bit,
      parameter WIDTH = 8
    )
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.parameters.size() == 5);

  auto const& mask_parameter{
      std::get<ParameterValueDeclaration>(module_declaration.parameters.at(0))};
  REQUIRE(mask_parameter.name == "MASK");
  REQUIRE(Lexemes(mask_parameter.type_specifier) ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(Lexemes(mask_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"WIDTH", "-", "1"});
  auto const& mask_default{
      ExprAs<BinaryExpression>(*mask_parameter.default_value)};
  REQUIRE(mask_default.operator_lexeme == "-");
  REQUIRE(ExprAs<IdentifierExpression>(*mask_default.left).name == "WIDTH");
  REQUIRE(ExprAs<LiteralExpression>(*mask_default.right).value == "1");

  auto const& continued_mask_parameter{
      std::get<ParameterValueDeclaration>(module_declaration.parameters.at(1))};
  REQUIRE(continued_mask_parameter.name == "ALT_MASK");
  REQUIRE(Lexemes(continued_mask_parameter.type_specifier) ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(Lexemes(continued_mask_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"2"});

  auto const& type_parameter{
      std::get<ParameterTypeDeclaration>(module_declaration.parameters.at(2))};
  REQUIRE(type_parameter.name == "T");
  REQUIRE(Lexemes(type_parameter.default_type) ==
          std::vector<std::string_view>{"logic", "[", "3", ":", "0", "]"});

  auto const& continued_type_parameter{
      std::get<ParameterTypeDeclaration>(module_declaration.parameters.at(3))};
  REQUIRE(continued_type_parameter.name == "U");
  REQUIRE(Lexemes(continued_type_parameter.default_type) ==
          std::vector<std::string_view>{"bit"});

  auto const& implicit_value_parameter{
      std::get<ParameterValueDeclaration>(module_declaration.parameters.at(4))};
  REQUIRE(implicit_value_parameter.name == "WIDTH");
  REQUIRE(implicit_value_parameter.type_specifier.empty());
  REQUIRE(Lexemes(implicit_value_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"8"});
}

TEST_CASE("Parse module parameter declarations", "[parser]") {
  std::string src = R"(
    module foo ();
      parameter int A = 1, B = 2;
      parameter type T = logic[3:0], U = bit;
      localparam int C = A + B, D = 4;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto const translation_unit{parser.Parse()};
  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 6);

  auto const& a_parameter{
      ParameterAs<ParameterValueDeclaration>(module_declaration.items.at(0))};
  REQUIRE(a_parameter.kind == ParameterKind::kParameter);
  REQUIRE(a_parameter.name == "A");
  REQUIRE(Lexemes(a_parameter.type_specifier) ==
          std::vector<std::string_view>{"int"});
  REQUIRE(Lexemes(a_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"1"});

  auto const& b_parameter{
      ParameterAs<ParameterValueDeclaration>(module_declaration.items.at(1))};
  REQUIRE(b_parameter.name == "B");
  REQUIRE(Lexemes(b_parameter.type_specifier) ==
          std::vector<std::string_view>{"int"});

  auto const& t_parameter{
      ParameterAs<ParameterTypeDeclaration>(module_declaration.items.at(2))};
  REQUIRE(t_parameter.name == "T");
  REQUIRE(t_parameter.kind == ParameterKind::kParameter);
  REQUIRE(Lexemes(t_parameter.default_type) ==
          std::vector<std::string_view>{"logic", "[", "3", ":", "0", "]"});

  auto const& u_parameter{
      ParameterAs<ParameterTypeDeclaration>(module_declaration.items.at(3))};
  REQUIRE(u_parameter.name == "U");
  REQUIRE(Lexemes(u_parameter.default_type) ==
          std::vector<std::string_view>{"bit"});

  auto const& c_parameter{
      ParameterAs<ParameterValueDeclaration>(module_declaration.items.at(4))};
  REQUIRE(c_parameter.kind == ParameterKind::kLocalparam);
  REQUIRE(c_parameter.name == "C");
  REQUIRE(Lexemes(c_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"A", "+", "B"});

  auto const& d_parameter{
      ParameterAs<ParameterValueDeclaration>(module_declaration.items.at(5))};
  REQUIRE(d_parameter.kind == ParameterKind::kLocalparam);
  REQUIRE(d_parameter.name == "D");
}

TEST_CASE("Parse top-level time declarations", "[parser]") {
  std::string src = R"(
    timeunit 1ns / 1ps;
    timeprecision 1ps;

    module foo ();
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 3);

  auto const& timeunit_declaration{
      std::get<TimeDeclaration>(translation_unit.at(0))};
  REQUIRE(timeunit_declaration.kind == TimeDeclarationKind::kTimeUnit);
  REQUIRE(Lexemes(timeunit_declaration.time_value) ==
          std::vector<std::string_view>{"1ns"});
  REQUIRE(Lexemes(timeunit_declaration.precision_value) ==
          std::vector<std::string_view>{"1ps"});
  REQUIRE(Lexemes(timeunit_declaration.tokens) ==
          std::vector<std::string_view>{"timeunit", "1ns", "/", "1ps", ";"});

  auto const& timeprecision_declaration{
      std::get<TimeDeclaration>(translation_unit.at(1))};
  REQUIRE(timeprecision_declaration.kind ==
          TimeDeclarationKind::kTimePrecision);
  REQUIRE(Lexemes(timeprecision_declaration.time_value) ==
          std::vector<std::string_view>{"1ps"});
  REQUIRE(timeprecision_declaration.precision_value.empty());
  REQUIRE(Lexemes(timeprecision_declaration.tokens) ==
          std::vector<std::string_view>{"timeprecision", "1ps", ";"});

  REQUIRE(std::get<ModuleDeclaration>(translation_unit.at(2)).name == "foo");
}

TEST_CASE("Parse module time declarations", "[parser]") {
  std::string src = R"(
    module foo ();
      timeunit 1ns / 1ps;
      timeprecision 1ps;
      logic done;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 3);

  auto const& timeunit_declaration{
      std::get<TimeDeclaration>(module_declaration.items.at(0))};
  REQUIRE(timeunit_declaration.kind == TimeDeclarationKind::kTimeUnit);
  REQUIRE(Lexemes(timeunit_declaration.time_value) ==
          std::vector<std::string_view>{"1ns"});
  REQUIRE(Lexemes(timeunit_declaration.precision_value) ==
          std::vector<std::string_view>{"1ps"});

  auto const& timeprecision_declaration{
      std::get<TimeDeclaration>(module_declaration.items.at(1))};
  REQUIRE(timeprecision_declaration.kind ==
          TimeDeclarationKind::kTimePrecision);
  REQUIRE(Lexemes(timeprecision_declaration.time_value) ==
          std::vector<std::string_view>{"1ps"});
  REQUIRE(timeprecision_declaration.precision_value.empty());

  auto const& done_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(2))};
  REQUIRE(done_declaration.name == "done");
}

TEST_CASE("Parse package declarations", "[parser]") {
  std::string src = R"(
    (* foo = 1 *) package static p;
      timeunit 1ns;
      parameter int x = 1;
      parameter type y_t = logic[x:0];
      import q::r;
      export *::*;
      program; endprogram
    endpackage

    module foo ();
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 2);

  auto const& package_declaration{
      std::get<PackageDeclaration>(translation_unit.at(0))};
  REQUIRE(package_declaration.name == "p");
  REQUIRE(package_declaration.lifetime == "static");
  REQUIRE(package_declaration.items.size() == 6);
  REQUIRE(Lexemes(package_declaration.tokens).front() == "(");

  auto const& time_declaration{
      std::get<TimeDeclaration>(package_declaration.items.at(0))};
  REQUIRE(time_declaration.kind == TimeDeclarationKind::kTimeUnit);
  REQUIRE(Lexemes(time_declaration.time_value) ==
          std::vector<std::string_view>{"1ns"});

  auto const& value_parameter{
      std::get<ParameterDeclaration>(package_declaration.items.at(1))};
  auto const& x_parameter{std::get<ParameterValueDeclaration>(value_parameter)};
  REQUIRE(x_parameter.name == "x");
  REQUIRE(Lexemes(x_parameter.type_specifier) ==
          std::vector<std::string_view>{"int"});
  REQUIRE(Lexemes(x_parameter.default_value->tokens) ==
          std::vector<std::string_view>{"1"});

  auto const& type_parameter{
      std::get<ParameterDeclaration>(package_declaration.items.at(2))};
  auto const& y_parameter{std::get<ParameterTypeDeclaration>(type_parameter)};
  REQUIRE(y_parameter.name == "y_t");
  REQUIRE(Lexemes(y_parameter.default_type) ==
          std::vector<std::string_view>{"logic", "[", "x", ":", "0", "]"});

  REQUIRE(Lexemes(std::get<PackageImportDeclaration>(
                      package_declaration.items.at(3))
                      .tokens) ==
          std::vector<std::string_view>{"import", "q", "::", "r", ";"});
  auto const& import_declaration{
      std::get<PackageImportDeclaration>(package_declaration.items.at(3))};
  REQUIRE(import_declaration.names.size() == 1);
  REQUIRE(import_declaration.names.front().scope == "q");
  REQUIRE(import_declaration.names.front().name == "r");

  REQUIRE(Lexemes(std::get<PackageExportDeclaration>(
                      package_declaration.items.at(4))
                      .tokens) ==
          std::vector<std::string_view>{"export", "*", "::", "*", ";"});
  auto const& export_declaration{
      std::get<PackageExportDeclaration>(package_declaration.items.at(4))};
  REQUIRE(export_declaration.names.size() == 1);
  REQUIRE(export_declaration.names.front().scope == "*");
  REQUIRE(export_declaration.names.front().name == "*");

  auto const& preserved_item{
      std::get<TokenPreservingDeclaration>(package_declaration.items.at(5))};
  REQUIRE(preserved_item.kind == "program");

  REQUIRE(std::get<ModuleDeclaration>(translation_unit.at(1)).name == "foo");
}

TEST_CASE("Parse import declarations", "[parser]") {
  std::string src = R"(
    import p::x, p::*;

    module foo import p::*, q::r; ();
      import p::x;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 2);

  auto const& top_level_import{
      std::get<ImportDeclaration>(translation_unit.at(0))};
  REQUIRE(top_level_import.names.size() == 2);
  REQUIRE(top_level_import.names.at(0).scope == "p");
  REQUIRE(top_level_import.names.at(0).name == "x");
  REQUIRE(top_level_import.names.at(1).scope == "p");
  REQUIRE(top_level_import.names.at(1).name == "*");

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.at(1))};
  REQUIRE(module_declaration.imports.size() == 1);
  REQUIRE(module_declaration.imports.front().names.size() == 2);
  REQUIRE(module_declaration.imports.front().names.at(0).scope == "p");
  REQUIRE(module_declaration.imports.front().names.at(0).name == "*");
  REQUIRE(module_declaration.imports.front().names.at(1).scope == "q");
  REQUIRE(module_declaration.imports.front().names.at(1).name == "r");

  REQUIRE(module_declaration.items.size() == 1);
  auto const& module_item_import{
      std::get<ImportDeclaration>(module_declaration.items.front())};
  REQUIRE(module_item_import.names.size() == 1);
  REQUIRE(module_item_import.names.front().scope == "p");
  REQUIRE(module_item_import.names.front().name == "x");
}

TEST_CASE("Parse compilation-unit class declarations", "[parser]") {
  std::string src = R"(
    timeunit 1ns / 1ps;

    package p;
      parameter int x = 1;
    endpackage

    class C;
      int i;
    endclass

    module foo ();
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 4);
  REQUIRE(std::get<TimeDeclaration>(translation_unit.at(0)).kind ==
          TimeDeclarationKind::kTimeUnit);
  REQUIRE(std::get<PackageDeclaration>(translation_unit.at(1)).name == "p");
  auto const& class_declaration =
      std::get<ClassDeclaration>(translation_unit.at(2));
  REQUIRE(class_declaration.name == "C");
  REQUIRE_FALSE(class_declaration.body.empty());
  REQUIRE(class_declaration.members.size() == 1);
  REQUIRE(class_declaration.members.front().kind ==
          ClassDeclaration::MemberKind::kField);
  REQUIRE(class_declaration.members.front().name == "i");
  REQUIRE(std::get<ModuleDeclaration>(translation_unit.at(3)).name == "foo");
}

TEST_CASE("Parse module ports", "[parser]") {
  std::string src = R"(
    module foo (
      input logic clk,
      output logic [7:0] data,
      ready
    )
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.ports.size() == 3);

  auto const& clk_port{module_declaration.ports.at(0)};
  REQUIRE(clk_port.name == "clk");
  REQUIRE(clk_port.direction == PortDirection::kInput);

  auto const& data_port{module_declaration.ports.at(1)};
  REQUIRE(data_port.name == "data");
  REQUIRE(data_port.direction == PortDirection::kOutput);
  REQUIRE(Lexemes(data_port.type_specifier) ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(data_port.packed_dimensions.size() == 1);
  REQUIRE(Lexemes(data_port.packed_dimensions.front()) ==
          std::vector<std::string_view>{"[", "7", ":", "0", "]"});

  auto const& ready_port{module_declaration.ports.at(2)};
  REQUIRE(ready_port.name == "ready");
  REQUIRE(ready_port.direction == PortDirection::kOutput);
}

TEST_CASE("Parse inout module ports", "[parser]") {
  std::string src = R"(
    module foo (
      inout wire pad
    )
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.ports.size() == 1);

  auto const& pad_port{module_declaration.ports.front()};
  REQUIRE(pad_port.name == "pad");
  REQUIRE(pad_port.direction == PortDirection::kInout);
  REQUIRE(Lexemes(pad_port.type_specifier) ==
          std::vector<std::string_view>{"wire"});
}

TEST_CASE("Parse rich module headers", "[parser]") {
  std::string src = R"(
    module automatic m1 import p::*, p::x; #(int i = 1)
      (a, b, , .c({a, b[0]}));
      input a;
      output [1:0] b;
    endmodule

    module m2 #(
      parameter i = 1,
      localparam j = i,
      parameter type x_t = bit
    )
      (input int a[], (* bar = "asdf" *) output logic b = 1, ref c,
       interface.mod d, .e());
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 2);

  auto const& m1{std::get<ModuleDeclaration>(translation_unit.at(0))};
  REQUIRE(m1.name == "m1");
  REQUIRE(m1.lifetime == "automatic");
  REQUIRE(m1.parameters.size() == 1);
  REQUIRE(m1.imports.size() == 1);
  REQUIRE(m1.imports.front().names.size() == 2);
  REQUIRE(m1.ports.size() == 4);
  REQUIRE(m1.ports.at(0).kind == ModulePortKind::kImplicit);
  REQUIRE(m1.ports.at(0).name == "a");
  REQUIRE(m1.ports.at(0).direction == PortDirection::kInput);
  REQUIRE(m1.ports.at(1).kind == ModulePortKind::kImplicit);
  REQUIRE(m1.ports.at(1).name == "b");
  REQUIRE(m1.ports.at(1).direction == PortDirection::kOutput);
  REQUIRE(m1.ports.at(2).kind == ModulePortKind::kEmpty);
  REQUIRE(m1.ports.at(2).name.empty());
  REQUIRE(not m1.ports.at(2).direction.has_value());
  REQUIRE(m1.ports.at(3).kind == ModulePortKind::kExplicitNamed);
  REQUIRE(m1.ports.at(3).name == "c");
  REQUIRE(not m1.ports.at(3).direction.has_value());
  REQUIRE(m1.items.empty());

  auto const& m2{std::get<ModuleDeclaration>(translation_unit.at(1))};
  REQUIRE(m2.name == "m2");
  REQUIRE(m2.lifetime.empty());
  REQUIRE(m2.parameters.size() == 3);
  REQUIRE(m2.ports.size() == 5);
  REQUIRE(m2.ports.at(0).kind == ModulePortKind::kImplicit);
  REQUIRE(m2.ports.at(0).name == "a");
  REQUIRE(m2.ports.at(0).direction == PortDirection::kInput);
  REQUIRE(Lexemes(m2.ports.at(0).type_specifier) ==
          std::vector<std::string_view>{"int"});
  REQUIRE(m2.ports.at(0).packed_dimensions.empty());
  REQUIRE(Lexemes(m2.ports.at(0).unpacked_dimensions) ==
          std::vector<std::string_view>{"[", "]"});
  REQUIRE(m2.ports.at(1).kind == ModulePortKind::kImplicit);
  REQUIRE(m2.ports.at(1).name == "b");
  REQUIRE(m2.ports.at(1).direction == PortDirection::kOutput);
  REQUIRE(
      Lexemes(m2.ports.at(1).attributes) ==
      std::vector<std::string_view>{"(", "*", "bar", "=", "asdf", "*", ")"});
  REQUIRE(Lexemes(m2.ports.at(1).type_specifier) ==
          std::vector<std::string_view>{"logic"});
  REQUIRE(Lexemes(m2.ports.at(1).default_value) ==
          std::vector<std::string_view>{"1"});
  REQUIRE(m2.ports.at(2).kind == ModulePortKind::kImplicit);
  REQUIRE(m2.ports.at(2).name == "c");
  REQUIRE(m2.ports.at(2).direction == PortDirection::kRef);
  REQUIRE(m2.ports.at(3).kind == ModulePortKind::kImplicit);
  REQUIRE(m2.ports.at(3).name == "d");
  REQUIRE(not m2.ports.at(3).direction.has_value());
  REQUIRE(Lexemes(m2.ports.at(3).interface_type) ==
          std::vector<std::string_view>{"interface", ".", "mod"});
  REQUIRE(m2.ports.at(4).kind == ModulePortKind::kExplicitNamed);
  REQUIRE(m2.ports.at(4).name == "e");
  REQUIRE(not m2.ports.at(4).direction.has_value());
}

TEST_CASE("Parse module parameters followed by ports", "[parser]") {
  std::string src = R"(
    module foo #(
      parameter WIDTH = 8
    ) (
      input clk,
      output [WIDTH-1:0] data
    )
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.parameters.size() == 1);
  REQUIRE(module_declaration.ports.size() == 2);

  auto const& width_parameter{
      std::get<ParameterValueDeclaration>(module_declaration.parameters.at(0))};
  REQUIRE(width_parameter.name == "WIDTH");

  auto const& clk_port{module_declaration.ports.at(0)};
  REQUIRE(clk_port.name == "clk");
  REQUIRE(clk_port.direction == PortDirection::kInput);

  auto const& data_port{module_declaration.ports.at(1)};
  REQUIRE(data_port.name == "data");
  REQUIRE(data_port.direction == PortDirection::kOutput);
}

TEST_CASE("Parse complete module declaration with body", "[parser]") {
  std::string src = R"(
    module foo #(parameter int N = 8) ();
      wire [N-1 : 0] bus;
      wire [8] word;
      logic ready;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.parameters.size() == 1);
  REQUIRE(module_declaration.ports.empty());
  REQUIRE(module_declaration.items.size() == 3);

  auto const& bus_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(0))};
  REQUIRE(bus_declaration.name == "bus");
  REQUIRE(bus_declaration.type == NetType::kWire);
  REQUIRE(Lexemes(bus_declaration.type_specifier) ==
          std::vector<std::string_view>{"[", "N", "-", "1", ":", "0", "]"});
  REQUIRE(bus_declaration.packed_dimensions.size() == 1);
  auto const& bus_dimension{std::get<PackedRangeDimension>(
      bus_declaration.packed_dimensions.front())};
  auto const& dimension_left{ExprAs<BinaryExpression>(*bus_dimension.left)};
  REQUIRE(dimension_left.operator_lexeme == "-");
  REQUIRE(ExprAs<IdentifierExpression>(*dimension_left.left).name == "N");
  REQUIRE(ExprAs<LiteralExpression>(*bus_dimension.right).value == "0");

  auto const& ready_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(2))};
  REQUIRE(ready_declaration.name == "ready");
  REQUIRE(ready_declaration.type == NetType::kLogic);
  REQUIRE(ready_declaration.type_specifier.empty());

  auto const& word_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(1))};
  REQUIRE(word_declaration.name == "word");
  REQUIRE(Lexemes(word_declaration.type_specifier) ==
          std::vector<std::string_view>{"[", "8", "]"});
  REQUIRE(word_declaration.packed_dimensions.size() == 1);
  auto const& word_dimension{std::get<PackedSizeDimension>(
      word_declaration.packed_dimensions.front())};
  REQUIRE(ExprAs<LiteralExpression>(*word_dimension.size).value == "8");
}

TEST_CASE("Parse variable declarations", "[parser]") {
  std::string src = R"(
    module foo ();
      (* keep = 1 *) reg [3:0] r, s[2] = 0;
      int i[2], j = 1;
      integer integer_value;
      shortint short_value;
      longint long_value;
      byte byte_value;
      bit bit_value;
      real real_value;
      time time_value;
      shortreal shortreal_value;
      chandle chandle_value;
      realtime realtime_value;
      event event_value;
      string string_value;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto const translation_unit{parser.Parse()};
  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 14);

  auto const& reg_declaration{
      std::get<VariableDeclaration>(module_declaration.items.at(0))};
  REQUIRE(reg_declaration.type == VariableType::kReg);
  REQUIRE(Lexemes(reg_declaration.attributes) ==
          std::vector<std::string_view>{"(", "*", "keep", "=", "1", "*", ")"});
  REQUIRE(Lexemes(reg_declaration.type_specifier) ==
          std::vector<std::string_view>{"[", "3", ":", "0", "]"});
  REQUIRE(reg_declaration.declarators.size() == 2);
  REQUIRE(reg_declaration.declarators.at(0).name == "r");
  REQUIRE(reg_declaration.declarators.at(1).name == "s");
  REQUIRE(Lexemes(reg_declaration.declarators.at(1).unpacked_dimensions) ==
          std::vector<std::string_view>{"[", "2", "]"});
  REQUIRE(Lexemes(reg_declaration.declarators.at(1).initializer) ==
          std::vector<std::string_view>{"0"});

  auto const& int_declaration{
      std::get<VariableDeclaration>(module_declaration.items.at(1))};
  REQUIRE(int_declaration.type == VariableType::kInt);
  REQUIRE(int_declaration.declarators.size() == 2);
  REQUIRE(Lexemes(int_declaration.declarators.at(0).unpacked_dimensions) ==
          std::vector<std::string_view>{"[", "2", "]"});
  REQUIRE(Lexemes(int_declaration.declarators.at(1).initializer) ==
          std::vector<std::string_view>{"1"});
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(2)).type ==
          VariableType::kInteger);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(3)).type ==
          VariableType::kShortint);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(4)).type ==
          VariableType::kLongint);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(5)).type ==
          VariableType::kByte);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(6)).type ==
          VariableType::kBit);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(7)).type ==
          VariableType::kReal);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(8)).type ==
          VariableType::kTime);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(9)).type ==
          VariableType::kShortreal);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(10)).type ==
          VariableType::kChandle);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(11)).type ==
          VariableType::kRealtime);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(12)).type ==
          VariableType::kEvent);
  REQUIRE(std::get<VariableDeclaration>(module_declaration.items.at(13)).type ==
          VariableType::kString);
}

TEST_CASE("Parse type and nettype declarations", "[parser]") {
  std::string src = R"(
    module foo ();
      typedef enum { red, green } color_t;
      typedef struct packed { int a; logic b; } struct_t;
      typedef union tagged { int value; } union_t;
      typedef class Base;
      nettype struct_t net_t;
      nettype struct_t resolved_t with pkg::resolve;
      net_t n;
      struct packed { int x; } anonymous;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto const translation_unit{parser.Parse()};
  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 8);

  auto const& enum_declaration{
      std::get<TypeDeclaration>(module_declaration.items.at(0))};
  REQUIRE(enum_declaration.kind == TypeDeclarationKind::kEnum);
  REQUIRE(enum_declaration.name == "color_t");
  REQUIRE(Lexemes(enum_declaration.body) ==
          std::vector<std::string_view>{"red", ",", "green"});

  auto const& struct_declaration{
      std::get<TypeDeclaration>(module_declaration.items.at(1))};
  REQUIRE(struct_declaration.kind == TypeDeclarationKind::kStruct);
  REQUIRE(struct_declaration.packed);
  REQUIRE(struct_declaration.name == "struct_t");
  REQUIRE(Lexemes(struct_declaration.body) ==
          std::vector<std::string_view>{"int", "a", ";", "logic", "b", ";"});

  auto const& union_declaration{
      std::get<TypeDeclaration>(module_declaration.items.at(2))};
  REQUIRE(union_declaration.kind == TypeDeclarationKind::kUnion);
  REQUIRE(union_declaration.tagged);

  auto const& forward_class{
      std::get<TypeDeclaration>(module_declaration.items.at(3))};
  REQUIRE(forward_class.forward);
  REQUIRE(forward_class.name == "Base");

  auto const& nettype_declaration{
      std::get<TypeDeclaration>(module_declaration.items.at(4))};
  REQUIRE(nettype_declaration.kind == TypeDeclarationKind::kNettype);
  REQUIRE(nettype_declaration.name == "net_t");

  auto const& resolved_nettype{
      std::get<TypeDeclaration>(module_declaration.items.at(5))};
  REQUIRE(resolved_nettype.name == "resolved_t");
  REQUIRE(Lexemes(resolved_nettype.resolution_function) ==
          std::vector<std::string_view>{"pkg", "::", "resolve"});

  auto const& net_declaration{
      std::get<UserDefinedNetDeclaration>(module_declaration.items.at(6))};
  REQUIRE(net_declaration.name == "n");
  REQUIRE(Lexemes(net_declaration.type_specifier) ==
          std::vector<std::string_view>{"net_t"});

  auto const& anonymous_declaration{
      std::get<StructuredVariableDeclaration>(module_declaration.items.at(7))};
  REQUIRE(anonymous_declaration.packed);
  REQUIRE(anonymous_declaration.declarators.front().name == "anonymous");
}

TEST_CASE("Parse interface declarations", "[parser]") {
  std::string src = R"(
    interface Iface(input wire clk);
      timeunit 1ns;
      int value;
      extern function void foo(int i);
      extern task bar;
      modport master(input clk, output value);
      default clocking cb;
    endinterface : Iface
  )";
  Parser parser{std::move(src)};

  auto const translation_unit{parser.Parse()};
  REQUIRE(translation_unit.size() == 1);
  auto const& interface_declaration{
      std::get<InterfaceDeclaration>(translation_unit.front())};
  REQUIRE(interface_declaration.name == "Iface");
  REQUIRE(interface_declaration.ports.size() == 1);
  REQUIRE(interface_declaration.ports.front().name == "clk");
  REQUIRE(interface_declaration.items.size() == 6);
  REQUIRE(std::holds_alternative<TimeDeclaration>(
      interface_declaration.items.at(0)));
  REQUIRE(std::holds_alternative<VariableDeclaration>(
      interface_declaration.items.at(1)));
  auto const& function_declaration{std::get<InterfaceSubroutineDeclaration>(
      interface_declaration.items.at(2))};
  REQUIRE(not function_declaration.task);
  REQUIRE(function_declaration.extern_declaration);
  auto const& task_declaration{std::get<InterfaceSubroutineDeclaration>(
      interface_declaration.items.at(3))};
  REQUIRE(task_declaration.task);
  auto const& modport_declaration{
      std::get<ModportDeclaration>(interface_declaration.items.at(4))};
  REQUIRE(modport_declaration.name == "master");
  REQUIRE(Lexemes(modport_declaration.ports) ==
          std::vector<std::string_view>{"(", "input", "clk", ",", "output",
                                        "value", ")"});
  REQUIRE(std::holds_alternative<DefaultClockingDeclaration>(
      interface_declaration.items.at(5)));
}

TEST_CASE("Parse unsupported module item declarations without losing sync",
          "[parser]") {
  std::string src = R"(
    module foo ();
      genvar g;
      wor [1:0] w;
      let inc(x) = x + 1;
      defparam u.WIDTH = 4;
      assign y = w;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 5);

  auto const& genvar_declaration{GenItemAs<GenvarDeclaration>(
      std::get<GenerateItem>(module_declaration.items.at(0)))};
  REQUIRE(genvar_declaration.identifiers.size() == 1);
  REQUIRE(genvar_declaration.identifiers.front().name == "g");
  auto const& wor_item{std::get<NetDeclaration>(
      module_declaration.items.at(1))};
  REQUIRE(wor_item.type == NetType::kWor);
  REQUIRE(std::get<TokenPreservingDeclaration>(module_declaration.items.at(2)).kind == "let");
  REQUIRE(std::get<TokenPreservingDeclaration>(module_declaration.items.at(3)).kind == "defparam");

  auto const& continuous_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(4))};
  REQUIRE(Lexemes(continuous_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"y"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"w"});
}

TEST_CASE("Parse unsupported module item blocks without losing sync",
          "[parser]") {
  std::string src = R"(
    module foo ();
      function int f(input int x);
        f = x;
      endfunction : f
      task t;
      endtask
      class C;
      endclass : C
      specify
        specparam tpd = 1;
      endspecify
      default clocking cb @(posedge clk);
      endclocking
      property p;
        a |-> b;
      endproperty
      sequence s;
        a ##1 b;
      endsequence
      covergroup cg @(posedge clk);
      endgroup
      checker ch;
      endchecker : ch
      assert property (p) else $error("bad");
      bind target checker_inst ci();
      final begin
        done = 1;
      end
      logic done;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 14);

  REQUIRE(std::holds_alternative<SubroutineDeclaration>(
      module_declaration.items.at(0)));
  REQUIRE(std::holds_alternative<SubroutineDeclaration>(
      module_declaration.items.at(1)));
  auto const& nested_class =
      std::get<ClassDeclaration>(module_declaration.items.at(2));
  REQUIRE(nested_class.name == "C");
  auto const& specify =
      std::get<SpecifyBlock>(module_declaration.items.at(3));
  REQUIRE_FALSE(specify.items.empty());
  REQUIRE(std::ranges::find_if(module_declaration.items, [](auto const& item) {
            return std::holds_alternative<FinalBlock>(item);
          }) != module_declaration.items.end());
  auto const done_iterator{std::ranges::find_if(
      module_declaration.items, [](auto const& item) {
        return std::holds_alternative<NetDeclaration>(item) and
               std::get<NetDeclaration>(item).name == "done";
      })};
  REQUIRE(done_iterator != module_declaration.items.end());
}

TEST_CASE("Parse unsupported module instances without losing sync",
          "[parser]") {
  std::string src = R"(
    module foo ();
      m13 instArr[3:1][2:5]();
      pullup p1(a);
      logic done;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 3);

  auto const& array_instance{
      std::get<ModuleInstantiation>(module_declaration.items.at(0))};
  REQUIRE(array_instance.instance_name == "instArr");
  REQUIRE_FALSE(array_instance.instance_dimensions.empty());

  auto const& primitive_instance{
      std::get<ModuleInstantiation>(module_declaration.items.at(1))};
  REQUIRE(primitive_instance.module_name == "pullup");
  REQUIRE(primitive_instance.instance_name == "p1");

  auto const& done_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(2))};
  REQUIRE(done_declaration.name == "done");
}

TEST_CASE("Parse module continuous assignments", "[parser]") {
  std::string src = R"(
    module foo (
      input a,
      input b,
      output y,
      output z
    );
      assign y = a + b;
      initial ignored;
      assign z = y;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.items.size() == 3);

  auto const& y_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(0))};
  REQUIRE(Lexemes(y_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"y"});
  REQUIRE(Lexemes(y_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"a", "+", "b"});
  REQUIRE(ExprAs<IdentifierExpression>(*y_assign.left_hand_side).name == "y");
  auto const& y_rhs{ExprAs<BinaryExpression>(*y_assign.right_hand_side)};
  REQUIRE(y_rhs.operator_lexeme == "+");
  REQUIRE(ExprAs<IdentifierExpression>(*y_rhs.left).name == "a");
  REQUIRE(ExprAs<IdentifierExpression>(*y_rhs.right).name == "b");

  auto const& initial_block{
      std::get<InitialBlock>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(initial_block.statement->tokens) ==
          std::vector<std::string_view>{"ignored", ";"});

  auto const& z_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(2))};
  REQUIRE(Lexemes(z_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"z"});
  REQUIRE(Lexemes(z_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"y"});
}

TEST_CASE("Parse expression AST for selects and concatenations", "[parser]") {
  std::string src = R"(
    module foo ();
      assign x = data[i];
      assign y = {a, b[0]};
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 2);

  auto const& x_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(0))};
  auto const& data_index{ExprAs<IndexExpression>(*x_assign.right_hand_side)};
  REQUIRE(ExprAs<IdentifierExpression>(*data_index.base).name == "data");
  REQUIRE(ExprAs<IdentifierExpression>(*data_index.index).name == "i");

  auto const& y_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(1))};
  auto const& concat{
      ExprAs<ConcatenationExpression>(*y_assign.right_hand_side)};
  REQUIRE(concat.expressions.size() == 2);
  REQUIRE(ExprAs<IdentifierExpression>(*concat.expressions.at(0)).name == "a");
  auto const& b_index{ExprAs<IndexExpression>(*concat.expressions.at(1))};
  REQUIRE(ExprAs<IdentifierExpression>(*b_index.base).name == "b");
  REQUIRE(ExprAs<LiteralExpression>(*b_index.index).value == "0");
}

TEST_CASE("Reject incomplete continuous assignment expressions", "[parser]") {
  std::string src = R"(
    module foo ();
      assign y = a + ;
      logic should_not_parse;
    endmodule
  )";
  Parser parser{std::move(src)};

  REQUIRE_THROWS_WITH(parser.Parse(), Catch::Matchers::ContainsSubstring(
                                          "[Parser] expected expression"));
}

TEST_CASE("Parse module always blocks", "[parser]") {
  std::string src = R"(
    module foo (
      input clk,
      input rst_n,
      input d,
      output q
    );
      always @(posedge clk) begin
        if (!rst_n) begin
          q <= 0;
        end else begin
          q <= d;
        end
      end
      assign q_shadow = q;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.items.size() == 2);

  auto const& always_block{
      std::get<AlwaysBlock>(module_declaration.items.at(0))};
  REQUIRE(
      Lexemes(always_block.statement->tokens) ==
      std::vector<std::string_view>{
          "@",     "(", "posedge", "clk", ")",  "begin", "if", "(",   "!",
          "rst_n", ")", "begin",   "q",   "<=", "0",     ";",  "end", "else",
          "begin", "q", "<=",      "d",   ";",  "end",   "end"});

  auto const& continuous_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(continuous_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"q_shadow"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"q"});
}

TEST_CASE("Parse module initial blocks", "[parser]") {
  std::string src = R"(
    module foo ();
      initial begin
        a = 0;
        begin
          b = a;
        end
      end
      initial ready = 1;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.items.size() == 2);

  auto const& begin_end_initial{
      std::get<InitialBlock>(module_declaration.items.at(0))};
  REQUIRE(Lexemes(begin_end_initial.statement->tokens) ==
          std::vector<std::string_view>{"begin", "a", "=", "0", ";", "begin",
                                        "b", "=", "a", ";", "end", "end"});

  auto const& single_statement_initial{
      std::get<InitialBlock>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(single_statement_initial.statement->tokens) ==
          std::vector<std::string_view>{"ready", "=", "1", ";"});
}

TEST_CASE("Reject unterminated begin-end statement blocks", "[parser]") {
  std::string src = R"(
    module foo ();
      initial if (enable) begin
        ready = 1;
    endmodule
  )";
  Parser parser{std::move(src)};

  REQUIRE_THROWS_WITH(
      parser.Parse(),
      Catch::Matchers::ContainsSubstring(
          "[Parser] expected 'end' while parsing begin-end block"));
}

TEST_CASE("Reject malformed procedural case statements", "[parser]") {
  SECTION("missing case item colon") {
    std::string src = R"(
      module foo ();
        initial begin
          case (sel)
            0 y = 0;
          endcase
        end
      endmodule
    )";
    Parser parser{std::move(src)};

    REQUIRE_THROWS_WITH(parser.Parse(),
                        Catch::Matchers::ContainsSubstring(
                            "[Parser] expected ':' while parsing case item"));
  }

  SECTION("missing endcase") {
    std::string src = R"(
      module foo ();
        initial begin
          case (sel)
            0: y = 0;
        end
      endmodule
    )";
    Parser parser{std::move(src)};

    REQUIRE_THROWS_WITH(
        parser.Parse(),
        Catch::Matchers::ContainsSubstring(
            "[Parser] expected 'endcase' while parsing case statement"));
  }
}

TEST_CASE("Reject missing procedural timing controls", "[parser]") {
  SECTION("delay control") {
    std::string src = R"(
      module foo ();
        initial #;
      endmodule
    )";
    Parser parser{std::move(src)};

    REQUIRE_THROWS_WITH(
        parser.Parse(),
        Catch::Matchers::ContainsSubstring(
            "[Parser] expected timing control while parsing timing control "
            "statement"));
  }

  SECTION("event control") {
    std::string src = R"(
      module foo ();
        initial @;
      endmodule
    )";
    Parser parser{std::move(src)};

    REQUIRE_THROWS_WITH(
        parser.Parse(),
        Catch::Matchers::ContainsSubstring(
            "[Parser] expected timing control while parsing timing control "
            "statement"));
  }
}

TEST_CASE("Parse procedural statement ASTs", "[parser]") {
  std::string src = R"(
    module foo ();
      initial begin
        begin
          a = 0;
          b <= c;
        end
        if (a) b = 1; else b <= 2;
        case (sel)
          0, 1: y = 0;
          default: y = 1;
        endcase
        casex (sel) default: y = 2; endcase
        casez (sel) default: y = 3; endcase
        for (i = 0; i < 3; i = i + 1) y = i;
        while (ready) y = 4;
        repeat (2) y = 5;
        foreach (arr[i]) y = arr[i];
        forever y = 6;
        @(posedge clk) y = 7;
        #5 y = 8;
        wait (ready) y = 9;
        wait_order (a, b) y = 10;
        wait fork;
        fork
          y = 11;
        join
        fork
          y = 12;
        join_any
        fork
          y = 13;
        join_none
        assign y = a;
        deassign y;
        force y = b;
        release y;
        $display("value=%0d", y);
      end
      always_ff @(posedge clk) q <= d;
      always_comb y = a;
      always_latch if (en) q <= d;
      final $finish;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 5);

  auto const& initial_block{
      std::get<InitialBlock>(module_declaration.items.at(0))};
  auto const& root_block{
      StmtAs<BeginEndBlockStatement>(*initial_block.statement)};
  REQUIRE(root_block.statements.size() == 23);

  auto const& nested_block{
      StmtAs<BeginEndBlockStatement>(*root_block.statements.at(0))};
  REQUIRE(nested_block.statements.size() == 2);
  auto const& blocking_assignment{
      StmtAs<AssignmentStatement>(*nested_block.statements.at(0))};
  REQUIRE(blocking_assignment.kind == AssignmentKind::kBlocking);
  REQUIRE(
      ExprAs<IdentifierExpression>(*blocking_assignment.left_hand_side).name ==
      "a");
  REQUIRE(
      ExprAs<LiteralExpression>(*blocking_assignment.right_hand_side).value ==
      "0");

  auto const& nonblocking_assignment{
      StmtAs<AssignmentStatement>(*nested_block.statements.at(1))};
  REQUIRE(nonblocking_assignment.kind == AssignmentKind::kNonblocking);
  REQUIRE(ExprAs<IdentifierExpression>(*nonblocking_assignment.left_hand_side)
              .name == "b");
  REQUIRE(ExprAs<IdentifierExpression>(*nonblocking_assignment.right_hand_side)
              .name == "c");

  auto const& if_else{StmtAs<IfElseStatement>(*root_block.statements.at(1))};
  REQUIRE(if_else.condition != nullptr);
  REQUIRE(ExprAs<IdentifierExpression>(*if_else.condition).name == "a");
  REQUIRE(if_else.else_statement != nullptr);
  REQUIRE(StmtAs<AssignmentStatement>(*if_else.then_statement).kind ==
          AssignmentKind::kBlocking);
  REQUIRE(StmtAs<AssignmentStatement>(*if_else.else_statement).kind ==
          AssignmentKind::kNonblocking);

  auto const& case_statement{
      StmtAs<CaseStatement>(*root_block.statements.at(2))};
  REQUIRE(case_statement.kind == CaseKind::kCase);
  REQUIRE(case_statement.expression != nullptr);
  REQUIRE(ExprAs<IdentifierExpression>(*case_statement.expression).name ==
          "sel");
  REQUIRE(case_statement.items.size() == 2);
  REQUIRE(case_statement.items.at(0).labels.size() == 2);
  REQUIRE(ExprAs<LiteralExpression>(*case_statement.items.at(0).labels.at(0))
              .value == "0");
  REQUIRE(ExprAs<LiteralExpression>(*case_statement.items.at(0).labels.at(1))
              .value == "1");
  REQUIRE_FALSE(case_statement.items.at(0).is_default);
  REQUIRE(
      StmtAs<AssignmentStatement>(*case_statement.items.at(0).statement).kind ==
      AssignmentKind::kBlocking);
  REQUIRE(case_statement.items.at(1).labels.empty());
  REQUIRE(case_statement.items.at(1).is_default);
  REQUIRE(
      StmtAs<AssignmentStatement>(*case_statement.items.at(1).statement).kind ==
      AssignmentKind::kBlocking);
  REQUIRE(StmtAs<CaseStatement>(*root_block.statements.at(3)).kind ==
          CaseKind::kCaseX);
  REQUIRE(StmtAs<CaseStatement>(*root_block.statements.at(4)).kind ==
          CaseKind::kCaseZ);

  auto const& for_loop{StmtAs<LoopStatement>(*root_block.statements.at(5))};
  REQUIRE(for_loop.kind == LoopKind::kFor);
  auto const& for_control{std::get<ForLoopControl>(for_loop.control)};
  REQUIRE(ExprAs<BinaryExpression>(*for_control.initializer).operator_lexeme ==
          "=");
  REQUIRE(ExprAs<BinaryExpression>(*for_control.condition).operator_lexeme ==
          "<");
  REQUIRE(ExprAs<BinaryExpression>(*for_control.step).operator_lexeme == "=");

  auto const& while_loop{StmtAs<LoopStatement>(*root_block.statements.at(6))};
  REQUIRE(while_loop.kind == LoopKind::kWhile);
  REQUIRE(ExprAs<IdentifierExpression>(
              *std::get<WhileLoopControl>(while_loop.control).condition)
              .name == "ready");

  auto const& repeat_loop{StmtAs<LoopStatement>(*root_block.statements.at(7))};
  REQUIRE(repeat_loop.kind == LoopKind::kRepeat);
  REQUIRE(ExprAs<LiteralExpression>(
              *std::get<RepeatLoopControl>(repeat_loop.control).count)
              .value == "2");

  auto const& foreach_loop{StmtAs<LoopStatement>(*root_block.statements.at(8))};
  REQUIRE(foreach_loop.kind == LoopKind::kForeach);
  auto const& foreach_control{
      std::get<ForeachLoopControl>(foreach_loop.control)};
  REQUIRE(
      ExprAs<IdentifierExpression>(*foreach_control.array_expression).name ==
      "arr");
  REQUIRE(foreach_control.loop_variables == std::vector<std::string_view>{"i"});

  auto const& forever_loop{StmtAs<LoopStatement>(*root_block.statements.at(9))};
  REQUIRE(forever_loop.kind == LoopKind::kForever);
  REQUIRE(std::holds_alternative<std::monostate>(forever_loop.control));

  auto const& event_timing{
      StmtAs<TimingControlStatement>(*root_block.statements.at(10))};
  REQUIRE(event_timing.kind == TimingControlKind::kEvent);
  auto const& event_control{std::get<EventControl>(event_timing.control)};
  REQUIRE(event_control.events.size() == 1);
  REQUIRE(std::holds_alternative<UnsupportedExpression>(
      event_control.events.at(0)->node));

  auto const& delay_timing{
      StmtAs<TimingControlStatement>(*root_block.statements.at(11))};
  REQUIRE(delay_timing.kind == TimingControlKind::kDelay);
  REQUIRE(ExprAs<LiteralExpression>(
              *std::get<DelayControl>(delay_timing.control).expression)
              .value == "5");

  auto const& wait_statement{
      StmtAs<WaitStatement>(*root_block.statements.at(12))};
  REQUIRE(wait_statement.kind == WaitKind::kWait);
  REQUIRE(
      ExprAs<IdentifierExpression>(
          *std::get<WaitExpressionControl>(wait_statement.control).expression)
          .name == "ready");

  auto const& wait_order_statement{
      StmtAs<WaitStatement>(*root_block.statements.at(13))};
  REQUIRE(wait_order_statement.kind == WaitKind::kWaitOrder);
  auto const& wait_order_control{
      std::get<WaitOrderControl>(wait_order_statement.control)};
  REQUIRE(wait_order_control.events.size() == 2);
  REQUIRE(ExprAs<IdentifierExpression>(*wait_order_control.events.at(0)).name ==
          "a");
  REQUIRE(ExprAs<IdentifierExpression>(*wait_order_control.events.at(1)).name ==
          "b");

  auto const& wait_fork_statement{
      StmtAs<WaitStatement>(*root_block.statements.at(14))};
  REQUIRE(wait_fork_statement.kind == WaitKind::kWaitFork);
  REQUIRE(std::holds_alternative<std::monostate>(wait_fork_statement.control));

  REQUIRE(StmtAs<ForkJoinStatement>(*root_block.statements.at(15)).kind ==
          ForkJoinKind::kJoin);
  REQUIRE(StmtAs<ForkJoinStatement>(*root_block.statements.at(16)).kind ==
          ForkJoinKind::kJoinAny);
  REQUIRE(StmtAs<ForkJoinStatement>(*root_block.statements.at(17)).kind ==
          ForkJoinKind::kJoinNone);

  auto const& procedural_assign{StmtAs<ProceduralContinuousAssignStatement>(
      *root_block.statements.at(18))};
  REQUIRE(procedural_assign.kind == ProceduralContinuousAssignKind::kAssign);
  REQUIRE(
      ExprAs<IdentifierExpression>(*procedural_assign.left_hand_side).name ==
      "y");
  REQUIRE(
      ExprAs<IdentifierExpression>(*procedural_assign.right_hand_side).name ==
      "a");

  auto const& procedural_deassign{StmtAs<ProceduralContinuousAssignStatement>(
      *root_block.statements.at(19))};
  REQUIRE(procedural_deassign.kind ==
          ProceduralContinuousAssignKind::kDeassign);
  REQUIRE(
      ExprAs<IdentifierExpression>(*procedural_deassign.left_hand_side).name ==
      "y");
  REQUIRE(procedural_deassign.right_hand_side == nullptr);

  auto const& procedural_force{StmtAs<ProceduralContinuousAssignStatement>(
      *root_block.statements.at(20))};
  REQUIRE(procedural_force.kind == ProceduralContinuousAssignKind::kForce);
  REQUIRE(ExprAs<IdentifierExpression>(*procedural_force.left_hand_side).name ==
          "y");
  REQUIRE(
      ExprAs<IdentifierExpression>(*procedural_force.right_hand_side).name ==
      "b");

  auto const& procedural_release{StmtAs<ProceduralContinuousAssignStatement>(
      *root_block.statements.at(21))};
  REQUIRE(procedural_release.kind == ProceduralContinuousAssignKind::kRelease);
  REQUIRE(
      ExprAs<IdentifierExpression>(*procedural_release.left_hand_side).name ==
      "y");
  REQUIRE(procedural_release.right_hand_side == nullptr);

  auto const& display_call{
      StmtAs<SystemTaskCallStatement>(*root_block.statements.at(22))};
  REQUIRE(display_call.name == "$display");
  REQUIRE(display_call.arguments.size() == 2);
  REQUIRE(ExprAs<LiteralExpression>(*display_call.arguments.at(0)).value ==
          "value=%0d");
  REQUIRE(ExprAs<IdentifierExpression>(*display_call.arguments.at(1)).name ==
          "y");

  auto const& always_ff_block{
      std::get<AlwaysBlock>(module_declaration.items.at(1))};
  REQUIRE(always_ff_block.kind == ProceduralBlockKind::kAlwaysFf);
  REQUIRE(std::holds_alternative<TimingControlStatement>(
      always_ff_block.statement->node));

  auto const& always_comb_block{
      std::get<AlwaysBlock>(module_declaration.items.at(2))};
  REQUIRE(always_comb_block.kind == ProceduralBlockKind::kAlwaysComb);
  REQUIRE(std::holds_alternative<AssignmentStatement>(
      always_comb_block.statement->node));

  auto const& always_latch_block{
      std::get<AlwaysBlock>(module_declaration.items.at(3))};
  REQUIRE(always_latch_block.kind == ProceduralBlockKind::kAlwaysLatch);
  REQUIRE(std::holds_alternative<IfElseStatement>(
      always_latch_block.statement->node));

  auto const& final_block{std::get<FinalBlock>(module_declaration.items.at(4))};
  auto const& finish_call{
      StmtAs<SystemTaskCallStatement>(*final_block.statement)};
  REQUIRE(finish_call.name == "$finish");
  REQUIRE(finish_call.arguments.empty());
}

TEST_CASE("Parse module generate blocks", "[parser]") {
  std::string src = R"(
    module foo #(parameter WIDTH = 4) (
      input [WIDTH-1:0] data,
      output [WIDTH-1:0] q
    );
      generate
        genvar i;
        for (i = 0; i < WIDTH; i = i + 1) begin : gen_q
          assign q[i] = data[i];
        end
      endgenerate
      assign done = 1;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.items.size() == 2);

  auto const& generate_item{
      std::get<GenerateItem>(module_declaration.items.at(0))};
  auto const& generate_block{GenItemAs<GenerateRegion>(generate_item)};
  REQUIRE(Lexemes(GenerateRegionBodyTokens(generate_item)) ==
          std::vector<std::string_view>{
              "genvar", "i",      ";", "for", "(",     "i",     "=",
              "0",      ";",      "i", "<",   "WIDTH", ";",     "i",
              "=",      "i",      "+", "1",   ")",     "begin", ":",
              "gen_q",  "assign", "q", "[",   "i",     "]",     "=",
              "data",   "[",      "i", "]",   ";",     "end"});
  REQUIRE(generate_block.items.size() == 2);
  auto const& genvar_declaration{
      GenItemAs<GenvarDeclaration>(*generate_block.items.at(0))};
  REQUIRE(genvar_declaration.identifiers.size() == 1);
  REQUIRE(genvar_declaration.identifiers.front().name == "i");
  auto const& structured_for{
      GenItemAs<GenerateFor>(*generate_block.items.at(1))};
  auto const& initialization{
      ExprAs<BinaryExpression>(*structured_for.initialization)};
  REQUIRE(initialization.operator_lexeme == "=");
  auto const& condition{ExprAs<BinaryExpression>(*structured_for.condition)};
  REQUIRE(condition.operator_lexeme == "<");
  REQUIRE(ExprAs<IdentifierExpression>(*condition.right).name == "WIDTH");
  auto const& step{ExprAs<BinaryExpression>(*structured_for.step)};
  REQUIRE(step.operator_lexeme == "=");
  REQUIRE(structured_for.loop_variable == "i");
  REQUIRE(structured_for.block_name == "gen_q");
  REQUIRE(structured_for.body.size() == 1);
  REQUIRE(std::holds_alternative<ContinuousAssign>(
      structured_for.body.front()->node));

  auto const& continuous_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(continuous_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"done"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"1"});
}

TEST_CASE("Reject generate expressions with unmatched parentheses",
          "[parser]") {
  std::string src = R"(
    module foo #(parameter WIDTH = 4) (
      input [WIDTH-1:0] data,
      output [WIDTH-1:0] q
    );
      generate
        genvar i;
        for (i = 0; i < WIDTH; i = i + 1 begin
          assign y = a;
        end
      endgenerate
    endmodule
  )";
  Parser parser{std::move(src)};

  REQUIRE_THROWS_WITH(
      parser.Parse(),
      Catch::Matchers::ContainsSubstring(
          "[Parser] expected ')' while parsing generate for expression"));
}

TEST_CASE("Reject genvar declaration without an identifier", "[parser]") {
  std::string src = R"(
    module foo ();
      genvar = 0;
    endmodule
  )";
  Parser parser{std::move(src)};

  REQUIRE_THROWS_WITH(
      parser.Parse(),
      Catch::Matchers::ContainsSubstring(
          "[Parser] expected identifier while parsing genvar declaration"));
}

TEST_CASE("Reject genvar declaration without a semicolon", "[parser]") {
  std::string src = R"(
    module foo ();
      genvar i
    endmodule
  )";
  Parser parser{std::move(src)};

  REQUIRE_THROWS_WITH(parser.Parse(),
                      Catch::Matchers::ContainsSubstring(
                          "[Parser] unexpected end-of-file while parsing "
                          "genvar declaration"));
}

TEST_CASE("Parse module-scope generate constructs", "[parser]") {
  std::string src = R"(
    module foo #(parameter WIDTH = 4) ();
      genvar j;
      for (genvar i = 0; i < WIDTH; i += 1)
        if (i == 2) begin : gen_hit
          assign hit = j;
        end

      case (WIDTH)
        1, 2: begin : small
          assign small_width = 1;
        end
        default: begin : wide
          assign small_width = 0;
        end
      endcase
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 3);

  auto const& genvar_declaration{GenItemAs<GenvarDeclaration>(
      std::get<GenerateItem>(module_declaration.items.at(0)))};
  REQUIRE(genvar_declaration.identifiers.size() == 1);
  REQUIRE(genvar_declaration.identifiers.front().name == "j");

  auto const& generate_for{GenItemAs<GenerateFor>(
      std::get<GenerateItem>(module_declaration.items.at(1)))};
  REQUIRE(generate_for.loop_variable == "i");
  REQUIRE(Lexemes(generate_for.initialization->tokens) ==
          std::vector<std::string_view>{"i", "=", "0"});
  REQUIRE(Lexemes(generate_for.condition->tokens) ==
          std::vector<std::string_view>{"i", "<", "WIDTH"});
  REQUIRE(generate_for.body.size() == 1);
  auto const& generate_if{GenItemAs<GenerateIf>(*generate_for.body.front())};
  REQUIRE(Lexemes(generate_if.condition->tokens) ==
          std::vector<std::string_view>{"i", "==", "2"});
  REQUIRE(generate_if.then_block_name == "gen_hit");
  REQUIRE(generate_if.then_body.size() == 1);
  REQUIRE(std::holds_alternative<ContinuousAssign>(
      generate_if.then_body.front()->node));

  auto const& generate_case{GenItemAs<GenerateCase>(
      std::get<GenerateItem>(module_declaration.items.at(2)))};
  REQUIRE(Lexemes(generate_case.expression->tokens) ==
          std::vector<std::string_view>{"WIDTH"});
  REQUIRE(generate_case.items.size() == 2);
  REQUIRE(generate_case.items.at(0).expressions.size() == 2);
  REQUIRE_FALSE(generate_case.items.at(0).is_default);
  REQUIRE(generate_case.items.at(0).body.size() == 1);
  REQUIRE(generate_case.items.at(1).is_default);
  REQUIRE(generate_case.items.at(1).body.size() == 1);
}

TEST_CASE("Preserve bare begin-end blocks in module body",
          "[parser]") {
  std::string src = R"(
    module foo ();
      begin : gen_scope
        assign y = a;
      end
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 1);

  auto const& preserved_item{
      std::get<TokenPreservingDeclaration>(module_declaration.items.front())};
  REQUIRE(preserved_item.kind == "begin");
}

TEST_CASE("Parse module instantiations", "[parser]") {
  std::string src = R"(
    module child #(parameter WIDTH = 8) (
      input clk,
      input [WIDTH-1:0] data,
      output ready
    );
    endmodule

    module top (
      input clk,
      input [3:0] data,
      output ready
    );
      child #(.WIDTH(4)) u_child (
        .clk(clk),
        .data(data),
        .ready(ready)
      );
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 2);

  auto const& top_module{std::get<ModuleDeclaration>(translation_unit.at(1))};
  REQUIRE(top_module.name == "top");
  REQUIRE(top_module.items.size() == 1);

  auto const& instantiation{
      std::get<ModuleInstantiation>(top_module.items.at(0))};
  REQUIRE(instantiation.module_name == "child");
  REQUIRE(instantiation.instance_name == "u_child");
  REQUIRE(Lexemes(instantiation.parameter_overrides) ==
          std::vector<std::string_view>{".", "WIDTH", "(", "4", ")"});
  REQUIRE(Lexemes(instantiation.port_connections) ==
          std::vector<std::string_view>{".", "clk", "(", "clk", ")", ",", ".",
                                        "data", "(", "data", ")", ",", ".",
                                        "ready", "(", "ready", ")"});
}

TEST_CASE("Parse nested module generate blocks", "[parser]") {
  std::string src = R"(
    module foo ();
      generate
        generate
          assign a = b;
        endgenerate
      endgenerate
      logic done;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "foo");
  REQUIRE(module_declaration.items.size() == 2);

  auto const& generate_item{
      std::get<GenerateItem>(module_declaration.items.at(0))};
  auto const& generate_block{GenItemAs<GenerateRegion>(generate_item)};
  REQUIRE(Lexemes(GenerateRegionBodyTokens(generate_item)) ==
          std::vector<std::string_view>{"generate", "assign", "a", "=", "b",
                                        ";", "endgenerate"});
  REQUIRE(generate_block.items.size() == 1);
  auto const& nested_item{*generate_block.items.front()};
  auto const& nested_region{GenItemAs<GenerateRegion>(nested_item)};
  REQUIRE(Lexemes(GenerateRegionBodyTokens(nested_item)) ==
          std::vector<std::string_view>{"assign", "a", "=", "b", ";"});
  REQUIRE(nested_region.items.size() == 1);

  auto const& net_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(1))};
  REQUIRE(net_declaration.name == "done");
}

TEST_CASE("Parse always block example file", "[parser]") {
  auto src{ReadFixture("always.sv")};
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "always_foo");
  REQUIRE(module_declaration.items.size() == 4);
  REQUIRE(
      std::holds_alternative<NetDeclaration>(module_declaration.items.at(0)));
  REQUIRE(
      std::holds_alternative<ContinuousAssign>(module_declaration.items.at(1)));
  REQUIRE(std::holds_alternative<InitialBlock>(module_declaration.items.at(2)));
  REQUIRE(std::holds_alternative<AlwaysBlock>(module_declaration.items.at(3)));
}

TEST_CASE("Parse generate block example file", "[parser]") {
  auto src{ReadFixture("generate.sv")};
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 1);

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.name == "generate_foo");
  REQUIRE(module_declaration.parameters.size() == 1);
  REQUIRE(module_declaration.ports.size() == 3);
  REQUIRE(module_declaration.items.size() == 3);
  REQUIRE(
      std::holds_alternative<NetDeclaration>(module_declaration.items.at(0)));
  REQUIRE(std::holds_alternative<GenerateItem>(module_declaration.items.at(1)));
  REQUIRE(
      std::holds_alternative<ContinuousAssign>(module_declaration.items.at(2)));
}

TEST_CASE("Parse module instantiation example file", "[parser]") {
  auto src{ReadFixture("module.sv")};
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(translation_unit.size() == 2);

  auto const& leaf_module{std::get<ModuleDeclaration>(translation_unit.at(0))};
  REQUIRE(leaf_module.name == "module_leaf");
  REQUIRE(leaf_module.parameters.size() == 1);
  REQUIRE(leaf_module.ports.size() == 3);
  REQUIRE(leaf_module.items.size() == 1);
  REQUIRE(std::holds_alternative<ContinuousAssign>(leaf_module.items.at(0)));

  auto const& top_module{std::get<ModuleDeclaration>(translation_unit.at(1))};
  REQUIRE(top_module.name == "module_foo");
  REQUIRE(top_module.ports.size() == 3);
  REQUIRE(top_module.items.size() == 1);

  auto const& instantiation{
      std::get<ModuleInstantiation>(top_module.items.at(0))};
  REQUIRE(instantiation.module_name == "module_leaf");
  REQUIRE(instantiation.instance_name == "u_leaf");
  REQUIRE(Lexemes(instantiation.parameter_overrides) ==
          std::vector<std::string_view>{".", "WIDTH", "(", "4", ")"});
  REQUIRE(Lexemes(instantiation.port_connections) ==
          std::vector<std::string_view>{".", "clk", "(", "clk", ")", ",", ".",
                                        "data", "(", "data", ")", ",", ".", "q",
                                        "(", "q", ")"});
}

TEST_CASE("Parse all SystemVerilog fixture as compilation unit", "[parser]") {
  auto src{ReadFixture("all.sv")};
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  REQUIRE(not translation_unit.empty());
  REQUIRE(translation_unit.size() > 10);
  auto const has_module_named{[&translation_unit](std::string_view const name) {
    return std::ranges::any_of(
        translation_unit, [name](auto const& design_element) {
          auto const* module_declaration{
              std::get_if<ModuleDeclaration>(&design_element)};
          return module_declaration != nullptr and
                 module_declaration->name == name;
        });
  }};

  REQUIRE(has_module_named("m1"));
  REQUIRE(has_module_named("m2"));
  REQUIRE(std::ranges::any_of(translation_unit, [](auto const& design_element) {
    auto const* module_declaration{
        std::get_if<ModuleDeclaration>(&design_element)};
    return module_declaration != nullptr and module_declaration->name == "m9";
  }));
}
