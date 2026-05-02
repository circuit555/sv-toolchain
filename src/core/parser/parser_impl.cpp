// SPDX-License-Identifier: MIT

module svt.core.parser;

import std;
import fmt;

namespace rng = std::ranges;

namespace {

using tokens_t = ::svt::core::Parser::tokens_t;
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
using UnsupportedModuleItem = ::svt::model::UnsupportedModuleItem;
using UnsupportedDesignElement = ::svt::model::UnsupportedDesignElement;
using Expression = ::svt::model::Expression;
using ExpressionPtr = ::svt::model::ExpressionPtr;
using ExpressionNode = ::svt::model::ExpressionNode;
using LiteralKind = ::svt::model::LiteralKind;
using IdentifierExpression = ::svt::model::IdentifierExpression;
using SystemIdentifierExpression = ::svt::model::SystemIdentifierExpression;
using LiteralExpression = ::svt::model::LiteralExpression;
using UnaryExpression = ::svt::model::UnaryExpression;
using BinaryExpression = ::svt::model::BinaryExpression;
using ConditionalExpression = ::svt::model::ConditionalExpression;
using IndexExpression = ::svt::model::IndexExpression;
using RangeSelectExpression = ::svt::model::RangeSelectExpression;
using ConcatenationExpression = ::svt::model::ConcatenationExpression;
using ReplicationExpression = ::svt::model::ReplicationExpression;
using CallExpression = ::svt::model::CallExpression;
using AssignmentPatternExpression = ::svt::model::AssignmentPatternExpression;
using UnsupportedExpression = ::svt::model::UnsupportedExpression;
using PackedDimension = ::svt::model::PackedDimension;
using PackedRangeDimension = ::svt::model::PackedRangeDimension;
using PackedSizeDimension = ::svt::model::PackedSizeDimension;
using GenerateExpression = ::svt::model::GenerateExpression;
using GenerateIfExpression = ::svt::model::GenerateIfExpression;
using GenerateForExpression = ::svt::model::GenerateForExpression;

std::array<std::string_view, 16> constexpr kTopLevelEndKeywords{
    "endmodule",    "endpackage",  "endinterface", "endprogram",
    "endprimitive", "endconfig",   "endchecker",   "endclass",
    "endfunction",  "endtask",     "endspecify",   "endclocking",
    "endproperty",  "endsequence", "endgroup",     "endgenerate"};

std::array<std::string_view, 37> constexpr kUnsupportedModuleItemKeywords{
    "reg",       "int",        "integer", "shortint", "longint",  "byte",
    "bit",       "real",       "event",   "genvar",   "time",     "shortreal",
    "chandle",   "realtime",   "typedef", "let",      "defparam", "bind",
    "assert",    "assume",     "cover",   "restrict", "deassign", "nettype",
    "default",   "extern",     "import",  "export",   "alias",    "modport",
    "parameter", "localparam", "input",   "output",   "inout",    "ref",
    "final"};

std::array<std::string_view, 14> constexpr kUnsupportedModuleItemNetTypes{
    "tri",  "tri0", "tri1",    "triand",  "trior", "trireg", "uwire",
    "wand", "wor",  "supply0", "supply1", "wire",  "logic",  "interconnect"};

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

enum class BoundaryEndBehavior : std::uint8_t {
  kStopAtEnd,
  kThrow,
};

auto AdvanceToTopLevelBoundary(tokens_t::iterator& token_iterator,
                               tokens_t::iterator const end_iterator,
                               auto should_stop, auto is_top_level_boundary,
                               bool const stop_at_unmatched_closing_delimiter,
                               std::size_t block_depth, auto is_block_start,
                               auto is_block_end,
                               BoundaryEndBehavior const end_behavior,
                               std::string_view const context) -> void {
  auto delimiter_depth{0UZ};
  while (token_iterator != end_iterator and
         token_iterator->type != TokenType::kEndOfFile) {
    if (should_stop(token_iterator)) {
      return;
    }

    if (is_block_start(token_iterator)) {
      block_depth++;
    } else if (is_block_end(token_iterator)) {
      if (std::cmp_equal(block_depth, 0UZ)) {
        return;
      }

      block_depth--;
      if (std::cmp_equal(block_depth, 0UZ)) {
        return;
      }
    } else if (IsOpeningDelimiter(token_iterator)) {
      delimiter_depth++;
    } else if (IsClosingDelimiter(token_iterator)) {
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        if (stop_at_unmatched_closing_delimiter) {
          return;
        }
      } else {
        delimiter_depth--;
      }
    } else if (std::cmp_equal(delimiter_depth, 0UZ) and
               is_top_level_boundary(token_iterator)) {
      return;
    }

    token_iterator++;
  }

  if (end_behavior == BoundaryEndBehavior::kThrow) {
    throw std::runtime_error{fmt::format(
        "[Parser] unexpected end-of-file while parsing {} at ({}, {})", context,
        token_iterator->location.row, token_iterator->location.column)};
  }
}

