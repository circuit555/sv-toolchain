// SPDX-License-Identifier: MIT

module svt.core.parser;

import std;
import fmt;

namespace rng = std::ranges;

namespace svt::core {

using tokens_t = Parser::tokens_t;
using TokenType = ::svt::model::TokenType;
using Token = ::svt::model::Token;
using token_stream_t = ::svt::model::token_stream_t;
using SourceLocation = ::svt::model::SourceLocation;
using DesignElement = ::svt::model::DesignElement;
using ModuleDeclaration = ::svt::model::ModuleDeclaration;
using ParameterDeclaration = ::svt::model::ParameterDeclaration;
using ParameterTypeDeclaration = ::svt::model::ParameterTypeDeclaration;
using ParameterValueDeclaration = ::svt::model::ParameterValueDeclaration;
using PortDeclaration = ::svt::model::PortDeclaration;
using PortDirection = ::svt::model::PortDirection;
using NetDeclaration = ::svt::model::NetDeclaration;
using NetType = ::svt::model::NetType;
using ContinuousAssign = ::svt::model::ContinuousAssign;
using AlwaysBlock = ::svt::model::AlwaysBlock;
using InitialBlock = ::svt::model::InitialBlock;
using GenerateBlock = ::svt::model::GenerateBlock;
using ModuleInstantiation = ::svt::model::ModuleInstantiation;
using ModuleItem = ::svt::model::ModuleItem;
using UnsupportedDesignElement = ::svt::model::UnsupportedDesignElement;

namespace {

std::array<std::string_view, 16> constexpr kTopLevelEndKeywords{
    "endmodule",    "endpackage",  "endinterface", "endprogram",
    "endprimitive", "endconfig",   "endchecker",   "endclass",
    "endfunction",  "endtask",     "endspecify",   "endclocking",
    "endproperty",  "endsequence", "endgroup",     "endgenerate"};

inline auto IsParameterDeclarationPrefix(tokens_t const tokens) -> bool {
  return tokens.front().lexeme == "parameter" or
         tokens.front().lexeme == "localparam";
}

inline auto IsOpeningDelimiter(tokens_t::iterator const token_iterator)
    -> bool {
  return token_iterator->type == TokenType::kLParen or
         token_iterator->type == TokenType::kLBracket or
         token_iterator->type == TokenType::kLBrace;
}

inline auto IsClosingDelimiter(tokens_t::iterator const token_iterator)
    -> bool {
  return token_iterator->type == TokenType::kRParen or
         token_iterator->type == TokenType::kRBracket or
         token_iterator->type == TokenType::kRBrace;
}

inline auto IsListSeparator(tokens_t::iterator const token_iterator) -> bool {
  return token_iterator->type == TokenType::kComma;
}

inline auto IsKeyword(tokens_t::iterator const token_iterator,
                      std::string_view const lexeme) -> bool {
  return token_iterator->type == TokenType::kKeyword and
         token_iterator->lexeme == lexeme;
}

inline auto IsNetType(tokens_t::iterator const token_iterator) -> bool {
  return token_iterator->type == TokenType::kKeyword and
         (token_iterator->lexeme == "wire" or
          token_iterator->lexeme == "logic");
}

inline auto IsTopLevelEndKeyword(tokens_t::iterator const token_iterator)
    -> bool {
  if (token_iterator->type != TokenType::kKeyword) {
    return false;
  }

  return rng::contains(kTopLevelEndKeywords, token_iterator->lexeme);
}

inline auto IsHorizontalWhiteSpace(unsigned char const character) -> bool {
  return character == ' ' or character == '\t' or character == '\r';
}

inline auto IsLineBreak(unsigned char const character) -> bool {
  return character == '\n';
}

inline auto StartsLineComment(unsigned char const current_character,
                              unsigned char const next_character) -> bool {
  return current_character == '/' and next_character == '/';
}

inline auto StartsBlockComment(unsigned char const current_character,
                               unsigned char const next_character) -> bool {
  return current_character == '/' and next_character == '*';
}

inline auto IsIdentifierBodyCharacter(unsigned char const character) -> bool {
  return std::cmp_not_equal(std::isalnum(static_cast<unsigned char>(character)),
                            0) or
         character == '_' or character == '$';
}

inline auto IsBasedLiteralDigit(unsigned char const character) -> bool {
  return std::cmp_not_equal(std::isalnum(static_cast<unsigned char>(character)),
                            0) or
         character == '_' or character == '?' or character == 'x' or
         character == 'X' or character == 'z' or character == 'Z';
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
auto MatchingTopLevelEndKeyword(std::string_view const keyword)
    -> std::optional<std::string_view> {
  if (keyword == "module" or keyword == "macromodule") {
    return kTopLevelEndKeywords[0];
  }
  if (keyword == "package") {
    return kTopLevelEndKeywords[1];
  }
  if (keyword == "interface") {
    return kTopLevelEndKeywords[2];
  }
  if (keyword == "program") {
    return kTopLevelEndKeywords[3];
  }
  if (keyword == "primitive") {
    return kTopLevelEndKeywords[4];
  }
  if (keyword == "config") {
    return kTopLevelEndKeywords[5];
  }
  if (keyword == "checker") {
    return kTopLevelEndKeywords[6];
  }
  if (keyword == "class") {
    return kTopLevelEndKeywords[7];
  }
  if (keyword == "function") {
    return kTopLevelEndKeywords[8];
  }
  if (keyword == "task") {
    return kTopLevelEndKeywords[9];
  }

  return std::nullopt;
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

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

auto ParseParameterDeclaration(tokens_t const tokens) -> ParameterDeclaration {
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

      value_parameter.type_specifier = std::span{
          rng::cbegin(tokens_slice), rng::prev(parameter_name_riterator.base(),
                                               1, rng::cbegin(tokens_slice))};

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

          default_value = std::span{
              rng::next(equals_operator_iterator, 1, rng::cend(tokens)),
              rng::cend(tokens)};
        },
        parameter);
  }

