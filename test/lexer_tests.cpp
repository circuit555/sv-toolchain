

#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

import svt.model.token;
import svt.core.parser;

namespace {
using Token = svt::model::Token;
using TokenType = svt::model::TokenType;
using Lexer = svt::core::Lexer;
}  // namespace

TEST_CASE("Identifier and keyword tokens", "[lexer]") {
  std::string src = "module foo endmodule";
  Lexer lexer{src};

  Token t1 = lexer.Next();
  REQUIRE(t1.type == TokenType::kKeyword);
  REQUIRE(t1.lexeme == "module");

  Token t2 = lexer.Next();
  REQUIRE(t2.type == TokenType::kIdentifier);
  REQUIRE(t2.lexeme == "foo");

  Token t3 = lexer.Next();
  REQUIRE(t3.type == TokenType::kKeyword);
  REQUIRE(t3.lexeme == "endmodule");

  Token t4 = lexer.Next();
  REQUIRE(t4.type == TokenType::kEndOfFile);
}

TEST_CASE("Integer and real literals", "[lexer]") {
  std::string src = "123 45.67";
  Lexer lexer{src};

  Token t1 = lexer.Next();
  REQUIRE(t1.type == TokenType::kIntegerLiteral);
  REQUIRE(t1.lexeme == "123");

  Token t2 = lexer.Next();
  REQUIRE(t2.type == TokenType::kRealLiteral);
  REQUIRE(t2.lexeme == "45.67");

  Token t3 = lexer.Next();
  REQUIRE(t3.type == TokenType::kEndOfFile);
}

TEST_CASE("String literal", "[lexer]") {
  // The src string literally contains: "hello \"world\""
  std::string src = R"("hello \"world\"")";
  Lexer lexer{src};

  // First token should be our string literal (without the quotes)
  Token t = lexer.Next();
  REQUIRE(t.type == TokenType::kStringLiteral);
  // The lexeme is the interior: hello \"world\"
  REQUIRE(t.lexeme == R"(hello \"world\")");

  // Then we hit EOF
  Token t2 = lexer.Next();
  REQUIRE(t2.type == TokenType::kEndOfFile);
}

TEST_CASE("Operators and punctuation", "[lexer]") {
  std::string src = "+ - * / == != && || < <= > >= ( ) ; , [ ] { } # : ?";
  Lexer lexer{src};

  struct Expect {
    TokenType type;
    std::string_view lex;
  };
  std::vector<Expect> expected = {
      {.type = TokenType::kOperator, .lex = "+"},
      {.type = TokenType::kOperator, .lex = "-"},
      {.type = TokenType::kOperator, .lex = "*"},
      {.type = TokenType::kOperator, .lex = "/"},
      {.type = TokenType::kOperator, .lex = "=="},
      {.type = TokenType::kOperator, .lex = "!="},
      {.type = TokenType::kOperator, .lex = "&&"},
      {.type = TokenType::kOperator, .lex = "||"},
      {.type = TokenType::kOperator, .lex = "<"},
      {.type = TokenType::kOperator, .lex = "<="},
      {.type = TokenType::kOperator, .lex = ">"},
      {.type = TokenType::kOperator, .lex = ">="},
      {.type = TokenType::kPunctuation, .lex = "("},
      {.type = TokenType::kPunctuation, .lex = ")"},
      {.type = TokenType::kPunctuation, .lex = ";"},
      {.type = TokenType::kPunctuation, .lex = ","},
      {.type = TokenType::kPunctuation, .lex = "["},
      {.type = TokenType::kPunctuation, .lex = "]"},
      {.type = TokenType::kPunctuation, .lex = "{"},
      {.type = TokenType::kPunctuation, .lex = "}"},
      {.type = TokenType::kPunctuation, .lex = "#"},
      {.type = TokenType::kPunctuation, .lex = ":"},
      {.type = TokenType::kPunctuation, .lex = "?"}};

  for (auto const& ex : expected) {
    Token t = lexer.Next();
    REQUIRE(t.type == ex.type);
    REQUIRE(t.lexeme == ex.lex);
  }

  Token eof = lexer.Next();
  REQUIRE(eof.type == TokenType::kEndOfFile);
}