auto AdvanceToTopLevelBoundary(tokens_t::iterator& token_iterator,
                               tokens_t::iterator const end_iterator,
                               auto should_stop, auto is_top_level_boundary,
                               bool const stop_at_unmatched_closing_delimiter,
                               BoundaryEndBehavior const end_behavior,
                               std::string_view const context) -> void {
  AdvanceToTopLevelBoundary(
      token_iterator, end_iterator, should_stop, is_top_level_boundary,
      stop_at_unmatched_closing_delimiter, 0UZ,
      [](tokens_t::iterator const) -> bool { return false; },
      [](tokens_t::iterator const) -> bool { return false; }, end_behavior,
      context);
}

auto AdvanceToMatchingEndKeyword(tokens_t::iterator& token_iterator,
                                 tokens_t::iterator const end_iterator,
                                 std::string_view const start_keyword,
                                 std::string_view const end_keyword,
                                 std::size_t const block_depth,
                                 auto matches_keyword) -> void {
  AdvanceToTopLevelBoundary(
      token_iterator, end_iterator,
      [](tokens_t::iterator const) -> bool { return false; },
      [](tokens_t::iterator const) -> bool { return false; }, false,
      block_depth,
      [matches_keyword, start_keyword](tokens_t::iterator const iterator)
          -> bool { return matches_keyword(iterator, start_keyword); },
      [matches_keyword, end_keyword](tokens_t::iterator const iterator)
          -> bool { return matches_keyword(iterator, end_keyword); },
      BoundaryEndBehavior::kStopAtEnd, end_keyword);

  if (token_iterator == end_iterator or
      token_iterator->type == TokenType::kEndOfFile) {
    throw std::runtime_error{
        fmt::format("[Parser] expected '{}' while parsing {} at ({}, {})",
                    start_keyword, end_keyword, token_iterator->location.row,
                    token_iterator->location.column)};
  }
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
    -> std::string_view {
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

  throw std::runtime_error{fmt::format(
      "[Parser] unexpected keyword '{}' while parsing module item", keyword)};
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
auto MatchingModuleItemEndKeyword(tokens_t::iterator const token_iterator)
    -> std::string_view {
  if (token_iterator->lexeme == "function") {
    return kTopLevelEndKeywords[8];
  }
  if (token_iterator->lexeme == "task") {
    return kTopLevelEndKeywords[9];
  }
  if (token_iterator->lexeme == "class") {
    return kTopLevelEndKeywords[7];
  }
  if (token_iterator->lexeme == "specify") {
    return kTopLevelEndKeywords[10];
  }
  if (token_iterator->lexeme == "clocking") {
    return kTopLevelEndKeywords[11];
  }
  if (token_iterator->lexeme == "property") {
    return kTopLevelEndKeywords[12];
  }
  if (token_iterator->lexeme == "sequence") {
    return kTopLevelEndKeywords[13];
  }
  if (token_iterator->lexeme == "covergroup") {
    return kTopLevelEndKeywords[14];
  }
  if (token_iterator->lexeme == "checker") {
    return kTopLevelEndKeywords[6];
  }
  if (token_iterator->lexeme == "default" and
      rng::next(token_iterator, 1)->lexeme == "clocking") {
    return kTopLevelEndKeywords[11];
  }
  if (token_iterator->lexeme == "global" and
      rng::next(token_iterator, 1)->lexeme == "clocking") {
    return kTopLevelEndKeywords[11];
  }

  return {};
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

class ExpressionParser final {
 public:
  explicit ExpressionParser(tokens_t const tokens)
      : m_tokens{tokens}, m_token_iterator{rng::cbegin(m_tokens)} {
    if (rng::empty(m_tokens)) {
      throw std::runtime_error{"[Parser] expected expression"};
    }
  }

  auto Parse() -> ExpressionPtr {
    // In this parser, `ParseConditionalExpression()` is the entry point for
    // “parse any expression” because conditional expression sits near the
    // top/lowest-precedence level of the expression grammar.
    //
    // The flow is:
    //
    // Parse()
    //   -> ParseConditionalExpression()
    //        -> ParseBinaryExpression(1)
    //             -> ParsePrefixExpression()
    //                  -> ParsePostfixExpression()
    //                       -> ParsePrimaryExpression()

    auto expression{ParseConditionalExpression()};
    if (m_token_iterator != rng::cend(m_tokens)) {
      throw std::runtime_error{fmt::format(
          "[Parser] unexpected token '{}' while parsing expression at ({}, {})",
          m_token_iterator->lexeme, m_token_iterator->location.row,
          m_token_iterator->location.column)};
    }

    expression->tokens = m_tokens;
    return expression;
  }

 private:
  auto AtEnd() const -> bool {
    return m_token_iterator == rng::cend(m_tokens) or
           m_token_iterator->type == TokenType::kEndOfFile;
  }

  auto Expect(TokenType const token_type, std::string_view const context)
      -> Token const& {
    if (AtEnd() or m_token_iterator->type != token_type) {
      throw std::runtime_error{
          fmt::format("[Parser] expected token while parsing {}", context)};
    }

    auto const& token{*m_token_iterator};
    m_token_iterator++;
    return token;
  }

  auto Match(TokenType const token_type) -> bool {
    if (not AtEnd() and m_token_iterator->type == token_type) {
      m_token_iterator++;
      return true;
    }

    return false;
  }

  auto ParseConditionalExpression() -> ExpressionPtr {
    auto const begin_iterator{m_token_iterator};
    auto condition{ParseBinaryExpression(1)};
    if (not Match(TokenType::kQuestion)) {
      return condition;
    }

    auto true_expression{ParseConditionalExpression()};
    Expect(TokenType::kColon, "conditional expression");
    auto false_expression{ParseConditionalExpression()};
    return std::make_unique<Expression>(
        ExpressionNode{std::in_place_type<ConditionalExpression>,
                       std::move(condition), std::move(true_expression),
                       std::move(false_expression)},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseBinaryExpression(int const minimum_precedence) -> ExpressionPtr {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    static auto const get_binary_precedence{[](Token const& token) -> int {
      switch (token.type) {
        case TokenType::kEquals:
          return 1;
        case TokenType::kOperator:
          break;
        default:
          return 0;
      }

      switch (token.lexeme.size()) {
        case 1:
          switch (token.lexeme.front()) {
            case '|':
              return 4;
            case '^':
              return 5;
            case '&':
              return 6;
            case '<':
            case '>':
              return 8;
            case '+':
            case '-':
              return 10;
            case '*':
            case '/':
            case '%':
              return 11;
            default:
              return 0;
          }
        case 2:
          switch (token.lexeme.front()) {
            case '|':
              return token.lexeme.back() == '|' ? 2 : 0;
            case '&':
              return token.lexeme.back() == '&' ? 3 : 0;
            case '^':
            case '~':
              return token.lexeme == "^~" or token.lexeme == "~^" ? 5 : 0;
            case '=':
            case '!':
              return token.lexeme == "==" or token.lexeme == "!=" ? 7 : 0;
            case '<':
            case '>':
              return token.lexeme.back() == '=' ? 8 : 9;
            case '*':
              return token.lexeme.back() == '*' ? 12 : 0;
            default:
              return 0;
          }
        case 3:
          switch (token.lexeme.front()) {
            case '=':
            case '!':
              return token.lexeme == "===" or token.lexeme == "!==" or
                             token.lexeme == "==?" or token.lexeme == "!=?"
                         ? 7
                         : 0;
            case '<':
              return token.lexeme == "<<<" ? 9 : 0;
            case '>':
              return token.lexeme == ">>>" ? 9 : 0;
            default:
              return 0;
          }
        default:
          return 0;
      }
    }};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    static auto const is_right_associative{[](Token const& token) -> bool {
      return token.lexeme == "**" or token.type == TokenType::kEquals;
    }};

    auto expression{ParsePrefixExpression()};

    while (not AtEnd()) {
      auto const operator_token{*m_token_iterator};
      auto const precedence{get_binary_precedence(operator_token)};
      if (precedence < minimum_precedence) {
        break;
      }

      m_token_iterator++;
      auto const next_minimum_precedence{
          is_right_associative(operator_token) ? precedence : precedence + 1};
      auto right{ParseBinaryExpression(next_minimum_precedence)};
      auto const begin_iterator{rng::cbegin(expression->tokens)};
      expression = std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<BinaryExpression>,
                         operator_token.lexeme, std::move(expression),
                         std::move(right)},
          tokens_t{begin_iterator, m_token_iterator});
    }

    return expression;
  }

  auto ParsePrefixExpression() -> ExpressionPtr {
    static auto const is_unary_operator{[](Token const& token) -> bool {
      return token.lexeme == "+" or token.lexeme == "-" or
             token.lexeme == "!" or token.lexeme == "~" or
             token.lexeme == "&" or token.lexeme == "|" or
             token.lexeme == "^" or token.lexeme == "~&" or
             token.lexeme == "~|" or token.lexeme == "~^" or
             token.lexeme == "^~";
    }};

    auto const begin_iterator{m_token_iterator};
    if (not AtEnd() and is_unary_operator(*m_token_iterator)) {
      auto const operator_lexeme{m_token_iterator->lexeme};
      m_token_iterator++;
      auto operand{ParsePrefixExpression()};
      return std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<UnaryExpression>, operator_lexeme,
                         std::move(operand)},
          tokens_t{begin_iterator, m_token_iterator});
    }

    return ParsePostfixExpression();
  }

  auto ParsePostfixExpression() -> ExpressionPtr {
    auto expression{ParsePrimaryExpression()};

    while (not AtEnd()) {
      auto const begin_iterator{rng::cbegin(expression->tokens)};

      if (Match(TokenType::kLBracket)) {
        auto left{ParseConditionalExpression()};
        if (Match(TokenType::kColon)) {
          auto right{ParseConditionalExpression()};
          Expect(TokenType::kRBracket, "range select expression");
          expression = std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<RangeSelectExpression>,
                             std::move(expression), std::move(left),
                             std::move(right)},
              tokens_t{begin_iterator, m_token_iterator});
        } else {
          Expect(TokenType::kRBracket, "index expression");
          expression = std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<IndexExpression>,
                             std::move(expression), std::move(left)},
              tokens_t{begin_iterator, m_token_iterator});
        }
      } else if (Match(TokenType::kLParen)) {
        auto arguments{ParseExpressionList(TokenType::kRParen)};
        Expect(TokenType::kRParen, "call expression");
        expression = std::make_unique<Expression>(
            ExpressionNode{std::in_place_type<CallExpression>,
                           std::move(expression), std::move(arguments)},
            tokens_t{begin_iterator, m_token_iterator});
      } else {
        break;
      }
    }

    return expression;
  }

  auto ParsePrimaryExpression() -> ExpressionPtr {
    static auto const match_lexeme{
        [this](std::string_view const lexeme) -> bool {
          if (not AtEnd() and m_token_iterator->lexeme == lexeme) {
            m_token_iterator++;
            return true;
          }

          return false;
        }};

    if (AtEnd()) {
      throw std::runtime_error{"[Parser] expected expression"};
    }

    auto const begin_iterator{m_token_iterator};

    if (m_token_iterator->type == TokenType::kIdentifier) {
      auto const name{m_token_iterator->lexeme};
      m_token_iterator++;
      if (name.starts_with("$")) {
        return std::make_unique<Expression>(
            ExpressionNode{SystemIdentifierExpression{{name}}},
            tokens_t{begin_iterator, m_token_iterator});
      }
      return std::make_unique<Expression>(
          ExpressionNode{IdentifierExpression{{name}}},
          tokens_t{begin_iterator, m_token_iterator});
    }

    if (m_token_iterator->type == TokenType::kIntegerLiteral or
        m_token_iterator->type == TokenType::kRealLiteral or
        m_token_iterator->type == TokenType::kStringLiteral) {
      auto const token{*m_token_iterator};
      m_token_iterator++;
      auto const kind{[token]() -> LiteralKind {
        if (token.type == TokenType::kIntegerLiteral) {
          return LiteralKind::kInteger;
        }
        if (token.type == TokenType::kRealLiteral) {
          return LiteralKind::kReal;
        }
        return LiteralKind::kString;
      }()};
      return std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<LiteralExpression>, kind,
                         token.lexeme},
          tokens_t{begin_iterator, m_token_iterator});
    }

    if (Match(TokenType::kLParen)) {
      auto expression{ParseConditionalExpression()};
      Expect(TokenType::kRParen, "parenthesized expression");
      expression->tokens = tokens_t{begin_iterator, m_token_iterator};
      return expression;
    }

    if (Match(TokenType::kLBrace)) {
      if (not AtEnd() and m_token_iterator->type != TokenType::kRBrace) {
        auto first_expression{ParseConditionalExpression()};
        if (Match(TokenType::kLBrace)) {
          auto replicated_expressions{ParseExpressionList(TokenType::kRBrace)};
          Expect(TokenType::kRBrace, "replication expression");
          Expect(TokenType::kRBrace, "replication expression");
          return std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<ReplicationExpression>,
                             std::move(first_expression),
                             std::move(replicated_expressions)},
              tokens_t{begin_iterator, m_token_iterator});
        }

        std::vector<ExpressionPtr> expressions{};
        expressions.push_back(std::move(first_expression));
        while (Match(TokenType::kComma)) {
          expressions.push_back(ParseConditionalExpression());
        }
        Expect(TokenType::kRBrace, "concatenation expression");
        return std::make_unique<Expression>(
            ExpressionNode{std::in_place_type<ConcatenationExpression>,
                           std::move(expressions)},
            tokens_t{begin_iterator, m_token_iterator});
      }

      std::vector<ExpressionPtr> expressions{};
      Expect(TokenType::kRBrace, "concatenation expression");
      return std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<ConcatenationExpression>,
                         std::move(expressions)},
          tokens_t{begin_iterator, m_token_iterator});
    }

    if (match_lexeme("'{")) {
      auto expressions{ParseExpressionList(TokenType::kRBrace)};
      Expect(TokenType::kRBrace, "assignment pattern expression");
      return std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<AssignmentPatternExpression>,
                         std::move(expressions)},
          tokens_t{begin_iterator, m_token_iterator});
    }

    throw std::runtime_error{"[Parser] expected expression"};
  }

  auto ParseExpressionList(TokenType const closing_token)
      -> std::vector<ExpressionPtr> {
    std::vector<ExpressionPtr> expressions{};

    while (not AtEnd() and m_token_iterator->type != closing_token) {
      expressions.push_back(ParseConditionalExpression());
      if (not Match(TokenType::kComma)) {
        break;
      }
    }

    return expressions;
  }

  tokens_t m_tokens;
  tokens_t::iterator m_token_iterator;
};