  return parameter;
}

auto ParsePortDeclaration(
    tokens_t const port_tokens,
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

auto StripLeadingAttributes(tokens_t port_tokens) -> tokens_t {
  while (not rng::empty(port_tokens) and
         port_tokens.front().type == TokenType::kLParen and
         rng::size(port_tokens) > 1 and
         rng::next(rng::cbegin(port_tokens))->lexeme == "*") {
    auto token_iterator{
        rng::next(rng::cbegin(port_tokens), 2, rng::cend(port_tokens))};
    while (token_iterator != rng::cend(port_tokens)) {
      auto const next_token_iterator{
          rng::next(token_iterator, 1, rng::cend(port_tokens))};
      if (token_iterator->lexeme == "*" and
          next_token_iterator != rng::cend(port_tokens) and
          next_token_iterator->type == TokenType::kRParen) {
        port_tokens =
            tokens_t{rng::next(next_token_iterator, 1, rng::cend(port_tokens)),
                     rng::cend(port_tokens)};
        break;
      }

      token_iterator++;
    }

    if (token_iterator == rng::cend(port_tokens)) {
      return {};
    }
  }

  return port_tokens;
}

auto TryParsePort(tokens_t const port_tokens,
                  std::optional<PortDirection> const previous_direction)
    -> std::optional<PortDeclaration> {
  auto const stripped_port_tokens{StripLeadingAttributes(port_tokens)};
  if (rng::empty(stripped_port_tokens)) {
    return std::nullopt;
  }

  if (stripped_port_tokens.front().type == TokenType::kDot or
      stripped_port_tokens.front().lexeme == "ref" or
      stripped_port_tokens.front().lexeme == "inout" or
      stripped_port_tokens.front().lexeme == "interface") {
    return std::nullopt;
  }

  if (auto const has_explicit_direction{
          stripped_port_tokens.front().lexeme == "input" or
          stripped_port_tokens.front().lexeme == "output"};
      not has_explicit_direction and not previous_direction.has_value()) {
    return std::nullopt;
  }

  return ParsePortDeclaration(stripped_port_tokens, previous_direction);
}

auto JoinLexemes(tokens_t const tokens) -> std::string {
  auto result = std::string{};
  for (auto const& token : tokens) {
    if (not rng::empty(result)) {
      result += " ";
    }
    result += token.lexeme;
  }
  return result;
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

auto ToString(NetType const type) -> std::string_view {
  switch (type) {
    case NetType::kWire:
      return "wire";
    case NetType::kLogic:
      return "logic";
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
                              NetDeclaration>) {
              auto const type_specifier{
                  JoinLexemes(resolved_item.type_specifier)};
              if (rng::empty(type_specifier)) {
                fmt::println("    {} {}", ToString(resolved_item.type),
                             resolved_item.name);
              } else {
                fmt::println("    {} {} {}", ToString(resolved_item.type),
                             type_specifier, resolved_item.name);
              }
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              ContinuousAssign>) {
              fmt::println("    assign {} = {}",
                           JoinLexemes(resolved_item.left_hand_side),
                           JoinLexemes(resolved_item.right_hand_side));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              AlwaysBlock>) {
              fmt::println("    always {} {}",
                           JoinLexemes(resolved_item.event_control),
                           JoinLexemes(resolved_item.body));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              InitialBlock>) {
              fmt::println("    initial {}", JoinLexemes(resolved_item.body));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              GenerateBlock>) {
              fmt::println("    generate {}", JoinLexemes(resolved_item.body));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              ModuleInstantiation>) {
              fmt::println("    {} {} #({}) ({})", resolved_item.module_name,
                           resolved_item.instance_name,
                           JoinLexemes(resolved_item.parameter_overrides),
                           JoinLexemes(resolved_item.port_connections));
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

auto Lexer::Tokens() const -> tokens_t {
  return m_tokens;
}

auto Lexer::ScanNext() -> Token {
  static std::array<char, 12> constexpr kSingleCharOperators{
      '+', '-', '*', '/', '<', '>', '!', '%', '^', '~', '&', '|'};
  static std::array<std::string_view, 45> constexpr kOperators{
      "<<<=", ">>>=", "==?", "!=?", "===", "!==", "<<<", ">>>", "<<=",
      ">>=",  "<->",  "#-#", "|->", "|=>", "##",  "++",  "--",  "+=",
      "-=",   "*=",   "/=",  "%=",  "&=",  "|=",  "^=",  "->",  "=>",
      "*>",   "<=",   ">=",  "==",  "!=",  "&&",  "||",  "**",  "<<",
      ">>",   "^~",   "~^",  "~&",  "~|",  "::",  ":=",  ":/",  "'{"};

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

  // multiple-character operators
  if (auto const* const operator_iterator{
          rng::find_if(kOperators,
                       [this](auto const operator_lexeme) -> bool {
                         return m_sv_source_code_view.substr(m_position - 1)
                             .starts_with(operator_lexeme);
                       })};
      operator_iterator != rng::cend(kOperators)) {
    auto remaining_source{m_sv_source_code_view.substr(m_position - 1)};
    auto const operator_size{rng::size(*operator_iterator)};
    m_position += operator_size - 1;
    m_source_location.column += operator_size - 1;

    return Token{.type = TokenType::kOperator,
                 .lexeme = remaining_source.substr(0, operator_size),
                 .location = token_source_location};
  }

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
    case '@':
      return Token{.type = TokenType::kAt,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    case '.':
      return Token{.type = TokenType::kDot,
                   .lexeme = punctuation_lexeme,
                   .location = token_source_location};
    default:
      break;
  }

  // if start of string
  if (character == '"') {
    return ScanString(token_source_location);
  }

  if (character == '=') {
    return Token{.type = TokenType::kEquals,
                 .lexeme = m_sv_source_code_view.substr(m_position - 1, 1),
                 .location = token_source_location};
  }

  // single-char operators
  if (rng::contains(kSingleCharOperators, character)) {
    return Token{.type = TokenType::kOperator,
                 .lexeme = m_sv_source_code_view.substr(m_position - 1, 1),
                 .location = token_source_location};
  }

  if (character == '$') {
    return ScanSystemIdentifier(token_source_location);
  }

  if (character == '\\') {
    return ScanEscapedIdentifier(token_source_location);
  }

  if (character == '\'') {
    return ScanApostropheToken(token_source_location);
  }

  // identifier or keyword
  if (std::cmp_not_equal(std::isalpha(static_cast<unsigned char>(character)),
                         0) or
      character == '_') {
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

  while (std::cmp_not_equal(std::isdigit(Peek()), 0) or Peek() == '_') {
    m_position += 1;
    m_source_location.column += 1;
  }

  if (Peek() == '\'') {
    m_position += 1;
    m_source_location.column += 1;

    if (std::tolower(Peek()) == 's') {
      m_position += 1;
      m_source_location.column += 1;
    }

    if (std::cmp_not_equal(std::isalpha(Peek()), 0)) {
      m_position += 1;
      m_source_location.column += 1;
    }

    while (IsBasedLiteralDigit(Peek())) {
      m_position += 1;
      m_source_location.column += 1;
    }

    return Token{.type = TokenType::kIntegerLiteral,
                 .lexeme = m_sv_source_code_view.substr(
                     start_position, m_position - start_position),
                 .location = token_source_location};
  }

  if (Peek() == '.') {
    m_position += 1;
    m_source_location.column += 1;

    while (std::isdigit(Peek()) != 0 or Peek() == '_') {
      m_position += 1;
      m_source_location.column += 1;
    }

    return Token{.type = TokenType::kRealLiteral,
                 .lexeme = m_sv_source_code_view.substr(
                     start_position, m_position - start_position),
                 .location = token_source_location};
  }

  while (std::isalpha(Peek()) != 0) {
    m_position += 1;
    m_source_location.column += 1;
  }

  return Token{.type = TokenType::kIntegerLiteral,
               .lexeme = m_sv_source_code_view.substr(
                   start_position, m_position - start_position),
               .location = token_source_location};
}

auto Lexer::ScanIdentifierOrKeyword(SourceLocation const& token_source_location)
    -> Token {
  static constexpr auto kKeywords{std::to_array<std::string_view>({
      "alias",        "always",       "always_comb",  "always_ff",
      "always_latch", "assign",       "assume",       "automatic",
      "begin",        "bind",         "bit",          "case",
      "checker",      "class",        "clocking",     "config",
      "constraint",   "cover",        "default",      "defparam",
      "disable",      "else",         "end",          "endchecker",
      "endclass",     "endclocking",  "endconfig",    "endfunction",
      "endgenerate",  "endgroup",     "endinterface", "endmodule",
      "endpackage",   "endprimitive", "endprogram",   "endproperty",
      "endsequence",  "endtask",      "event",        "export",
      "extern",       "final",        "for",          "function",
      "generate",     "genvar",       "if",           "import",
      "initial",      "input",        "interface",    "inout",
      "localparam",   "logic",        "macromodule",  "module",
      "nettype",      "output",       "package",      "parameter",
      "primitive",    "program",      "ref",          "reg",
      "task",         "wire",
  })};

  auto const start_position{m_position - 1};

  while (IsIdentifierBodyCharacter(Peek())) {
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

auto Lexer::ScanSystemIdentifier(SourceLocation const& token_source_location)
    -> Token {
  auto const start_position{m_position - 1};

  while (IsIdentifierBodyCharacter(Peek())) {
    m_position += 1;
    m_source_location.column += 1;
  }

  return Token{.type = TokenType::kIdentifier,
               .lexeme = m_sv_source_code_view.substr(
                   start_position, m_position - start_position),
               .location = token_source_location};
}

auto Lexer::ScanEscapedIdentifier(SourceLocation const& token_source_location)
    -> Token {
  auto const start_position{m_position - 1};

  while (Peek() != '\0' and std::cmp_equal(std::isspace(Peek()), 0)) {
    m_position += 1;
    m_source_location.column += 1;
  }

  return Token{.type = TokenType::kIdentifier,
               .lexeme = m_sv_source_code_view.substr(
                   start_position, m_position - start_position),
               .location = token_source_location};
}

auto Lexer::ScanApostropheToken(SourceLocation const& token_source_location)
    -> Token {
  auto const start_position{m_position - 1};

  if (std::tolower(Peek()) == 's') {
    m_position += 1;
    m_source_location.column += 1;
  }

  if (Peek() == '0' or Peek() == '1' or std::tolower(Peek()) == 'x' or
      std::tolower(Peek()) == 'z') {
    m_position += 1;
    m_source_location.column += 1;
    return Token{.type = TokenType::kIntegerLiteral,
                 .lexeme = m_sv_source_code_view.substr(
                     start_position, m_position - start_position),
                 .location = token_source_location};
  }

  return Token{.type = TokenType::kOperator,
               .lexeme = m_sv_source_code_view.substr(start_position, 1),
               .location = token_source_location};
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
auto Lexer::Peek() const -> unsigned char {
  try {
    return static_cast<unsigned char>(
        m_sv_source_code_view.at(m_position + kOffset));
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
    translation_unit.push_back(ParseDesignElement());
  }
  return translation_unit;
}

auto Print(Parser::TranslationUnit const& translation_unit) -> void {
  for (auto const& design_element : translation_unit) {
    std::visit(
        [](auto const& resolved_node) -> void {
          if constexpr (std::same_as<
                            std::remove_cvref_t<decltype(resolved_node)>,
                            ModuleDeclaration>) {
            PrintModule(resolved_node);
          } else if constexpr (std::same_as<
                                   std::remove_cvref_t<decltype(resolved_node)>,
                                   UnsupportedDesignElement>) {
            fmt::println("{} <unsupported>", resolved_node.kind);
          } else {
            fmt::println("<unsupported AST node>");
          }
        },
        design_element);
  }
}

auto Parser::ParseDesignElement() -> DesignElement {
  if (IsKeyword(m_token_iterator, "module")) {
    auto const module_keyword_iterator{m_token_iterator};
    m_token_iterator++;

    try {
      return ParseModuleDeclaration();
    } catch (std::runtime_error const&) {
      m_token_iterator = module_keyword_iterator;
      return ParseUnsupportedDesignElement();
    }
  }

  return ParseUnsupportedDesignElement();
}

auto Parser::ParseUnsupportedDesignElement() -> UnsupportedDesignElement {
  auto const element_begin_iterator{m_token_iterator};
  auto const kind{m_token_iterator->lexeme};

  SkipAttributeInstances();

  if (m_token_iterator->type == TokenType::kKeyword) {
    if (auto const end_keyword{
            MatchingTopLevelEndKeyword(m_token_iterator->lexeme)};
        end_keyword.has_value()) {
      SkipUnsupportedDesignElementToMatchingEnd(m_token_iterator->lexeme,
                                                end_keyword.value());
      return UnsupportedDesignElement{
          .kind = kind,
          .tokens = std::span{element_begin_iterator, m_token_iterator}};
    }
  }

  SkipUnsupportedDesignElementToSemicolon();
  if (element_begin_iterator == m_token_iterator and
      m_token_iterator->type != TokenType::kEndOfFile) {
    m_token_iterator++;
  }

  return UnsupportedDesignElement{
      .kind = kind,
      .tokens = std::span{element_begin_iterator, m_token_iterator}};
}

auto Parser::SkipAttributeInstances() -> void {
  while (m_token_iterator->type == TokenType::kLParen and
         rng::next(m_token_iterator, 1, rng::cend(m_tokens))->lexeme == "*") {
    m_token_iterator += 2;
    while (m_token_iterator->type != TokenType::kEndOfFile) {
      if (m_token_iterator->lexeme == "*" and
          rng::next(m_token_iterator, 1, rng::cend(m_tokens))->type ==
              TokenType::kRParen) {
        m_token_iterator += 2;
        break;
      }

      m_token_iterator++;
    }
  }
}

auto Parser::SkipUnsupportedDesignElementToSemicolon() -> void {
  auto delimiter_depth{0UZ};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    if (IsOpeningDelimiter(m_token_iterator)) {
      delimiter_depth++;
    } else if (IsClosingDelimiter(m_token_iterator)) {
      if (std::cmp_not_equal(delimiter_depth, 0UZ)) {
        delimiter_depth--;
      }
    } else if (m_token_iterator->type == TokenType::kSemicolon and
               std::cmp_equal(delimiter_depth, 0UZ)) {
      m_token_iterator++;
      return;
    }

    m_token_iterator++;
  }
}

auto Parser::SkipUnsupportedDesignElementToMatchingEnd(
    std::string_view const start_keyword, std::string_view const end_keyword)
    -> void {
  auto block_depth{0UZ};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    if (IsKeyword(m_token_iterator, start_keyword)) {
      block_depth++;
    } else if (IsKeyword(m_token_iterator, end_keyword)) {
      if (std::cmp_not_equal(block_depth, 0UZ)) {
        block_depth--;
      }

      if (std::cmp_equal(block_depth, 0UZ)) {
        m_token_iterator++;
        if (m_token_iterator->type == TokenType::kColon) {
          m_token_iterator++;
          if (m_token_iterator->type == TokenType::kIdentifier) {
            m_token_iterator++;
          }
        }
        return;
      }
    }

    m_token_iterator++;
  }
}

auto Parser::ParseModuleDeclaration() -> ModuleDeclaration {
  if (IsKeyword(m_token_iterator, "automatic") or
      IsKeyword(m_token_iterator, "static")) {
    m_token_iterator++;
  }

  if (m_token_iterator->type != TokenType::kIdentifier) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected module name at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  ModuleDeclaration module_declaration{};
  module_declaration.name = m_token_iterator->lexeme;
  m_token_iterator++;

  SkipModuleHeaderImports();

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

auto Parser::SkipModuleHeaderImports() -> void {
  while (IsKeyword(m_token_iterator, "import")) {
    SkipUnsupportedDesignElementToSemicolon();
  }
}

auto Parser::ParseModuleItems() -> std::vector<ModuleItem> {
  std::vector<ModuleItem> items{};

  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->lexeme != "endmodule") {
    if (auto module_item{ParseModuleItem()}; module_item.has_value()) {
      items.push_back(module_item.value());
      continue;
    }

    SkipUnsupportedModuleItem();
  }

  if (m_token_iterator->lexeme == "endmodule") {
    m_token_iterator++;
  }

  return items;
}

auto Parser::ParseModuleItem() -> std::optional<ModuleItem> {
  if (IsNetType(m_token_iterator)) {
    return ModuleItem{std::in_place_type<NetDeclaration>,
                      ParseNetDeclaration()};
  }

  if (IsKeyword(m_token_iterator, "assign")) {
    return ModuleItem{std::in_place_type<ContinuousAssign>,
                      ParseContinuousAssign()};
  }

  if (IsKeyword(m_token_iterator, "always")) {
    ExpectToken(TokenType::kKeyword, "always block");
    return ModuleItem{std::in_place_type<AlwaysBlock>,
                      ParseAlwaysEventControl(),
                      IsKeyword(m_token_iterator, "begin")
                          ? ParseBeginEndBlockBody("always block")
                          : ParseSingleStatementBody("always block")};
  }

  if (IsKeyword(m_token_iterator, "initial")) {
    ExpectToken(TokenType::kKeyword, "initial block");
    return ModuleItem{std::in_place_type<InitialBlock>,
                      IsKeyword(m_token_iterator, "begin")
                          ? ParseBeginEndBlockBody("initial block")
                          : ParseSingleStatementBody("initial block")};
  }

  if (IsKeyword(m_token_iterator, "generate")) {
    return ModuleItem{std::in_place_type<GenerateBlock>, ParseGenerateBlock()};
  }

  if (m_token_iterator->type == TokenType::kIdentifier) {
    return ModuleItem{std::in_place_type<ModuleInstantiation>,
                      ParseModuleInstantiation()};
  }

  return std::nullopt;
}

auto Parser::SkipUnsupportedModuleItem() -> void {
  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->type != TokenType::kSemicolon and
         m_token_iterator->lexeme != "endmodule") {
    m_token_iterator++;
  }

  if (m_token_iterator->type == TokenType::kSemicolon) {
    m_token_iterator++;
  }
}

auto Parser::ParseNetDeclaration() -> NetDeclaration {
  auto parse_net_type{[](Token const& token) -> NetType {
    if (token.lexeme == "wire") {
      return NetType::kWire;
    }

    if (token.lexeme == "logic") {
      return NetType::kLogic;
    }

    throw std::runtime_error{
        fmt::format("[Parser] expected net type at ({}, {})",
                    token.location.row, token.location.column)};
  }};

  auto const net_type{parse_net_type(*m_token_iterator)};
  m_token_iterator++;

  auto const declaration_begin_iterator{m_token_iterator};

  auto const declaration_end_iterator{
      rng::find_if(std::span{m_token_iterator, rng::cend(m_tokens)},
                   [](Token const& token) -> bool {
                     return token.type == TokenType::kSemicolon or
                            token.type == TokenType::kEndOfFile;
                   })};
  if (declaration_end_iterator == rng::cend(m_tokens) or
      declaration_end_iterator->type == TokenType::kEndOfFile) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected ';' while parsing net declaration at ({}, {})",
        declaration_begin_iterator->location.row,
        declaration_begin_iterator->location.column)};
  }
  if (declaration_begin_iterator == declaration_end_iterator) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected net name while parsing net declaration at ({}, {})",
        declaration_end_iterator->location.row,
        declaration_end_iterator->location.column)};
  }

