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
using NetDeclaration = svt::model::NetDeclaration;
using NetType = svt::model::NetType;

auto Lexemes(auto const& tokens) -> std::vector<std::string_view> {
  std::vector<std::string_view> result{};
  for (auto const& token : tokens) {
    result.push_back(token.lexeme);
  }
  return result;
}

auto ReadExample(std::filesystem::path const& example_path) -> std::string {
  auto const test_path{std::filesystem::path{__FILE__}.parent_path()};
  std::ifstream file_stream{test_path / ".." / example_path,
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
  REQUIRE(Lexemes(mask_parameter.default_value) ==
          std::vector<std::string_view>{"WIDTH", "-", "1"});

  auto const& continued_mask_parameter{
      std::get<ParameterValueDeclaration>(module_declaration.parameters.at(1))};
  REQUIRE(continued_mask_parameter.name == "ALT_MASK");
  REQUIRE(Lexemes(continued_mask_parameter.type_specifier) ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(Lexemes(continued_mask_parameter.default_value) ==
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
  REQUIRE(Lexemes(implicit_value_parameter.default_value) ==
          std::vector<std::string_view>{"8"});
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
  REQUIRE(module_declaration.items.size() == 2);

  auto const& bus_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(0))};
  REQUIRE(bus_declaration.name == "bus");
  REQUIRE(bus_declaration.type == NetType::kWire);
  REQUIRE(Lexemes(bus_declaration.type_specifier) ==
          std::vector<std::string_view>{"[", "N", "-", "1", ":", "0", "]"});

  auto const& ready_declaration{
      std::get<NetDeclaration>(module_declaration.items.at(1))};
  REQUIRE(ready_declaration.name == "ready");
  REQUIRE(ready_declaration.type == NetType::kLogic);
  REQUIRE(ready_declaration.type_specifier.empty());
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
  REQUIRE(Lexemes(y_assign.left_hand_side) ==
          std::vector<std::string_view>{"y"});
  REQUIRE(Lexemes(y_assign.right_hand_side) ==
          std::vector<std::string_view>{"a", "+", "b"});

  auto const& initial_block{
      std::get<InitialBlock>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(initial_block.body) ==
          std::vector<std::string_view>{"ignored"});

  auto const& z_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(2))};
  REQUIRE(Lexemes(z_assign.left_hand_side) ==
          std::vector<std::string_view>{"z"});
  REQUIRE(Lexemes(z_assign.right_hand_side) ==
          std::vector<std::string_view>{"y"});
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
  REQUIRE(Lexemes(continuous_assign.left_hand_side) ==
          std::vector<std::string_view>{"q_shadow"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side) ==
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
          std::vector<std::string_view>{"genvar", "i", ";", "for", "(",
                                        "i", "=", "0", ";", "i", "<",
                                        "WIDTH", ";", "i", "=", "i", "+",
                                        "1", ")", "begin", ":", "gen_q",
                                        "assign", "q", "[", "i", "]", "=",
                                        "data", "[", "i", "]", ";", "end"});

  auto const& continuous_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(continuous_assign.left_hand_side) ==
          std::vector<std::string_view>{"done"});
  REQUIRE(Lexemes(continuous_assign.right_hand_side) ==
          std::vector<std::string_view>{"1"});
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
  auto src{ReadExample("example/lexer/always_foo.sv")};
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
  auto src{ReadExample("example/lexer/generate_foo.sv")};
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
  REQUIRE(std::holds_alternative<GenerateBlock>(module_declaration.items.at(1)));
  REQUIRE(
      std::holds_alternative<ContinuousAssign>(module_declaration.items.at(2)));
}