auto FindMatchingDelimiter(tokens_t::iterator token_iterator,
                           tokens_t::iterator const end_iterator,
                           TokenType const opening_token,
                           TokenType const closing_token)
    -> tokens_t::iterator {
  auto delimiter_depth{0UZ};
  while (token_iterator != end_iterator) {
    if (token_iterator->type == opening_token) {
      delimiter_depth++;
    } else if (token_iterator->type == closing_token) {
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        return end_iterator;
      }
      delimiter_depth--;
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        return token_iterator;
      }
    }

    token_iterator++;
  }

  return end_iterator;
}

auto ParsePackedDimensions(tokens_t const tokens)
    -> std::vector<PackedDimension> {
  std::vector<PackedDimension> result{};
  auto token_iterator{rng::cbegin(tokens)};
  while (token_iterator != rng::cend(tokens)) {
    if (token_iterator->type != TokenType::kLBracket) {
      token_iterator++;
      continue;
    }

    auto const dimension_begin{token_iterator};
    auto const dimension_end{
        FindMatchingDelimiter(token_iterator, rng::cend(tokens),
                              TokenType::kLBracket, TokenType::kRBracket)};
    if (dimension_end == rng::cend(tokens)) {
      break;
    }

    auto colon_iterator{rng::find_if(
        std::span{rng::next(dimension_begin, 1, dimension_end), dimension_end},
        [](Token const& token) -> bool {
          return token.type == TokenType::kColon;
        })};
    if (colon_iterator != dimension_end) {
      result.emplace_back(PackedRangeDimension{
          .left = ExpressionParser{tokens_t{rng::next(dimension_begin, 1,
                                                      dimension_end),
                                            colon_iterator}}
                      .Parse(),
          .right = ExpressionParser{tokens_t{rng::next(colon_iterator, 1,
                                                       dimension_end),
                                             dimension_end}}
                       .Parse(),
          .tokens = tokens_t{dimension_begin,
                             rng::next(dimension_end, 1, rng::cend(tokens))}});
    } else {
      result.emplace_back(PackedSizeDimension{
          .size = ExpressionParser{tokens_t{rng::next(dimension_begin, 1,
                                                      dimension_end),
                                            dimension_end}}
                      .Parse(),
          .tokens = tokens_t{dimension_begin,
                             rng::next(dimension_end, 1, rng::cend(tokens))}});
    }

    token_iterator = rng::next(dimension_end, 1, rng::cend(tokens));
  }

  return result;
}

