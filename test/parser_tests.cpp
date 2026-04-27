// SPDX-License-Identifier: MIT

#include <catch2/catch_all.hpp>

import svt.model.ast;
import svt.core.parser;

namespace {
using Parser = svt::core::Parser;
using ModuleDeclaration = svt::model::ModuleDeclaration;
using PortDirection = svt::model::PortDirection;
using ParameterTypeDeclaration = svt::model::ParameterTypeDeclaration;
using ParameterValueDeclaration = svt::model::ParameterValueDeclaration;
using ContinuousAssign = svt::model::ContinuousAssign;
using NetDeclaration = svt::model::NetDeclaration;
using NetType = svt::model::NetType;

auto Lexemes(auto const& tokens) -> std::vector<std::string_view> {
  std::vector<std::string_view> result{};
  for (auto const& token : tokens) {
    result.push_back(token.lexeme);
  }
  return result;
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
  REQUIRE(module_declaration.items.size() == 2);

  auto const& y_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(0))};
  REQUIRE(Lexemes(y_assign.left_hand_side) ==
          std::vector<std::string_view>{"y"});
  REQUIRE(Lexemes(y_assign.right_hand_side) ==
          std::vector<std::string_view>{"a", "+", "b"});

  auto const& z_assign{
      std::get<ContinuousAssign>(module_declaration.items.at(1))};
  REQUIRE(Lexemes(z_assign.left_hand_side) ==
          std::vector<std::string_view>{"z"});
  REQUIRE(Lexemes(z_assign.right_hand_side) ==
          std::vector<std::string_view>{"y"});
}
