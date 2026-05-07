// SPDX-License-Identifier: MIT

#include <catch2/catch_all.hpp>

import std;
import svt.model.ast;
import svt.core.parser;

namespace {
using Parser = svt::core::Parser;
using ModuleDeclaration = svt::model::ModuleDeclaration;
using PortDirection = svt::model::PortDirection;
using ParameterTypeDeclaration = svt::model::ParameterTypeDeclaration;
using ParameterValueDeclaration = svt::model::ParameterValueDeclaration;
using ContinuousAssign = svt::model::ContinuousAssign;
using AlwaysBlock = svt::model::AlwaysBlock;
using InitialBlock = svt::model::InitialBlock;
using GenerateBlock = svt::model::GenerateBlock;
using ModuleInstantiation = svt::model::ModuleInstantiation;
using NetDeclaration = svt::model::NetDeclaration;
using NetType = svt::model::NetType;
using UnsupportedModuleItem = svt::model::UnsupportedModuleItem;
using UnsupportedDesignElement = svt::model::UnsupportedDesignElement;
using Expression = svt::model::Expression;
using IdentifierExpression = svt::model::IdentifierExpression;
using LiteralExpression = svt::model::LiteralExpression;
using BinaryExpression = svt::model::BinaryExpression;
using IndexExpression = svt::model::IndexExpression;
using RangeSelectExpression = svt::model::RangeSelectExpression;
using ConcatenationExpression = svt::model::ConcatenationExpression;
using PackedRangeDimension = svt::model::PackedRangeDimension;
using PackedSizeDimension = svt::model::PackedSizeDimension;
using GenerateForExpression = svt::model::GenerateForExpression;

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

TEST_CASE("Parse unsupported compilation-unit design elements", "[parser]") {
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
  REQUIRE(std::get<UnsupportedDesignElement>(translation_unit.at(0)).kind ==
          "timeunit");
  REQUIRE(std::get<UnsupportedDesignElement>(translation_unit.at(1)).kind ==
          "package");
  REQUIRE(std::get<UnsupportedDesignElement>(translation_unit.at(2)).kind ==
          "class");
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

  auto const& ready_port{module_declaration.ports.at(2)};
  REQUIRE(ready_port.name == "ready");
  REQUIRE(ready_port.direction == PortDirection::kOutput);
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
  REQUIRE(m1.parameters.size() == 1);
  REQUIRE(m1.ports.empty());

  auto const& m2{std::get<ModuleDeclaration>(translation_unit.at(1))};
  REQUIRE(m2.name == "m2");
  REQUIRE(m2.parameters.size() == 3);
  REQUIRE(m2.ports.size() == 2);
  REQUIRE(m2.ports.at(0).name == "a");
  REQUIRE(m2.ports.at(0).direction == PortDirection::kInput);
  REQUIRE(m2.ports.at(1).name == "b");
  REQUIRE(m2.ports.at(1).direction == PortDirection::kOutput);
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

TEST_CASE("Parse unsupported module item declarations without losing sync",
          "[parser]") {
  std::string src = R"(
    module foo ();
      reg [3:0] r;
      int i[2];
      event ev;
      genvar g;
      time t;
      shortreal sr;
      chandle c;
      realtime rt;
      wor [1:0] w;
      typedef logic [3:0] nibble_t;
      let inc(x) = x + 1;
      defparam u.WIDTH = 4;
      assign y = r;
    endmodule
  )";
  Parser parser{std::move(src)};

  auto translation_unit = parser.Parse();

  auto const& module_declaration{
      std::get<ModuleDeclaration>(translation_unit.front())};
  REQUIRE(module_declaration.items.size() == 13);

  auto const expected_kinds{std::vector<std::string_view>{
      "reg", "int", "event", "genvar", "time", "shortreal", "chandle",
      "realtime", "wor", "typedef", "let", "defparam"}};
  for (auto const item_index : std::views::iota(0UZ, expected_kinds.size())) {
    auto const& unsupported_item{std::get<UnsupportedModuleItem>(
        module_declaration.items.at(item_index))};
    REQUIRE(unsupported_item.kind == expected_kinds.at(item_index));
    REQUIRE(not unsupported_item.tokens.empty());
  }

  auto const& continuous_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(12))};
  REQUIRE(Lexemes(continuous_assign.left_hand_side->tokens) ==
          std::vector<std::string_view>{"y"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side->tokens) ==
          std::vector<std::string_view>{"r"});
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
  REQUIRE(module_declaration.items.size() == 13);

  auto const expected_kinds{std::vector<std::string_view>{
      "function", "task", "class", "specify", "default", "property", "sequence",
      "covergroup", "checker", "assert", "bind", "final"}};
  for (auto const item_index : std::views::iota(0UZ, expected_kinds.size())) {
    auto const& unsupported_item{std::get<UnsupportedModuleItem>(
        module_declaration.items.at(item_index))};
    REQUIRE(unsupported_item.kind == expected_kinds.at(item_index));
    REQUIRE(not unsupported_item.tokens.empty());
  }

  auto const& done_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(12))};
  REQUIRE(done_declaration.name == "done");
  REQUIRE(done_declaration.type == NetType::kLogic);
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
      std::get<UnsupportedModuleItem>(module_declaration.items.at(0))};
  REQUIRE(array_instance.kind == "m13");
  REQUIRE(Lexemes(array_instance.tokens) ==
          std::vector<std::string_view>{"m13", "instArr", "[", "3", ":", "1",
                                        "]", "[", "2", ":", "5", "]", "(", ")",
                                        ";"});

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
  REQUIRE(Lexemes(initial_block.body) ==
          std::vector<std::string_view>{"ignored"});

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
  REQUIRE(Lexemes(always_block.event_control) ==
          std::vector<std::string_view>{"@", "(", "posedge", "clk", ")"});
  REQUIRE(Lexemes(always_block.body) ==
          std::vector<std::string_view>{"if", "(", "!", "rst_n", ")", "begin",
                                        "q", "<=", "0", ";", "end", "else",
                                        "begin", "q", "<=", "d", ";", "end"});

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
  REQUIRE(Lexemes(begin_end_initial.body) ==
          std::vector<std::string_view>{"a", "=", "0", ";", "begin", "b", "=",
                                        "a", ";", "end"});

  auto const& single_statement_initial{
      std::get<InitialBlock>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(single_statement_initial.body) ==
          std::vector<std::string_view>{"ready", "=", "1"});
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

  auto const& generate_block{
      std::get<GenerateBlock>(module_declaration.items.at(0))};
  REQUIRE(Lexemes(generate_block.body) ==
          std::vector<std::string_view>{
              "genvar", "i",      ";", "for", "(",     "i",     "=",
              "0",      ";",      "i", "<",   "WIDTH", ";",     "i",
              "=",      "i",      "+", "1",   ")",     "begin", ":",
              "gen_q",  "assign", "q", "[",   "i",     "]",     "=",
              "data",   "[",      "i", "]",   ";",     "end"});
  REQUIRE(generate_block.expressions.size() == 1);
  auto const& generate_for{
      std::get<GenerateForExpression>(generate_block.expressions.front())};
  auto const& initialization{
      ExprAs<BinaryExpression>(*generate_for.initialization)};
  REQUIRE(initialization.operator_lexeme == "=");
  auto const& condition{ExprAs<BinaryExpression>(*generate_for.condition)};
  REQUIRE(condition.operator_lexeme == "<");
  REQUIRE(ExprAs<IdentifierExpression>(*condition.right).name == "WIDTH");
  auto const& step{ExprAs<BinaryExpression>(*generate_for.step)};
  REQUIRE(step.operator_lexeme == "=");

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

  auto const& generate_block{
      std::get<GenerateBlock>(module_declaration.items.at(0))};
  REQUIRE(Lexemes(generate_block.body) ==
          std::vector<std::string_view>{"generate", "assign", "a", "=", "b",
                                        ";", "endgenerate"});

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
  REQUIRE(
      std::holds_alternative<GenerateBlock>(module_declaration.items.at(1)));
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