auto SplitTopLevelSemicolonExpressions(tokens_t const tokens)
    -> std::vector<tokens_t> {
  std::vector<tokens_t> result{};
  auto expression_begin{rng::cbegin(tokens)};
  for (auto token_iterator{rng::cbegin(tokens)};
       token_iterator != rng::cend(tokens) and
       token_iterator->type != TokenType::kEndOfFile;
       rng::advance(token_iterator, 1, rng::cend(tokens))) {
    expression_begin = token_iterator;

    AdvanceToTopLevelBoundary(
        token_iterator, rng::cend(tokens),
        [](tokens_t::iterator const) -> bool { return false; },
        [](tokens_t::iterator const boundary_iterator) -> bool {
          return boundary_iterator->type == TokenType::kSemicolon;
        },
        false, BoundaryEndBehavior::kStopAtEnd, "semicolon expression");

    if (token_iterator != rng::cend(tokens) and
        token_iterator->type != TokenType::kEndOfFile) [[likely]] {
      result.emplace_back(expression_begin, token_iterator);
    }
  }

  result.emplace_back(expression_begin, rng::cend(tokens));
  return result;
}

auto ParseGenerateExpressions(tokens_t const tokens)
    -> std::vector<GenerateExpression> {
  std::vector<GenerateExpression> result{};
  auto token_iterator{rng::cbegin(tokens)};
  while (token_iterator != rng::cend(tokens)) {
    if ((token_iterator->lexeme != "if" and token_iterator->lexeme != "for") or
        rng::next(token_iterator, 1, rng::cend(tokens)) == rng::cend(tokens) or
        rng::next(token_iterator, 1, rng::cend(tokens))->type !=
            TokenType::kLParen) {
      token_iterator++;
      continue;
    }

    // NOTE(): no need to do bound check here as we are assured that
    // `token_iterator` is not at the end of `tokens` in this code path
    auto const open_paren_iterator{rng::next(token_iterator)};
    auto const close_paren_iterator{
        FindMatchingDelimiter(open_paren_iterator, rng::cend(tokens),
                              TokenType::kLParen, TokenType::kRParen)};
    if (close_paren_iterator == rng::cend(tokens)) {
      throw std::runtime_error{
          fmt::format("[Parser] expected ')' while parsing generate {} "
                      "expression at ({}, "
                      "{})",
                      token_iterator->lexeme, token_iterator->location.row,
                      token_iterator->location.column)};
    }

    auto const expression_tokens{
        tokens_t{rng::next(open_paren_iterator, 1, close_paren_iterator),
                 close_paren_iterator}};
    if (token_iterator->lexeme == "if") {
      result.emplace_back(
          std::in_place_type<GenerateIfExpression>,
          ExpressionParser{expression_tokens}.Parse(),
          tokens_t{token_iterator,
                   rng::next(close_paren_iterator, 1, rng::cend(tokens))});
    } else {
      auto const for_expressions{
          SplitTopLevelSemicolonExpressions(expression_tokens)};
      if (for_expressions.size() == 3) {
        result.emplace_back(
            std::in_place_type<GenerateForExpression>,
            ExpressionParser{for_expressions[0]}.Parse(),
            ExpressionParser{for_expressions[1]}.Parse(),
            ExpressionParser{for_expressions[2]}.Parse(),
            tokens_t{token_iterator,
                     rng::next(close_paren_iterator, 1, rng::cend(tokens))});
      }
    }

    token_iterator = rng::next(close_paren_iterator, 1, rng::cend(tokens));
  }

  return result;
}