TEST_CASE("Skip whitespace and comments", "[lexer]") {
  // Leading spaces, a line comment, a block comment spanning lines, then "foo"
  std::string src = R"(  // comment line
/* block
comment */ foo)";
  Lexer lexer{src};

  Token t = lexer.Next();
  REQUIRE(t.type == TokenType::kIdentifier);
  REQUIRE(t.lexeme == "foo");

  Token eof = lexer.Next();
  REQUIRE(eof.type == TokenType::kEndOfFile);
}

TEST_CASE("Unknown character throws", "[lexer]") {
  std::string src = "@";
  Lexer lexer{src};

  REQUIRE_THROWS_AS(lexer.Next(), std::runtime_error);
}

TEST_CASE("Lex module declaration with parameter and vector", "[lexer]") {
  // Multi-line SV code: parameterized module, empty port list, bus declaration,
  // endmodule
  std::string src = R"(
    module foo #(parameter int N = 8) ();
    wire [N-1:0] bus;
    endmodule
  )";
  Lexer lexer{src};

  struct Expect {
    TokenType type;
    std::string_view lex;
  };
  std::vector<Expect> expected = {
      {.type = TokenType::kKeyword, .lex = "module"},
      {.type = TokenType::kIdentifier, .lex = "foo"},
      {.type = TokenType::kPunctuation, .lex = "#"},
      {.type = TokenType::kPunctuation, .lex = "("},
      {.type = TokenType::kKeyword, .lex = "parameter"},
      {.type = TokenType::kIdentifier, .lex = "int"},
      {.type = TokenType::kIdentifier, .lex = "N"},
      {.type = TokenType::kOperator, .lex = "="},
      {.type = TokenType::kIntegerLiteral, .lex = "8"},
      {.type = TokenType::kPunctuation, .lex = ")"},
      {.type = TokenType::kPunctuation, .lex = "("},
      {.type = TokenType::kPunctuation, .lex = ")"},
      {.type = TokenType::kPunctuation, .lex = ";"},
      {.type = TokenType::kKeyword, .lex = "wire"},
      {.type = TokenType::kPunctuation, .lex = "["},
      {.type = TokenType::kIdentifier, .lex = "N"},
      {.type = TokenType::kOperator, .lex = "-"},
      {.type = TokenType::kIntegerLiteral, .lex = "1"},
      {.type = TokenType::kPunctuation, .lex = ":"},
      {.type = TokenType::kIntegerLiteral, .lex = "0"},
      {.type = TokenType::kPunctuation, .lex = "]"},
      {.type = TokenType::kIdentifier, .lex = "bus"},
      {.type = TokenType::kPunctuation, .lex = ";"},
      {.type = TokenType::kKeyword, .lex = "endmodule"},
  };

  for (auto const& ex : expected) {
    Token t = lexer.Next();
    REQUIRE(t.type == ex.type);
    REQUIRE(t.lexeme == ex.lex);
  }

  Token eof = lexer.Next();
  REQUIRE(eof.type == TokenType::kEndOfFile);
}

TEST_CASE("Token source locations are correct", "[lexer]") {
  // 'a' at (1,1), 'b' at (1,3), then 'c' at (2,1)
  std::string src = R"(a b
c)";
  Lexer lexer{src};

  Token t1 = lexer.Next();
  REQUIRE(t1.type == TokenType::kIdentifier);
  REQUIRE(t1.lexeme == "a");
  REQUIRE(t1.location.row == 1);
  REQUIRE(t1.location.column == 1);

  Token t2 = lexer.Next();
  REQUIRE(t2.type == TokenType::kIdentifier);
  REQUIRE(t2.lexeme == "b");
  REQUIRE(t2.location.row == 1);
  REQUIRE(t2.location.column == 3);

  Token t3 = lexer.Next();
  REQUIRE(t3.type == TokenType::kIdentifier);
  REQUIRE(t3.lexeme == "c");
  REQUIRE(t3.location.row == 2);
  REQUIRE(t3.location.column == 1);

  Token eof = lexer.Next();
  REQUIRE(eof.type == TokenType::kEndOfFile);
}
