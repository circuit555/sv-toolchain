// SPDX-License-Identifier: MIT

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
using ParameterTypeDeclaration = ::svt::model::ParameterTypeDeclaration;
using ParameterValueDeclaration = ::svt::model::ParameterValueDeclaration;
using PortDeclaration = ::svt::model::PortDeclaration;
using PortDirection = ::svt::model::PortDirection;
using ContinuousAssign = ::svt::model::ContinuousAssign;
using ModuleItem = ::svt::model::ModuleItem;

namespace {

inline auto IsParameterDeclarationPrefix(std::span<Token const> tokens)
    -> bool {
  return tokens.front().lexeme == "parameter" or
         tokens.front().lexeme == "localparam";
}

inline auto IsOpeningDelimiter(Token const& token) -> bool {
  return token.type == TokenType::kLParen or
         token.type == TokenType::kLBracket or token.type == TokenType::kLBrace;
}

inline auto IsClosingDelimiter(Token const& token) -> bool {
  return token.type == TokenType::kRParen or
         token.type == TokenType::kRBracket or token.type == TokenType::kRBrace;
}

inline auto IsListSeparator(Token const& token) -> bool {
  return token.type == TokenType::kComma;
}

inline auto IsHorizontalWhiteSpace(char const character) -> bool {
  return character == ' ' or character == '\t' or character == '\r';
}

inline auto IsLineBreak(char const character) -> bool {
  return character == '\n';
}

inline auto StartsLineComment(char const current_character,
                              char const next_character) -> bool {
  return current_character == '/' and next_character == '/';
}

inline auto StartsBlockComment(char const current_character,
                               char const next_character) -> bool {
  return current_character == '/' and next_character == '*';
}

auto FindValueParameterNameIndex(std::vector<Token> const& tokens,
                                 std::size_t const head_begin,
                                 std::size_t const head_end) -> std::size_t {
  auto name_index{head_end};
  while (name_index > head_begin) {
    name_index -= 1;
    if (tokens.at(name_index).type == TokenType::kIdentifier) {
      break;
    }
  }

  if (tokens.at(name_index).type != TokenType::kIdentifier) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected parameter name at ({}, {})",
                    tokens.at(head_begin).location.row,
                    tokens.at(head_begin).location.column)};
  }

  return name_index;
}