auto ParseParameterDeclaration(
    tokens_t const tokens,
    std::optional<ParameterDeclaration const&> const& previous_parameter)
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

      value_parameter.type_specifier = std::span{
          rng::cbegin(tokens_slice), rng::prev(parameter_name_riterator.base(),
                                               1, rng::cbegin(tokens_slice))};

      return value_parameter;
    }();
  }()};

  auto const default_value_tokens{
      equals_operator_iterator == rng::cend(tokens)
          ? tokens_t{}
          : tokens_t{rng::next(equals_operator_iterator, 1, rng::cend(tokens)),
                     rng::cend(tokens)}};

  // Resolve omitted parameter-declaration context before parsing value
  // defaults. For example, handle 'B' and 'U' parameters below:
  //   parameter logic [7:0] A = 1, B = 2
  //   parameter type T = logic, U = bit
  if (not IsParameterDeclarationPrefix(tokens) and
      previous_parameter.has_value() and
      std::holds_alternative<ParameterValueDeclaration>(parameter)) {
    auto& value_parameter{std::get<ParameterValueDeclaration>(parameter)};
    if (rng::empty(value_parameter.type_specifier)) {
      if (std::holds_alternative<ParameterTypeDeclaration>(
              previous_parameter.value())) {
        ParameterTypeDeclaration type_parameter{};
        type_parameter.name = value_parameter.name;
        type_parameter.default_type = default_value_tokens;
        parameter = type_parameter;
      } else {
        value_parameter.type_specifier =
            std::get<ParameterValueDeclaration>(previous_parameter.value())
                .type_specifier;
      }
    }
  }

  if (not rng::empty(default_value_tokens)) {
    std::visit(
        [default_value_tokens](auto& resolved_parameter) -> void {
          if constexpr (std::same_as<
                            std::remove_cvref_t<decltype(resolved_parameter)>,
                            ParameterTypeDeclaration>) {
            resolved_parameter.default_type = default_value_tokens;
          } else {
            resolved_parameter.default_value =
                ExpressionParser{default_value_tokens}.Parse();
          }
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
          fmt::println(
              "    parameter {} {} = {}",
              JoinLexemes(resolved_parameter.type_specifier),
              resolved_parameter.name,
              JoinLexemes(resolved_parameter.default_value
                              ? resolved_parameter.default_value->tokens
                              : std::span<Token const>{}));
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
                           JoinLexemes(resolved_item.left_hand_side->tokens),
                           JoinLexemes(resolved_item.right_hand_side->tokens));
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
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              UnsupportedModuleItem>) {
              fmt::println("    {} <unsupported>", resolved_item.kind);
            }
          },
          item);
    }
  }
}

}  // namespace