  auto reversed_declaration{
      std::span{declaration_begin_iterator, declaration_end_iterator} |
      rng::views::reverse};
  auto const name_riterator{
      rng::find_if(reversed_declaration, [](Token const& token) -> bool {
        return token.type == TokenType::kIdentifier;
      })};
  if (name_riterator == rng::cend(reversed_declaration)) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected net name while parsing net declaration at ({}, {})",
        declaration_begin_iterator->location.row,
        declaration_begin_iterator->location.column)};
  }
  // NOTE(): we do not use bounded check for rng::prev() in following as we are
  // assured that `reversed_declaration` is not empty
  auto const name_iterator{std::prev(name_riterator.base())};

  NetDeclaration net_declaration{};
  net_declaration.type = net_type;
  net_declaration.name = name_iterator->lexeme;
  net_declaration.type_specifier =
      std::span{declaration_begin_iterator, name_iterator};

  m_token_iterator = declaration_end_iterator;
  ExpectToken(TokenType::kSemicolon, "net declaration");

  return net_declaration;
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
  continuous_assign.left_hand_side =
      std::span{assignment_begin_iterator, equals_operator_iterator};
  continuous_assign.right_hand_side =
      std::span{rng::next(equals_operator_iterator, 1, rng::cend(m_tokens)),
                assignment_end_iterator};

  m_token_iterator = assignment_end_iterator;
  ExpectToken(TokenType::kSemicolon, "continuous assignment");

  return continuous_assign;
}