auto ParseParameterDeclaration(std::span<Token const> const tokens)
    -> ParameterDeclaration {
  if (rng::empty(tokens)) [[unlikely]] {
    throw std::runtime_error{"[Parser] expected parameter declarations"};
  }

  auto const parameter_begin_iterator{rng::next(
      rng::cbegin(tokens), IsParameterDeclarationPrefix(tokens) ? 1 : 0,
      rng::cend(tokens))};
  auto const equals_operator_iterator{rng::find_if(
      std::span{rng::next(parameter_begin_iterator, 1, rng::cend(tokens)),
                rng::cend(tokens)},
      [](Token const& token) -> bool {
        return token.type == TokenType::kEquals;
      })};

  auto const tokens_slice{
      std::span{parameter_begin_iterator,
                rng::next(equals_operator_iterator, 1, rng::cend(tokens))}};
  if (rng::empty(tokens_slice)) [[unlikely]] {
    throw std::runtime_error{"[Parser] expected parameter tokens"};
  }

  auto parameter{[tokens_slice]() -> ParameterDeclaration {
    if (tokens_slice.front().lexeme == "type") {
      return [tokens_slice]() -> ParameterTypeDeclaration {
        ParameterTypeDeclaration type_parameter{};

        auto const parameter_name_iterator{
            rng::next(rng::cbegin(tokens_slice), 1, rng::cend(tokens_slice))};

        // NOTE(): we do not use bounded check for rng::prev() in following as
        // we are assured that `tokens_slice` is not empty
        if (parameter_name_iterator == rng::prev(rng::cend(tokens_slice)))
            [[unlikely]] {
          throw std::runtime_error{
              fmt::format("[Parser] expected type parameter name at ({}, {})",
                          tokens_slice.front().location.row,
                          tokens_slice.front().location.column)};
        }

        if (parameter_name_iterator->type != TokenType::kIdentifier)
            [[unlikely]] {
          throw std::runtime_error{
              fmt::format("[Parser] unexpected token '{}' while parsing type "
                          "parameter name at ({}, {})",
                          parameter_name_iterator->lexeme,
                          parameter_name_iterator->location.row,
                          parameter_name_iterator->location.column)};
        }

        type_parameter.name = parameter_name_iterator->lexeme;

        return type_parameter;
      }();
    }

    return [tokens_slice]() -> ParameterValueDeclaration {
      ParameterValueDeclaration value_parameter{};

      auto const parameter_name_riterator{rng::find_if(
          tokens_slice | rng::views::reverse, [](Token const& token) -> bool {
            return token.type == TokenType::kIdentifier;
          })};
      if (parameter_name_riterator == rng::crend(tokens_slice)) {
        throw std::runtime_error{
            fmt::format("[Parser] expected parameter name at ({}, {})",
                        tokens_slice.front().location.row,
                        tokens_slice.front().location.column)};
      }

      value_parameter.name = parameter_name_riterator->lexeme;

      value_parameter.type_specifier =
          std::span{rng::cbegin(tokens_slice),
                    rng::prev(parameter_name_riterator.base(), 1,
                              rng::cbegin(tokens_slice))} |
          rng::views::transform([](auto const& token) -> std::string_view {
            return token.lexeme;
          }) |
          rng::to<std::vector>();

      return value_parameter;
    }();
  }()};

  if (equals_operator_iterator != rng::cend(tokens)) {
    std::visit(
        [equals_operator_iterator, tokens](auto& resolved_parameter) -> void {
          auto& default_value = [&resolved_parameter]() -> auto& {
            if constexpr (std::same_as<
                              std::remove_cvref_t<decltype(resolved_parameter)>,
                              ParameterTypeDeclaration>) {
              return resolved_parameter.default_type;
            } else {
              return resolved_parameter.default_value;
            }
          }();

          default_value =
              std::span{
                  rng::next(equals_operator_iterator, 1, rng::cend(tokens)),
                  rng::cend(tokens)} |
              rng::views::transform([](auto const& token) -> std::string_view {
                return token.lexeme;
              }) |
              rng::to<std::vector>();
        },
        parameter);
  }

  return parameter;
}

auto ParsePortDeclaration(
    std::span<Token const> const port_tokens,
    std::optional<PortDirection> const& previous_direction) -> PortDeclaration {
  auto is_port_direction_token{[](Token const& token) -> bool {
    return token.lexeme == "input" or token.lexeme == "output";
  }};

  auto parse_port_direction{[port_tokens]() -> PortDirection {
    if (port_tokens.front().lexeme == "input") {
      return PortDirection::kInput;
    }

    if (port_tokens.front().lexeme == "output") {
      return PortDirection::kOutput;
    }

    throw std::runtime_error{fmt::format(
        "[Parser] expected port direction at ({}, {})",
        port_tokens.front().location.row, port_tokens.front().location.column)};
  }};

  if (rng::empty(port_tokens)) [[unlikely]] {
    throw std::runtime_error{"[Parser] expected port declaration"};
  }

  if (not is_port_direction_token(port_tokens.front()) and
      not previous_direction.has_value()) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected port direction at ({}, {})",
        port_tokens.front().location.row, port_tokens.front().location.column)};
  }

  auto const port_name_iterator{
      rng::find_if(port_tokens | rng::views::reverse,
                   [&is_port_direction_token](Token const& token) -> bool {
                     return token.type == TokenType::kIdentifier and
                            not is_port_direction_token(token);
                   })};
  if (port_name_iterator == rng::crend(port_tokens)) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected port name at ({}, {})",
        port_tokens.front().location.row, port_tokens.front().location.column)};
  }

  PortDeclaration port{};
  port.name = port_name_iterator->lexeme;
  port.direction = is_port_direction_token(port_tokens.front())
                       ? parse_port_direction()
                       : previous_direction.value();
  return port;
}