namespace svt::core {
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

  // NOTE: for now, we abort early on seeing an unexpected character. Later
  // for rich tooling (e.g. capture all syntax errors in sv source-code), this
  // can be relaxed to just remember this occurrence and continue further
  // lexing
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
      "alias",        "always",      "always_comb", "always_ff",
      "always_latch", "assign",      "assume",      "automatic",
      "begin",        "bind",        "bit",         "case",
      "chandle",      "checker",     "class",       "clocking",
      "config",       "constraint",  "cover",       "covergroup",
      "default",      "defparam",    "disable",     "else",
      "end",          "endchecker",  "endclass",    "endclocking",
      "endconfig",    "endfunction", "endgenerate", "endgroup",
      "endinterface", "endmodule",   "endpackage",  "endprimitive",
      "endprogram",   "endproperty", "endsequence", "endspecify",
      "endtask",      "event",       "export",      "extern",
      "final",        "for",         "function",    "generate",
      "genvar",       "global",      "if",          "import",
      "initial",      "input",       "int",         "integer",
      "interface",    "inout",       "let",         "localparam",
      "logic",        "longint",     "macromodule", "module",
      "nettype",      "output",      "package",     "parameter",
      "primitive",    "program",     "property",    "real",
      "realtime",     "ref",         "reg",         "restrict",
      "sequence",     "shortint",    "shortreal",   "specify",
      "task",         "time",        "typedef",     "wand",
      "wire",         "wor",
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

  // NOTE: we ignore last '"' for lexeme end as this is meant to be excluded
  // in resulting token lexeme
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
    } catch (std::runtime_error const& exception) {
      if (std::string_view{exception.what()}.starts_with(
              "[Parser] expected expression") or
          std::string_view{exception.what()}.starts_with(
              "[Parser] expected ')' while parsing generate")) {
        throw;
      }

      m_token_iterator = module_keyword_iterator;
      return ParseUnsupportedDesignElement();
    }
  }

  return ParseUnsupportedDesignElement();
}