auto Parser::ConsumeBalancedDelimitedTokens(TokenType const opening_token,
                                            TokenType const closing_token,
                                            std::string_view const context)
    -> tokens_t {
  ExpectToken(opening_token, context);

  auto const body_begin_iterator{m_token_iterator};
  auto delimiter_depth{1UZ};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    if (m_token_iterator->type == opening_token) {
      delimiter_depth++;
    } else if (m_token_iterator->type == closing_token) {
      delimiter_depth--;
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        auto const body{tokens_t{body_begin_iterator, m_token_iterator}};
        m_token_iterator++;
        return body;
      }
    }

    m_token_iterator++;
  }

  throw std::runtime_error{fmt::format(
      "[Parser] expected closing delimiter while parsing {} at ({}, "
      "{})",
      context, body_begin_iterator->location.row,
      body_begin_iterator->location.column)};
}

auto Parser::ParseKeywordBlockBody(std::string_view const start_keyword,
                                   std::string_view const end_keyword,
                                   std::string_view const context) -> tokens_t {
  ExpectToken(TokenType::kKeyword, context);

  auto const body_begin_iterator{m_token_iterator};
  auto block_depth{1UZ};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    if (IsKeyword(m_token_iterator, start_keyword)) {
      block_depth++;
    } else if (IsKeyword(m_token_iterator, end_keyword)) {
      block_depth--;
      if (std::cmp_equal(block_depth, 0UZ)) {
        auto const body{tokens_t{body_begin_iterator, m_token_iterator}};
        m_token_iterator++;
        return body;
      }
    }

    m_token_iterator++;
  }

  throw std::runtime_error{
      fmt::format("[Parser] expected '{}' while parsing {} at ({}, {})",
                  end_keyword, context, body_begin_iterator->location.row,
                  body_begin_iterator->location.column)};
}