auto JoinLexemes(std::span<std::string_view const> const lexemes)
    -> std::string {
  auto result = std::string{};
  for (auto const lexeme : lexemes) {
    if (not rng::empty(result)) {
      result += " ";
    }
    result += lexeme;
  }
  return result;
}

auto TokenLexemes(std::span<Token const> const tokens)
    -> std::vector<std::string_view> {
  return tokens |
         rng::views::transform([](Token const& token) -> std::string_view {
           return token.lexeme;
         }) |
         rng::to<std::vector>();
}

auto ToString(PortDirection const direction) -> std::string_view {
  switch (direction) {
    case PortDirection::kInput:
      return "input";
    case PortDirection::kOutput:
      return "output";
  }

  std::unreachable();
}

auto PrintParameter(ParameterDeclaration const& parameter) -> void {
  std::visit(
      [](auto const& resolved_parameter) -> void {
        if constexpr (std::same_as<
                          std::remove_cvref_t<decltype(resolved_parameter)>,
                          ParameterTypeDeclaration>) {
          fmt::println("    parameter type {} = {}", resolved_parameter.name,
                       JoinLexemes(resolved_parameter.default_type));
        } else {
          fmt::println("    parameter {} {} = {}",
                       JoinLexemes(resolved_parameter.type_specifier),
                       resolved_parameter.name,
                       JoinLexemes(resolved_parameter.default_value));
        }
      },
      parameter);
}

auto PrintModule(ModuleDeclaration const& module_declaration) -> void {
  fmt::println("module {}", module_declaration.name);

  if (not rng::empty(module_declaration.parameters)) {
    fmt::println("  parameters:");
    for (auto const& parameter : module_declaration.parameters) {
      PrintParameter(parameter);
    }
  }

  if (not rng::empty(module_declaration.ports)) {
    fmt::println("  ports:");
    for (auto const& port : module_declaration.ports) {
      fmt::println("    {} {}", ToString(port.direction), port.name);
    }
  }

  if (not rng::empty(module_declaration.items)) {
    fmt::println("  items:");
    for (auto const& item : module_declaration.items) {
      std::visit(
          [](auto const& resolved_item) -> void {
            if constexpr (std::same_as<
                              std::remove_cvref_t<decltype(resolved_item)>,
                              ContinuousAssign>) {
              fmt::println("    assign {} = {}",
                           JoinLexemes(resolved_item.left_hand_side),
                           JoinLexemes(resolved_item.right_hand_side));
            }
          },
          item);
    }
  }
}

}  // namespace

Lexer::Lexer(std::string&& sv_source_code)
    : m_sv_source_code{std::move(sv_source_code)},
      m_sv_source_code_view{m_sv_source_code} {
  while (true) {
    m_tokens.push_back(ScanNext());
    if (m_tokens.back().type == TokenType::kEndOfFile) {
      break;
    }
  }
}

auto Lexer::Tokens() const -> std::span<Token const> {
  return m_tokens;
}

