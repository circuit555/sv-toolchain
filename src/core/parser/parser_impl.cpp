

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

namespace {

inline auto IsParameterDeclarationPrefix(std::span<Token const> tokens)
    -> bool {
  return tokens.front().lexeme == "parameter" or
         tokens.front().lexeme == "localparam";
}

// TODO(): replace with token.type such as LParen, LBracket, LBrace, RParen,
inline auto IsOpeningDelimiter(Token const& token) -> bool {
  return token.type == TokenType::kPunctuation and
         (token.lexeme == "(" or token.lexeme == "[" or token.lexeme == "{");
}

inline auto IsClosingDelimiter(Token const& token) -> bool {
  return token.type == TokenType::kPunctuation and
         (token.lexeme == ")" or token.lexeme == "]" or token.lexeme == "}");
}

inline auto IsListSeparator(Token const& token) -> bool {
  return token.type == TokenType::kPunctuation and token.lexeme == ",";
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
        return token.type == TokenType::kOperator and token.lexeme == "=";
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
    : m_lexer{std::move(sv_source_code)},
      m_tokens{m_lexer.Tokens()},
      m_token_iterator{rng::cbegin(m_tokens)} {
}

auto Parser::ExpectToken(TokenType const expected_type,
                         std::string_view const expected_lexeme,
                         std::string_view const context) -> Token {
  if (m_token_iterator->type != expected_type or
      m_token_iterator->lexeme != expected_lexeme) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected '{}' while parsing {} at ({}, {})",
                    expected_lexeme, context, m_token_iterator->location.row,
                    m_token_iterator->location.column)};
  }

  m_token_iterator++;
  return *m_token_iterator;
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

  if (m_token_iterator->type != TokenType::kPunctuation) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected punctuation token (param/port) at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }
  if (m_token_iterator->lexeme != "#" and m_token_iterator->lexeme != "(" and
      m_token_iterator->lexeme != ";") [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] unexpected punctuation token at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  if (m_token_iterator->lexeme == "#") {
    m_token_iterator++;
    module_declaration.parameters = ParseParameters();
  }

  if (m_token_iterator->lexeme == "(") {
    m_token_iterator++;
    module_declaration.ports = ParsePorts();
  }

  if (m_token_iterator->lexeme == ";") {
    m_token_iterator++;
  }

  // FIXME: this is a hack to consume the rest of the module body
  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->lexeme != "endmodule") {
    m_token_iterator++;
  }
  if (m_token_iterator->lexeme == "endmodule") {
    m_token_iterator++;
  }

  return module_declaration;
}

auto Parser::ParseParameters() -> std::vector<ParameterDeclaration> {
  auto is_parameter_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kPunctuation and
           m_token_iterator->lexeme == ")";
  }};

  std::vector<ParameterDeclaration> result{};

  ExpectToken(TokenType::kPunctuation, "(", "parameter list");

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

    if (m_token_iterator->type == TokenType::kPunctuation and
        m_token_iterator->lexeme == ",") {
      m_token_iterator++;
    } else if (not is_parameter_list_end()) [[unlikely]] {
      throw std::runtime_error{fmt::format(
          "[Parser] expected ',' or ')' while parsing parameter "
          "list at ({}, {})",
          m_token_iterator->location.row, m_token_iterator->location.column)};
    }
  }

  ExpectToken(TokenType::kPunctuation, ")", "parameter list");

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
    return m_token_iterator->type == TokenType::kPunctuation and
           m_token_iterator->lexeme == ")";
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

  ExpectToken(TokenType::kPunctuation, ")", "port list");

  return result;
}

}  // namespace svt::core