auto Parser::AdvanceToTopLevelListBoundary(std::string_view const context)
    -> void {
  auto delimiter_depth{0UZ};
  while (m_token_iterator->type != TokenType::kEndOfFile) {
    if (IsOpeningDelimiter(m_token_iterator)) {
      delimiter_depth++;
    } else if (IsClosingDelimiter(m_token_iterator)) {
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        return;
      }

      delimiter_depth--;
    } else if (IsListSeparator(m_token_iterator) and
               std::cmp_equal(delimiter_depth, 0UZ)) {
      return;
    }

    m_token_iterator++;
  }

  throw std::runtime_error{fmt::format(
      "[Parser] unexpected end-of-file while parsing {} at ({}, {})", context,
      m_token_iterator->location.row, m_token_iterator->location.column)};
}

auto Parser::ParseModuleInstantiation() -> ModuleInstantiation {
  auto const module_name_token{*m_token_iterator};
  ExpectToken(TokenType::kIdentifier, "module instantiation");

  tokens_t parameter_overrides{};
  if (m_token_iterator->type == TokenType::kHash) {
    m_token_iterator++;
    parameter_overrides = ConsumeBalancedDelimitedTokens(
        TokenType::kLParen, TokenType::kRParen,
        "module instantiation parameter override");
  }

  auto const instance_name_token{*m_token_iterator};
  ExpectToken(TokenType::kIdentifier, "module instantiation instance name");
  auto const port_connections =
      ConsumeBalancedDelimitedTokens(TokenType::kLParen, TokenType::kRParen,
                                     "module instantiation port connections");
  ExpectToken(TokenType::kSemicolon, "module instantiation");

  return ModuleInstantiation{.module_name = module_name_token.lexeme,
                             .instance_name = instance_name_token.lexeme,
                             .parameter_overrides = parameter_overrides,
                             .port_connections = port_connections};
}

