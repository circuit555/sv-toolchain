// SPDX-License-Identifier: MIT

#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

import std;
import svt.model.token;
import svt.core.parser;

namespace {
using Token = svt::model::Token;
using TokenType = svt::model::TokenType;
using Lexer = svt::core::Lexer;

struct ExpectedToken {
  TokenType type;
  std::string_view lexeme;
};

auto RequireTokens(std::span<Token const> const tokens,
                   std::span<ExpectedToken const> const expected) -> void {
  REQUIRE(tokens.size() == expected.size());
  for (std::size_t i{0}; i < expected.size(); i += 1) {
    REQUIRE(tokens.at(i).type == expected.at(i).type);
    REQUIRE(tokens.at(i).lexeme == expected.at(i).lexeme);
  }
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

TEST_CASE("Identifier and keyword tokens", "[lexer]") {
  std::string src = "module foo logic timeunit timeprecision endmodule";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "module"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "foo"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "logic"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "timeunit"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "timeprecision"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "endmodule"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("Integer and real literals", "[lexer]") {
  std::string src = "123 45.67 12'h800 1'b0 2'd3 'x '0 5ns 1step";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "123"},
      ExpectedToken{.type = TokenType::kRealLiteral, .lexeme = "45.67"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "12'h800"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "1'b0"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "2'd3"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "'x"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "'0"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "5ns"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "1step"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("System and escaped identifiers", "[lexer]") {
  std::string src = R"($info $bits $ \begin )";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "$info"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "$bits"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "$"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = R"(\begin)"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("String literal", "[lexer]") {
  // The src string literally contains: "hello \"world\""
  std::string src = R"("hello \"world\"")";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kStringLiteral,
                    .lexeme = R"(hello \"world\")"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("Operators and punctuation", "[lexer]") {
  std::string src =
      "+ - * / = == != && || < <= > >= ( ) ; , [ ] { } # : ? @ .";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "+"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "-"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "*"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "/"},
      ExpectedToken{.type = TokenType::kEquals, .lexeme = "="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "=="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "!="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "&&"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "||"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "<"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "<="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = ">"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = ">="},
      ExpectedToken{.type = TokenType::kLParen, .lexeme = "("},
      ExpectedToken{.type = TokenType::kRParen, .lexeme = ")"},
      ExpectedToken{.type = TokenType::kSemicolon, .lexeme = ";"},
      ExpectedToken{.type = TokenType::kComma, .lexeme = ","},
      ExpectedToken{.type = TokenType::kLBracket, .lexeme = "["},
      ExpectedToken{.type = TokenType::kRBracket, .lexeme = "]"},
      ExpectedToken{.type = TokenType::kLBrace, .lexeme = "{"},
      ExpectedToken{.type = TokenType::kRBrace, .lexeme = "}"},
      ExpectedToken{.type = TokenType::kHash, .lexeme = "#"},
      ExpectedToken{.type = TokenType::kColon, .lexeme = ":"},
      ExpectedToken{.type = TokenType::kQuestion, .lexeme = "?"},
      ExpectedToken{.type = TokenType::kAt, .lexeme = "@"},
      ExpectedToken{.type = TokenType::kDot, .lexeme = "."},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("SystemVerilog operators", "[lexer]") {
  std::string src = R"(% ^ ~ & | ** -> <-> => *> ## #-# += <<= >>>= ==? !=? '{)";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "%"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "^"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "~"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "&"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "|"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "**"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "->"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "<->"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "=>"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "*>"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "##"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "#-#"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "+="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "<<="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = ">>>="},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "==?"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "!=?"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "'{"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("Skip whitespace and comments", "[lexer]") {
  // Leading spaces, a line comment, a block comment spanning lines, then "foo"
  std::string src = R"(  // comment line
/* block
comment */ foo)";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "foo"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("Unknown character throws", "[lexer]") {
  std::string src = "`";

  REQUIRE_THROWS_AS(Lexer{std::move(src)}, std::runtime_error);
}

TEST_CASE("Lex module declaration with parameter and vector", "[lexer]") {
  // Multi-line SV code: parameterized module, empty port list, bus declaration,
  // endmodule
  std::string src = R"(
    module foo #(parameter int N = 8) ();
    wire [N-1:0] bus;
    endmodule
  )";
  Lexer lexer{std::move(src)};

  std::array expected{
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "module"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "foo"},
      ExpectedToken{.type = TokenType::kHash, .lexeme = "#"},
      ExpectedToken{.type = TokenType::kLParen, .lexeme = "("},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "parameter"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "int"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "N"},
      ExpectedToken{.type = TokenType::kEquals, .lexeme = "="},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "8"},
      ExpectedToken{.type = TokenType::kRParen, .lexeme = ")"},
      ExpectedToken{.type = TokenType::kLParen, .lexeme = "("},
      ExpectedToken{.type = TokenType::kRParen, .lexeme = ")"},
      ExpectedToken{.type = TokenType::kSemicolon, .lexeme = ";"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "wire"},
      ExpectedToken{.type = TokenType::kLBracket, .lexeme = "["},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "N"},
      ExpectedToken{.type = TokenType::kOperator, .lexeme = "-"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "1"},
      ExpectedToken{.type = TokenType::kColon, .lexeme = ":"},
      ExpectedToken{.type = TokenType::kIntegerLiteral, .lexeme = "0"},
      ExpectedToken{.type = TokenType::kRBracket, .lexeme = "]"},
      ExpectedToken{.type = TokenType::kIdentifier, .lexeme = "bus"},
      ExpectedToken{.type = TokenType::kSemicolon, .lexeme = ";"},
      ExpectedToken{.type = TokenType::kKeyword, .lexeme = "endmodule"},
      ExpectedToken{.type = TokenType::kEndOfFile, .lexeme = ""}};
  RequireTokens(lexer.Tokens(), expected);
}

TEST_CASE("Token source locations are correct", "[lexer]") {
  // 'a' at (1,1), 'b' at (1,3), then 'c' at (2,1)
  std::string src = R"(a b
c)";
  Lexer lexer{std::move(src)};

  auto const tokens{lexer.Tokens()};
  REQUIRE(tokens.size() == 4);

  Token const& t1 = tokens.at(0);
  REQUIRE(t1.type == TokenType::kIdentifier);
  REQUIRE(t1.lexeme == "a");
  REQUIRE(t1.location.row == 1);
  REQUIRE(t1.location.column == 1);

  Token const& t2 = tokens.at(1);
  REQUIRE(t2.type == TokenType::kIdentifier);
  REQUIRE(t2.lexeme == "b");
  REQUIRE(t2.location.row == 1);
  REQUIRE(t2.location.column == 3);

  Token const& t3 = tokens.at(2);
  REQUIRE(t3.type == TokenType::kIdentifier);
  REQUIRE(t3.lexeme == "c");
  REQUIRE(t3.location.row == 2);
  REQUIRE(t3.location.column == 1);

  Token const& eof = tokens.at(3);
  REQUIRE(eof.type == TokenType::kEndOfFile);
}

TEST_CASE("Lex all SystemVerilog fixture", "[lexer]") {
  auto src{ReadFixture("all.sv")};
  Lexer lexer{std::move(src)};

  REQUIRE(not lexer.Tokens().empty());
  REQUIRE(lexer.Tokens().back().type == TokenType::kEndOfFile);
}