auto Lexer::ScanNext() -> Token {
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
  auto const punctuation_lexeme{
      m_sv_source_code_view.substr(m_position - 1, 1)};
  switch (character) {
    case '(':
      return Token{.type = TokenType::kLParen,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case ')':
      return Token{.type = TokenType::kRParen,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '[':
      return Token{.type = TokenType::kLBracket,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case ']':
      return Token{.type = TokenType::kRBracket,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '{':
      return Token{.type = TokenType::kLBrace,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '}':
      return Token{.type = TokenType::kRBrace,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case ';':
      return Token{.type = TokenType::kSemicolon,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case ',':
      return Token{.type = TokenType::kComma,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '#':
      return Token{.type = TokenType::kHash,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case ':':
      return Token{.type = TokenType::kColon,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '?':
      return Token{.type = TokenType::kQuestion,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    default:
      break;
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

  if (character == '=') {
    return Token{.type = TokenType::kEquals,
                 .lexeme = m_sv_source_code_view.substr(m_position - 1, 1),
                 .location = token_source_location};
  }

  // single-char operators
  static std::array<char, 7> constexpr kSingleCharOperators{'+', '-', '*', '/',
                                                            '<', '>', '!'};
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

    if (character == '\0') [[unlikely]] {
      throw std::runtime_error{
          fmt::format("Unterminated string literal at row: {}, column: {}",
                      token_source_location.row, token_source_location.column)};
    }

    if (character == '\\') {
      // consume backslash
      m_position += 1;
      m_source_location.column += 1;

      auto const next_character{Peek()};
      if (next_character == '\0') [[unlikely]] {
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

    if (IsHorizontalWhiteSpace(next_character)) {
      SkipHorizontalWhiteSpace();
      continue;
    }

    if (IsLineBreak(next_character)) {
      SkipLineBreak();
      continue;
    }

    if (StartsLineComment(next_character, Peek<1>())) {
      SkipLineComment();
      continue;
    }

    if (StartsBlockComment(next_character, Peek<1>())) {
      SkipBlockComment();
      continue;
    }

    break;
  }
}

auto Lexer::SkipHorizontalWhiteSpace() -> void {
  m_position += 1;
  m_source_location.column += 1;
}

auto Lexer::SkipLineBreak() -> void {
  m_position += 1;
  m_source_location.row += 1;
  m_source_location.column = 1;
}

auto Lexer::SkipLineComment() -> void {
  m_position += 2;
  m_source_location.column += 2;

  while (Peek() != '\n' and Peek() != '\0') {
    m_position += 1;
    m_source_location.column += 1;
  }
}

auto Lexer::SkipBlockComment() -> void {
  m_position += 2;
  m_source_location.column += 2;

  while (Peek() != '*' or Peek<1>() != '/') {
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

  m_position += 2;
  m_source_location.column += 2;
}

Parser::Parser(std::string&& sv_source_code)
    : m_lexer{std::move(sv_source_code)},
      m_tokens{m_lexer.Tokens()},
      m_token_iterator{rng::cbegin(m_tokens)} {
}

auto Parser::ExpectToken(TokenType const expected_type,
                         std::string_view const context) -> void {
  if (m_token_iterator->type != expected_type) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] unexpected token '{}' while parsing {} at ({}, {})",
        m_token_iterator->lexeme, context, m_token_iterator->location.row,
        m_token_iterator->location.column)};
  }

  m_token_iterator++;
}

auto Parser::Parse() -> TranslationUnit {
  TranslationUnit translation_unit{};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    translation_unit.push_back(ParseDeclaration());
  }
  return translation_unit;
}

auto Print(Parser::TranslationUnit const& translation_unit) -> void {
  for (auto const& node : translation_unit) {
    std::visit(
        [](auto const& resolved_node) -> void {
          if constexpr (std::same_as<
                            std::remove_cvref_t<decltype(resolved_node)>,
                            ModuleDeclaration>) {
            PrintModule(resolved_node);
          } else {
            fmt::println("<unsupported AST node>");
          }
        },
        node);
  }
}

auto Parser::ParseDeclaration() -> AstNode {
  switch (m_token_iterator->type) {
    case TokenType::kKeyword: {
      if (m_token_iterator->lexeme == "module") {
        m_token_iterator++;
        return ParseModuleDeclaration();
      }

      throw std::runtime_error{fmt::format(
          "[Parser/Debug] unexpected top-level token at ({}, {})",
          m_token_iterator->location.row, m_token_iterator->location.column)};
    }

    [[unlikely]] default: {
      throw std::runtime_error{fmt::format(
          "[Parser] unexpected top-level token at ({}, {})",
          m_token_iterator->location.row, m_token_iterator->location.column)};
    }
  }
}

auto Parser::ParseModuleDeclaration() -> ModuleDeclaration {
  if (m_token_iterator->type != TokenType::kIdentifier) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected module name at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  ModuleDeclaration module_declaration{};
  module_declaration.name = m_token_iterator->lexeme;
  m_token_iterator++;

  if (m_token_iterator->type != TokenType::kHash and
      m_token_iterator->type != TokenType::kLParen and
      m_token_iterator->type != TokenType::kSemicolon) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected module parameter list, port list, or ';' at ({}, "
        "{})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  if (m_token_iterator->type == TokenType::kHash) {
    m_token_iterator++;
    module_declaration.parameters = ParseParameters();
  }

  if (m_token_iterator->type == TokenType::kLParen) {
    m_token_iterator++;
    module_declaration.ports = ParsePorts();
  }

  if (m_token_iterator->type == TokenType::kSemicolon) {
    m_token_iterator++;
  }

  module_declaration.items = ParseModuleItems();

  return module_declaration;
}

auto Parser::ParseModuleItems() -> std::vector<ModuleItem> {
  std::vector<ModuleItem> items{};

  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->lexeme != "endmodule") {
    if (m_token_iterator->type == TokenType::kKeyword and
        m_token_iterator->lexeme == "assign") {
      items.emplace_back(std::in_place_type<ContinuousAssign>,
                         ParseContinuousAssign());
      continue;
    }

    while (m_token_iterator->type != TokenType::kEndOfFile and
           m_token_iterator->type != TokenType::kSemicolon and
           m_token_iterator->lexeme != "endmodule") {
      m_token_iterator++;
    }

    if (m_token_iterator->type == TokenType::kSemicolon) {
      m_token_iterator++;
    }
  }

  if (m_token_iterator->lexeme == "endmodule") {
    m_token_iterator++;
  }

  return items;
}

auto Parser::ParseContinuousAssign() -> ContinuousAssign {
  ExpectToken(TokenType::kKeyword, "continuous assignment");

  auto const assignment_begin_iterator{m_token_iterator};

  auto const equals_operator_iterator{rng::find_if(
      std::span{rng::next(m_token_iterator, 1, rng::cend(m_tokens)),
                rng::cend(m_tokens)},
      [](Token const& token) -> bool {
        return token.type == TokenType::kEndOfFile or
               token.type == TokenType::kSemicolon or
               token.type == TokenType::kEquals;
      })};
  if (equals_operator_iterator == rng::cend(m_tokens) or
      equals_operator_iterator->type != TokenType::kEquals) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected '=' while parsing continuous assignment at ({}, {})",
        equals_operator_iterator->location.row,
        equals_operator_iterator->location.column)};
  }
  if (assignment_begin_iterator == equals_operator_iterator) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected left hand side while parsing continuous assignment "
        "at ({}, {})",
        assignment_begin_iterator->location.row,
        assignment_begin_iterator->location.column)};
  }

  auto const assignment_end_iterator{rng::find_if(
      std::span{rng::next(equals_operator_iterator, 1, rng::cend(m_tokens)),
                rng::cend(m_tokens)},
      [](Token const& token) -> bool {
        return token.type == TokenType::kSemicolon or
               token.type == TokenType::kEndOfFile;
      })};
  if (rng::next(equals_operator_iterator, 1, rng::cend(m_tokens)) ==
      assignment_end_iterator) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected right hand side while parsing continuous assignment "
        "at ({}, {})",
        equals_operator_iterator->location.row,
        equals_operator_iterator->location.column)};
  }

  ContinuousAssign continuous_assign{};
  continuous_assign.left_hand_side = TokenLexemes(
      std::span{assignment_begin_iterator, equals_operator_iterator});
  continuous_assign.right_hand_side = TokenLexemes(
      std::span{rng::next(equals_operator_iterator, 1, rng::cend(m_tokens)),
                assignment_end_iterator});

  m_token_iterator = assignment_end_iterator;
  ExpectToken(TokenType::kSemicolon, "continuous assignment");

  return continuous_assign;
}