auto Parser::ParseAlwaysEventControl() -> tokens_t {
  if (m_token_iterator->type != TokenType::kAt) {
    return {};
  }

  auto const event_begin_iterator{m_token_iterator};

  m_token_iterator++;
  if (m_token_iterator->type == TokenType::kLParen) {
    ConsumeBalancedDelimitedTokens(TokenType::kLParen, TokenType::kRParen,
                                   "always event control");
  } else {
    m_token_iterator++;
  }

  return std::span{event_begin_iterator, m_token_iterator};
}

auto Parser::ParseGenerateBlock() -> GenerateBlock {
  return GenerateBlock{.body = ParseKeywordBlockBody("generate", "endgenerate",
                                                     "generate block")};
}

auto Parser::ParseBeginEndBlockBody(std::string_view const context)
    -> tokens_t {
  return ParseKeywordBlockBody("begin", "end", context);
}

auto Parser::ParseSingleStatementBody(std::string_view const context)
    -> tokens_t {
  auto const body_begin_iterator{m_token_iterator};
  auto const body_end_iterator{
      rng::find_if(std::span{m_token_iterator, rng::cend(m_tokens)},
                   [](Token const& token) -> bool {
                     return token.type == TokenType::kSemicolon or
                            token.type == TokenType::kEndOfFile;
                   })};

  if (body_begin_iterator == body_end_iterator) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected statement while parsing {} at ({}, {})",
                    context, body_begin_iterator->location.row,
                    body_begin_iterator->location.column)};
  }

  auto const body{std::span{body_begin_iterator, body_end_iterator}};
  m_token_iterator = body_end_iterator;
  ExpectToken(TokenType::kSemicolon, context);

  return body;
}