auto Parser::ParseUnsupportedDesignElement() -> UnsupportedDesignElement {
  auto const element_begin_iterator{m_token_iterator};

  SkipAttributeInstances();

  if (m_token_iterator->type == TokenType::kKeyword) {
    try {
      SkipUnsupportedElementToMatchingEnd(
          m_token_iterator->lexeme,
          MatchingTopLevelEndKeyword(m_token_iterator->lexeme), {});
      return UnsupportedDesignElement{
          .kind = element_begin_iterator->lexeme,
          .tokens = std::span{element_begin_iterator, m_token_iterator}};
    } catch (std::runtime_error const& exception) {
      // FIXME: ignoring the unsupported design element for now
    }
  }

  SkipUnsupportedElementToSemicolon();
  if (element_begin_iterator == m_token_iterator and
      m_token_iterator->type != TokenType::kEndOfFile) {
    m_token_iterator++;
  }

  return UnsupportedDesignElement{
      .kind = element_begin_iterator->lexeme,
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

auto Parser::SkipUnsupportedElementToSemicolon(
    std::string_view const stop_keyword) -> void {
  AdvanceToTopLevelBoundary(
      m_token_iterator, rng::cend(m_tokens),
      [stop_keyword](tokens_t::iterator const token_iterator) -> bool {
        return not rng::empty(stop_keyword) and
               token_iterator->lexeme == stop_keyword;
      },
      [](tokens_t::iterator const token_iterator) -> bool {
        return token_iterator->type == TokenType::kSemicolon;
      },
      false, BoundaryEndBehavior::kStopAtEnd, "unsupported element");

  if (m_token_iterator->type == TokenType::kSemicolon) {
    m_token_iterator++;
  }
}

auto Parser::SkipUnsupportedElementToMatchingEnd(
    std::string_view const start_keyword, std::string_view const end_keyword,
    UnsupportedElementEndOptions const& options) -> void {
  auto matches_keyword{[&options](tokens_t::iterator const token_iterator,
                                  std::string_view const keyword) -> bool {
    if (options.match_keyword_tokens_only) {
      return IsKeyword(token_iterator, keyword);
    }

    return token_iterator->lexeme == keyword;
  }};

  AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                              start_keyword, end_keyword, 0UZ, matches_keyword);
  m_token_iterator++;
  if (m_token_iterator->type == TokenType::kColon) {
    m_token_iterator++;
    if ((options.trailing_label_must_be_identifier and
         m_token_iterator->type == TokenType::kIdentifier) or
        (not options.trailing_label_must_be_identifier and
         m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->type != TokenType::kSemicolon)) {
      m_token_iterator++;
    }
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

  while (IsKeyword(m_token_iterator, "import")) {
    SkipUnsupportedElementToSemicolon();
  }

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
    auto const module_item_begin_iterator{m_token_iterator};
    try {
      items.push_back(ParseModuleItem());
    } catch (std::runtime_error const& exception) {
      if (std::string_view{exception.what()}.starts_with(
              "[Parser] expected expression") or
          std::string_view{exception.what()}.starts_with(
              "[Parser] expected ')' while parsing generate")) {
        throw;
      }

      m_token_iterator = module_item_begin_iterator;
      items.emplace_back(std::in_place_type<UnsupportedModuleItem>,
                         ParseUnsupportedModuleItem());
    }
  }

  if (m_token_iterator->lexeme == "endmodule") {
    m_token_iterator++;
  }

  return items;
}

auto Parser::ParseModuleItem() -> ModuleItem {
  if (IsNetType(m_token_iterator)) {
    return ModuleItem{std::in_place_type<NetDeclaration>,
                      ParseNetDeclaration()};
  }

  if (IsKeyword(m_token_iterator, "assign")) {
    return ModuleItem{std::in_place_type<ContinuousAssign>,
                      ParseContinuousAssign()};
  }

  if (IsKeyword(m_token_iterator, "always") or
      IsKeyword(m_token_iterator, "always_ff") or
      IsKeyword(m_token_iterator, "always_comb") or
      IsKeyword(m_token_iterator, "always_latch")) {
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
    ModuleItem result{
        std::in_place_type<GenerateBlock>,
        ParseKeywordBlockBody("generate", "endgenerate", "generate block"),
        std::vector<GenerateExpression>{}};
    auto& generate_block{std::get<GenerateBlock>(result)};
    generate_block.expressions = ParseGenerateExpressions(generate_block.body);
    return result;
  }

  if (not rng::empty(MatchingModuleItemEndKeyword(m_token_iterator)) or
      rng::contains(kUnsupportedModuleItemKeywords, m_token_iterator->lexeme) or
      rng::contains(kUnsupportedModuleItemNetTypes, m_token_iterator->lexeme)) {
    return ModuleItem{std::in_place_type<UnsupportedModuleItem>,
                      ParseUnsupportedModuleItem()};
  }

  if (m_token_iterator->type == TokenType::kIdentifier) {
    return ModuleItem{std::in_place_type<ModuleInstantiation>,
                      ParseModuleInstantiation()};
  }

  throw std::runtime_error{
      fmt::format("[Parser] unexpected token '{}' while parsing module item",
                  m_token_iterator->lexeme)};
}

auto Parser::ParseUnsupportedModuleItem() -> UnsupportedModuleItem {
  auto const module_item_begin_iterator{m_token_iterator};

  if (auto const end_keyword{MatchingModuleItemEndKeyword(m_token_iterator)};
      not rng::empty(end_keyword)) {
    SkipUnsupportedElementToMatchingEnd(
        m_token_iterator->lexeme, end_keyword,
        {.match_keyword_tokens_only = false,
         .require_end_keyword = true,
         .trailing_label_must_be_identifier = false,
         .context = "unsupported module item"});
  } else if (m_token_iterator->lexeme == "final") {
    m_token_iterator++;
    if (m_token_iterator->lexeme == "begin") {
      ParseBeginEndBlockBody("final block");
    } else {
      ParseSingleStatementBody("final block");
    }
  } else {
    SkipUnsupportedElementToSemicolon("endmodule");
  }

  return UnsupportedModuleItem{
      .kind = module_item_begin_iterator->lexeme,
      .tokens = std::span{module_item_begin_iterator, m_token_iterator}};
}

auto Parser::ParseNetDeclaration() -> NetDeclaration {
  auto const net_type{[](tokens_t::iterator const token_iterator) -> NetType {
    if (token_iterator->lexeme == "wire") {
      return NetType::kWire;
    }

    if (token_iterator->lexeme == "logic") {
      return NetType::kLogic;
    }

    throw std::runtime_error{fmt::format(
        "[Parser] expected net type at ({}, {})", token_iterator->location.row,
        token_iterator->location.column)};
  }(m_token_iterator)};

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
    throw std::runtime_error{
        fmt::format("[Parser] expected net name while parsing net "
                    "declaration at ({}, {})",
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
    throw std::runtime_error{
        fmt::format("[Parser] expected net name while parsing net "
                    "declaration at ({}, {})",
                    declaration_begin_iterator->location.row,
                    declaration_begin_iterator->location.column)};
  }
  // NOTE(): we do not use bounded check for rng::prev() in following as we
  // are assured that `reversed_declaration` is not empty
  auto const name_iterator{std::prev(name_riterator.base())};

  NetDeclaration net_declaration{};
  net_declaration.type = net_type;
  net_declaration.name = name_iterator->lexeme;
  net_declaration.type_specifier =
      std::span{declaration_begin_iterator, name_iterator};
  net_declaration.packed_dimensions =
      ParsePackedDimensions(net_declaration.type_specifier);

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
    throw std::runtime_error{
        fmt::format("[Parser] expected '=' while parsing continuous "
                    "assignment at ({}, {})",
                    equals_operator_iterator->location.row,
                    equals_operator_iterator->location.column)};
  }
  if (assignment_begin_iterator == equals_operator_iterator) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected left hand side while parsing "
                    "continuous assignment "
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
    throw std::runtime_error{
        fmt::format("[Parser] expected right hand side while parsing "
                    "continuous assignment "
                    "at ({}, {})",
                    equals_operator_iterator->location.row,
                    equals_operator_iterator->location.column)};
  }

  ContinuousAssign continuous_assign{};
  continuous_assign.left_hand_side = ExpressionParser{
      std::span{
          assignment_begin_iterator,
          equals_operator_iterator}}.Parse();
  continuous_assign.right_hand_side = ExpressionParser{
      std::span{rng::next(equals_operator_iterator, 1, rng::cend(m_tokens)),
                assignment_end_iterator}}.Parse();

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
  AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                              start_keyword, end_keyword, 1UZ, IsKeyword);
  auto const body{tokens_t{body_begin_iterator, m_token_iterator}};
  m_token_iterator++;
  return body;
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

    std::optional<ParameterDeclaration const&> previous_parameter{};
    if (not rng::empty(result)) {
      previous_parameter = std::cref(result.back());
    }

    result.push_back(
        ParseParameterDeclaration(parameter_tokens, previous_parameter));

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

  AdvanceToTopLevelBoundary(
      m_token_iterator, rng::cend(m_tokens),
      [](tokens_t::iterator const) -> bool { return false; }, IsListSeparator,
      true, BoundaryEndBehavior::kThrow, "parameter list");

  return {parameter_begin, m_token_iterator};
}

auto Parser::ParsePorts() -> std::vector<PortDeclaration> {
  std::vector<PortDeclaration> result{};

  auto const is_port_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  while (not is_port_list_end()) {
    auto const port_begin{m_token_iterator};

    AdvanceToTopLevelBoundary(
        m_token_iterator, rng::cend(m_tokens),
        [](tokens_t::iterator const) -> bool { return false; }, IsListSeparator,
        true, BoundaryEndBehavior::kThrow, "port list");

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