auto Parser::ParseParameters() -> std::vector<ParameterDeclaration> {
  auto is_parameter_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  std::vector<ParameterDeclaration> result{};

  ExpectToken(TokenType::kLParen, "parameter list");

  while (not is_parameter_list_end()) {
    auto const parameter_tokens{ParseParameterTokens()};
    auto parameter{ParseParameterDeclaration(parameter_tokens)};

    if (auto const has_declaration_prefix{
            not rng::empty(parameter_tokens) and
            IsParameterDeclarationPrefix(parameter_tokens)};
        not has_declaration_prefix and
        not std::holds_alternative<ParameterTypeDeclaration>(parameter) and
        not rng::empty(result)) {
      auto& value_parameter{std::get<ParameterValueDeclaration>(parameter)};
      auto const& previous_parameter{result.back()};

      if (rng::empty(value_parameter.type_specifier)) {
        if (std::holds_alternative<ParameterTypeDeclaration>(
                previous_parameter)) {
          ParameterTypeDeclaration type_parameter{};
          type_parameter.name = value_parameter.name;
          type_parameter.default_type =
              std::move(value_parameter.default_value);
          parameter = std::move(type_parameter);
        } else {
          value_parameter.type_specifier =
              std::get<ParameterValueDeclaration>(previous_parameter)
                  .type_specifier;
        }
      }
    }

    result.push_back(std::move(parameter));

    if (m_token_iterator->type == TokenType::kComma) {
      m_token_iterator++;
    } else if (not is_parameter_list_end()) [[unlikely]] {
      throw std::runtime_error{fmt::format(
          "[Parser] expected ',' or ')' while parsing parameter "
          "list at ({}, {})",
          m_token_iterator->location.row, m_token_iterator->location.column)};
    }
  }

  ExpectToken(TokenType::kRParen, "parameter list");

  return result;
}