auto Parser::ParseParameters() -> std::vector<ParameterDeclaration> {
  auto is_parameter_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  std::vector<ParameterDeclaration> result{};

  ExpectToken(TokenType::kLParen, "parameter list");

  while (not is_parameter_list_end()) {
    auto const parameter_tokens{ParseParameterTokens()};
    result.push_back(ParseParameterDeclaration(parameter_tokens));

    if (auto const has_declaration_prefix{
            not rng::empty(parameter_tokens) and
            IsParameterDeclarationPrefix(parameter_tokens)};
        not has_declaration_prefix and
        not std::holds_alternative<ParameterTypeDeclaration>(result.back()) and
        rng::size(result) > 1) {
      auto const& previous_parameter{result.at(result.size() - 2)};
      auto& value_parameter{std::get<ParameterValueDeclaration>(result.back())};

      if (rng::empty(value_parameter.type_specifier)) {
        if (std::holds_alternative<ParameterTypeDeclaration>(
                previous_parameter)) {
          ParameterTypeDeclaration type_parameter{};
          type_parameter.name = value_parameter.name;
          type_parameter.default_type = value_parameter.default_value;
          result.back() = type_parameter;
        } else {
          value_parameter.type_specifier =
              std::get<ParameterValueDeclaration>(previous_parameter)
                  .type_specifier;
        }
      }
    }

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

auto Parser::ParseParameterTokens() -> tokens_t {
  auto const parameter_begin{m_token_iterator};

  AdvanceToTopLevelListBoundary("parameter list");

  return {parameter_begin, m_token_iterator};
}

auto Parser::ParsePorts() -> std::vector<PortDeclaration> {
  std::vector<PortDeclaration> result{};

  auto const is_port_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  while (not is_port_list_end()) {
    auto const port_begin{m_token_iterator};

    AdvanceToTopLevelListBoundary("port list");

    auto const port_tokens{tokens_t{port_begin, m_token_iterator}};
    auto const previous_direction{rng::empty(result)
                                      ? std::nullopt
                                      : std::optional{result.back().direction}};
    if (auto port{TryParsePort(port_tokens, previous_direction)};
        port.has_value()) {
      result.push_back(port.value());
    }

    if (IsListSeparator(m_token_iterator)) {
      m_token_iterator++;
    }
  }

  ExpectToken(TokenType::kRParen, "port list");

  return result;
}

}  // namespace svt::core
