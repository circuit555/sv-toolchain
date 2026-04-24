#include <catch2/catch_all.hpp>

import svt.model.ast;
import svt.core.parser;

namespace {
using Parser = svt::core::Parser;
using ModuleDeclaration = svt::model::ModuleDeclaration;
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

  auto const& mask_parameter{module_declaration.parameters.at(0)};
  REQUIRE(mask_parameter.name == "MASK");
  REQUIRE(mask_parameter.type_specifier ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(mask_parameter.default_value ==
          std::vector<std::string_view>{"WIDTH", "-", "1"});
  REQUIRE(mask_parameter.is_type_parameter == false);

  auto const& continued_mask_parameter{module_declaration.parameters.at(1)};
  REQUIRE(continued_mask_parameter.name == "ALT_MASK");
  REQUIRE(continued_mask_parameter.type_specifier ==
          std::vector<std::string_view>{"logic", "[", "7", ":", "0", "]"});
  REQUIRE(continued_mask_parameter.default_value ==
          std::vector<std::string_view>{"2"});
  REQUIRE(continued_mask_parameter.is_type_parameter == false);

  auto const& type_parameter{module_declaration.parameters.at(2)};
  REQUIRE(type_parameter.name == "T");
  REQUIRE(type_parameter.type_specifier ==
          std::vector<std::string_view>{"type"});
  REQUIRE(type_parameter.default_value ==
          std::vector<std::string_view>{"logic", "[", "3", ":", "0", "]"});
  REQUIRE(type_parameter.is_type_parameter == true);

  auto const& continued_type_parameter{module_declaration.parameters.at(3)};
  REQUIRE(continued_type_parameter.name == "U");
  REQUIRE(continued_type_parameter.type_specifier ==
          std::vector<std::string_view>{"type"});
  REQUIRE(continued_type_parameter.default_value ==
          std::vector<std::string_view>{"bit"});
  REQUIRE(continued_type_parameter.is_type_parameter == true);

  auto const& implicit_type_parameter{module_declaration.parameters.at(4)};
  REQUIRE(implicit_type_parameter.name == "WIDTH");
  REQUIRE(implicit_type_parameter.type_specifier.empty());
  REQUIRE(implicit_type_parameter.default_value ==
          std::vector<std::string_view>{"8"});
  REQUIRE(implicit_type_parameter.is_type_parameter == false);
}