auto Parser::ParseParameterTokens() -> std::span<Token const> {
  auto const parameter_begin{m_token_iterator};

  auto delimiter_depth{0UZ};
  while (true) {
    if (IsOpeningDelimiter(*m_token_iterator)) {
      delimiter_depth++;
    } else if (IsClosingDelimiter(*m_token_iterator)) {
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        break;
      }

      delimiter_depth--;
    } else if (IsListSeparator(*m_token_iterator) and
               std::cmp_equal(delimiter_depth, 0UZ)) {
      break;
    }

    m_token_iterator++;
  }

  return {parameter_begin, m_token_iterator};
}

auto Parser::ParsePorts() -> std::vector<PortDeclaration> {
  std::vector<PortDeclaration> result{};

  auto const is_port_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  while (not is_port_list_end()) {
    auto const port_begin{m_token_iterator};

    auto delimiter_depth{0UZ};
    while (true) {
      if (m_token_iterator->type == TokenType::kEndOfFile) [[unlikely]] {
        throw std::runtime_error{fmt::format(
            "[Parser] unexpected end-of-file while parsing port list at ({}, "
            "{})",
            m_token_iterator->location.row, m_token_iterator->location.column)};
      }

      if (IsOpeningDelimiter(*m_token_iterator)) {
        delimiter_depth++;
      } else if (IsClosingDelimiter(*m_token_iterator)) {
        if (std::cmp_equal(delimiter_depth, 0UZ)) {
          break;
        }

        delimiter_depth--;
      } else if (IsListSeparator(*m_token_iterator) and
                 std::cmp_equal(delimiter_depth, 0UZ)) {
        break;
      }

      m_token_iterator++;
    }

    auto const port_tokens{
        std::span<Token const>{port_begin, m_token_iterator}};
    auto const previous_direction{
        rng::empty(result)
            ? std::optional<PortDirection>{}
            : std::optional<PortDirection>{result.back().direction}};
    result.push_back(ParsePortDeclaration(port_tokens, previous_direction));

    if (IsListSeparator(*m_token_iterator)) {
      m_token_iterator++;
    }
  }

  ExpectToken(TokenType::kRParen, "port list");

  return result;
}

}  // namespace svt::core

// TODO(): do we really need std::vector<std::string_view> stuff for holding
// tokens if they are occurring closeby in token span, why not instead use a
// subspan for them?
