

module svt.core.parser;

import std;
import fmt;

namespace rng = std::ranges;

namespace svt::core {

using TokenType = ::svt::model::TokenType;
using Token = ::svt::model::Token;
using token_stream_t = ::svt::model::token_stream_t;
using SourceLocation = ::svt::model::SourceLocation;
using AstNode = ::svt::model::AstNode;
using AstNodePointer = ::svt::model::AstNodePointer;
using ModuleDeclaration = ::svt::model::ModuleDeclaration;
using ParameterDeclaration = ::svt::model::ParameterDeclaration;
using PortDeclaration = ::svt::model::PortDeclaration;

Lexer::Lexer(std::string&& sv_source_code)
    : m_sv_source_code{std::move(sv_source_code)},
      m_sv_source_code_view{m_sv_source_code} {
}

auto Lexer::Next() -> Token {
  SkipWhiteSpaceAndComments();
  if (m_position >= rng::size(m_sv_source_code_view)) {
    return Token{.type = TokenType::kEndOfFile,
                 .lexeme = "",
                 .location = m_source_location};
  }

  // consume current character
  auto const token_source_location{m_source_location};
  auto const character{Peek()};
  m_position += 1;
  m_source_location.column += 1;

  // single-character punctuation
  static std::array<char, 11> constexpr kPunctuationCharacters{
      '(', ')', '[', ']', '{', '}', ';', ',', '#', ':', '?'};
  if (rng::contains(kPunctuationCharacters, character)) {
    return Token{.type = TokenType::kPunctuation,
                 .lexeme = m_sv_source_code_view.substr(m_position - 1, 1),
                 .location = token_source_location};
  }

  // if start of string
  if (character == '"') {
    return ScanString(token_source_location);
  }

  // two-character operators
  static std::array<std::pair<char, char>, 6> constexpr kTwoCharOperators{
      {{'=', '='}, {'!', '='}, {'<', '='}, {'>', '='}, {'&', '&'}, {'|', '|'}}};
  if (rng::contains(kTwoCharOperators,
                    std::pair<char, char>(character, Peek()))) {
    m_position += 1;
    m_source_location.column += 1;
    return Token{.type = TokenType::kOperator,
                 .lexeme = m_sv_source_code_view.substr(m_position - 2, 2),
                 .location = token_source_location};
  }

  // single-char operators
  static std::array<char, 8> constexpr kSingleCharOperators{'+', '-', '*', '/',
                                                            '<', '>', '=', '!'};
  if (rng::contains(kSingleCharOperators, character)) {
    return Token{.type = TokenType::kOperator,
                 .lexeme = m_sv_source_code_view.substr(m_position - 1, 1),
                 .location = token_source_location};
  }

  // identifier or keyword
  if (std::isalpha(character) != 0 or character == '_') {
    return ScanIdentifierOrKeyword(token_source_location);
  }

  // number literal
  if (std::isdigit(character) != 0) {
    return ScanNumber(token_source_location);
  }

  // NOTE: for now, we abort early on seeing an unexpected character. Later for
  // rich tooling (e.g. capture all syntax errors in sv source-code), this can
  // be relaxed to just remember this occurrence and continue further lexing
  throw std::runtime_error{
      fmt::format("Unexpected character '{}' at row: {}, column: {}", character,
                  token_source_location.row, token_source_location.column)};
}

auto Lexer::ScanNumber(SourceLocation const& token_source_location) -> Token {
  auto const start_position{m_position - 1};

  while (std::isdigit(Peek()) != 0) {
    m_position += 1;
    m_source_location.column += 1;
  }

  if (Peek() == '.') {
    m_position += 1;
    m_source_location.column += 1;

    while (std::isdigit(Peek()) != 0) {
      m_position += 1;
      m_source_location.column += 1;
    }

    return Token{.type = TokenType::kRealLiteral,
                 .lexeme = m_sv_source_code_view.substr(
                     start_position, m_position - start_position),
                 .location = token_source_location};
  }

  return Token{.type = TokenType::kIntegerLiteral,
               .lexeme = m_sv_source_code_view.substr(
                   start_position, m_position - start_position),
               .location = token_source_location};
}

auto Lexer::ScanIdentifierOrKeyword(SourceLocation const& token_source_location)
    -> Token {
  static std::array<std::string_view, 11> constexpr kKeywords{
      "module", "endmodule", "parameter", "wire",   "assign", "begin",
      "end",    "if",        "else",      "always", "initial"};

  auto const start_position{m_position - 1};

  while (std::isalnum(Peek()) != 0 or Peek() == '_') {
    m_position += 1;
    m_source_location.column += 1;
  }

  Token result{};
  result.lexeme =
      m_sv_source_code_view.substr(start_position, m_position - start_position);
  result.location = token_source_location;
  result.type = rng::contains(kKeywords, result.lexeme)
                    ? TokenType::kKeyword
                    : TokenType::kIdentifier;
  return result;
}

auto Lexer::ScanString(SourceLocation const& token_source_location) -> Token {
  // NOTE: we ignore first '"' for start position as this will be excluded in
  // result token's lexeme
  auto const start_position{m_position};

  while (true) {
    auto const character{Peek()};

    if (character == '\0') {
      throw std::runtime_error{
          fmt::format("Unterminated string literal at row: {}, column: {}",
                      token_source_location.row, token_source_location.column)};
    }

    if (character == '\\') {
      // consume backslash
      m_position += 1;
      m_source_location.column += 1;

      auto const next_character{Peek()};
      if (next_character == '\0') {
        throw std::runtime_error{fmt::format(
            "Unterminated string literal at row: {}, column: {}",
            token_source_location.row, token_source_location.column)};
      }

      m_position += 1;
      if (next_character == '\n') {
        m_source_location.row += 1;
        m_source_location.column = 1;
      } else {
        m_source_location.column += 1;
      }

      continue;
    }

    if (character == '"') {
      // found closing quote

      // consuming it
      m_position += 1;
      m_source_location.column += 1;

      break;
    }

    // regular char or newline
    m_position += 1;
    if (character == '\n') {
      m_source_location.row += 1;
      m_source_location.column = 1;
    } else {
      m_source_location.column += 1;
    }
  }

  // NOTE: we ignore last '"' for lexeme end as this is meant to be excluded in
  // resulting token lexeme
  return Token{.type = TokenType::kStringLiteral,
               .lexeme = m_sv_source_code_view.substr(
                   start_position, m_position - start_position - 1),
               .location = token_source_location};
}

template <std::size_t kOffset>
auto Lexer::Peek() const -> char {
  try {
    return m_sv_source_code_view.at(m_position + kOffset);
  } catch (std::out_of_range const& exception) {
    return '\0';
  }
}

auto Lexer::SkipWhiteSpaceAndComments() -> void {
  while (true) {
    auto const next_character{Peek()};

    // whitespace
    if (next_character == ' ' or next_character == '\t' or
        next_character == '\r') {
      m_position += 1;
      m_source_location.column += 1;
    }
    // line break
    else if (next_character == '\n') {
      m_position += 1;
      m_source_location.row += 1;
      m_source_location.column = 1;
    }
    // line comment `// ---`
    else if (next_character == '/' and Peek<1>() == '/') {
      m_position += 2;
      m_source_location.column += 2;
      // keep advancing till end of line or EOF
      while (Peek() != '\n' and Peek() != '\0') {
        m_position += 1;
        m_source_location.column += 1;
      }
    }
    // block comment `/* --- */`
    else if (next_character == '/' and Peek<1>() == '*') {
      m_position += 2;
      m_source_location.column += 2;
      // keep advancing till closing `*/` or EOF
      while (Peek() != '*' or Peek<1>() != '/') {
        // EOF reached
        if (Peek() == '\0') {
          break;
        }

        m_position += 1;
        if (Peek() != '\n') {
          m_source_location.column += 1;
        } else {
          m_source_location.row += 1;
          m_source_location.column = 1;
        }
      }
      // finally consume `*/`
      m_position += 2;
      m_source_location.column += 2;
    }
    // real content
    else {
      break;
    }
  }
}

Parser::Parser(std::string&& sv_source_code)
    : m_lexer{std::move(sv_source_code)} {
}

auto Parser::Peek(std::size_t offset) -> Token {
  while (rng::size(m_lookahead_buffer) <= offset) {
    m_lookahead_buffer.push(m_lexer.Next());
  }

  return m_lookahead_buffer.front();
}

auto Parser::ConsumeToken() -> void {
  m_lookahead_buffer.pop();
}

auto Parser::Parse() -> TranslationUnit {
  TranslationUnit translation_unit{};

  while (true) {
    if (Peek().type == TokenType::kEndOfFile) {
      break;
    }

    translation_unit.push_back(ParseDeclaration());
  }

  for (auto const& declaration : translation_unit) {
    fmt::print("name: {}", std::get<ModuleDeclaration>(declaration).name);
  }

  return translation_unit;
}

auto Parser::ParseDeclaration() -> AstNode {
  auto const token{Peek()};

  switch (token.type) {
    case TokenType::kKeyword: {
      if (token.lexeme == "module") {
        ConsumeToken();
        return ParseModuleDeclaration();
      }

      throw std::runtime_error{
          fmt::format("[Parser/Debug] unexpected top-level token at ({}, {})",
                      token.location.row, token.location.column)};
    }

    default: {
      throw std::runtime_error{
          fmt::format("[Parser] unexpected top-level token at ({}, {})",
                      token.location.row, token.location.column)};
    }
  }
}

auto Parser::ParseModuleDeclaration() -> AstNode {
  AstNode result{std::in_place_type<ModuleDeclaration>};

  auto& module_declaration = std::get<ModuleDeclaration>(result);
  module_declaration.name = Peek().lexeme;
  ConsumeToken();

  if (auto const& token{Peek()}; token.type == TokenType::kPunctuation) {
    if (token.lexeme == "#") {
      ConsumeToken();
      module_declaration.parameters.push_back(ParseParameters());
    } else if (token.lexeme == "(") {
      ConsumeToken();
      module_declaration.ports.push_back(ParsePorts());
    } else {
      throw std::runtime_error{
          fmt::format("[Parser] unexpected punctuation token at ({}, {})",
                      token.location.row, token.location.column)};
    }
  } else {
    throw std::runtime_error{fmt::format(
        "[Parser] expected punctuation token (param/port) at ({}, {})",
        token.location.row, token.location.column)};
  }

  return result;
}

auto Parser::ParseParameters() -> ParameterDeclaration {
  ParameterDeclaration result{};
  result.name = Peek().lexeme;
  ConsumeToken();

  if (auto const& token{Peek()}; token.type == TokenType::kPunctuation) {
  }

  return result;
}

auto Parser::ParsePorts() -> PortDeclaration {
  PortDeclaration result{};
  return result;
}

}  // namespace svt::core

// TODO(): do we really need m_lookeahead_buffer in Parser? can't we just use
// Lexer's token-stream directly?
