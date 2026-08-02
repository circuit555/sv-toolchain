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
using ModulePort = ::svt::model::ModulePort;
using ModulePortKind = ::svt::model::ModulePortKind;
using PortDirection = ::svt::model::PortDirection;
using NetDeclaration = ::svt::model::NetDeclaration;
using NetType = ::svt::model::NetType;
using VariableDeclaration = ::svt::model::VariableDeclaration;
using VariableDeclarator = ::svt::model::VariableDeclarator;
using VariableType = ::svt::model::VariableType;
using ContinuousAssign = ::svt::model::ContinuousAssign;
using AlwaysBlock = ::svt::model::AlwaysBlock;
using InitialBlock = ::svt::model::InitialBlock;
using FinalBlock = ::svt::model::FinalBlock;
using GenvarDeclaration = ::svt::model::GenvarDeclaration;
using GenvarIdentifier = ::svt::model::GenvarIdentifier;
using GenerateFor = ::svt::model::GenerateFor;
using GenerateIf = ::svt::model::GenerateIf;
using GenerateCase = ::svt::model::GenerateCase;
using GenerateCaseItem = ::svt::model::GenerateCaseItem;
using GenerateRegion = ::svt::model::GenerateRegion;
using GenerateItemNode = ::svt::model::GenerateItemNode;
using GenerateItem = ::svt::model::GenerateItem;
using GenerateItemPtr = ::svt::model::GenerateItemPtr;
using UnsupportedGenerateItem = ::svt::model::UnsupportedGenerateItem;
using ModuleInstantiation = ::svt::model::ModuleInstantiation;
using ModuleItem = ::svt::model::ModuleItem;
using UnsupportedModuleItem = ::svt::model::UnsupportedModuleItem;
using TimeDeclaration = ::svt::model::TimeDeclaration;
using TimeDeclarationKind = ::svt::model::TimeDeclarationKind;
using PackageDeclaration = ::svt::model::PackageDeclaration;
using PackageItem = ::svt::model::PackageItem;
using PackageImportDeclaration = ::svt::model::PackageImportDeclaration;
using PackageExportDeclaration = ::svt::model::PackageExportDeclaration;
using PackageScopeName = ::svt::model::PackageScopeName;
using ImportDeclaration = ::svt::model::ImportDeclaration;
using ExportDeclaration = ::svt::model::ExportDeclaration;
using UnsupportedPackageItem = ::svt::model::UnsupportedPackageItem;
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
using Statement = ::svt::model::Statement;
using StatementPtr = ::svt::model::StatementPtr;
using StatementNode = ::svt::model::StatementNode;
using ProceduralBlockKind = ::svt::model::ProceduralBlockKind;
using AssignmentKind = ::svt::model::AssignmentKind;
using CaseKind = ::svt::model::CaseKind;
using LoopKind = ::svt::model::LoopKind;
using TimingControlKind = ::svt::model::TimingControlKind;
using WaitKind = ::svt::model::WaitKind;
using ForkJoinKind = ::svt::model::ForkJoinKind;
using ProceduralContinuousAssignKind =
    ::svt::model::ProceduralContinuousAssignKind;
using BeginEndBlockStatement = ::svt::model::BeginEndBlockStatement;
using AssignmentStatement = ::svt::model::AssignmentStatement;
using IfElseStatement = ::svt::model::IfElseStatement;
using CaseItem = ::svt::model::CaseItem;
using CaseStatement = ::svt::model::CaseStatement;
using WhileLoopControl = ::svt::model::WhileLoopControl;
using RepeatLoopControl = ::svt::model::RepeatLoopControl;
using ForLoopControl = ::svt::model::ForLoopControl;
using ForeachLoopControl = ::svt::model::ForeachLoopControl;
using LoopControl = ::svt::model::LoopControl;
using LoopStatement = ::svt::model::LoopStatement;
using DelayControl = ::svt::model::DelayControl;
using EventControl = ::svt::model::EventControl;
using TimingControl = ::svt::model::TimingControl;
using TimingControlStatement = ::svt::model::TimingControlStatement;
using WaitExpressionControl = ::svt::model::WaitExpressionControl;
using WaitOrderControl = ::svt::model::WaitOrderControl;
using WaitControl = ::svt::model::WaitControl;
using WaitStatement = ::svt::model::WaitStatement;
using ForkJoinStatement = ::svt::model::ForkJoinStatement;
using ProceduralContinuousAssignStatement =
    ::svt::model::ProceduralContinuousAssignStatement;
using SystemTaskCallStatement = ::svt::model::SystemTaskCallStatement;
using UnsupportedStatement = ::svt::model::UnsupportedStatement;
using PackedDimension = ::svt::model::PackedDimension;
using PackedRangeDimension = ::svt::model::PackedRangeDimension;
using PackedSizeDimension = ::svt::model::PackedSizeDimension;
using TokenParserBase = ::svt::core::TokenParserBase;

enum class BoundaryEndBehavior : std::uint8_t {
  kStopAtEnd,
  kThrow,
};

struct KeywordEndPair {
  std::string_view keyword;
  std::string_view end_keyword;
  bool top_level_start;
};

struct BinaryOperatorPrecedence {
  std::string_view lexeme;
  std::size_t precedence;
};

std::array<std::string_view, 16> constexpr kTopLevelEndKeywords{
    "endmodule",    "endpackage",  "endinterface", "endprogram",
    "endprimitive", "endconfig",   "endchecker",   "endclass",
    "endfunction",  "endtask",     "endspecify",   "endclocking",
    "endproperty",  "endsequence", "endgroup",     "endgenerate"};

std::array<std::string_view, 24> constexpr kUnsupportedModuleItemKeywords{
    "genvar",     "typedef", "let",      "defparam", "bind",    "assert",
    "assume",     "cover",   "restrict", "deassign", "nettype", "default",
    "extern",     "import",  "export",   "alias",    "modport", "parameter",
    "localparam", "input",   "output",   "inout",    "ref",     "final"};

std::array<std::string_view, 14> constexpr kUnsupportedModuleItemNetTypes{
    "tri",  "tri0", "tri1",    "triand",  "trior", "trireg", "uwire",
    "wand", "wor",  "supply0", "supply1", "wire",  "logic",  "interconnect"};

auto constexpr kKeywordEndPairs{std::to_array<KeywordEndPair>({
    {.keyword = "module", .end_keyword = "endmodule", .top_level_start = true},
    {.keyword = "macromodule",
     .end_keyword = "endmodule",
     .top_level_start = true},
    {.keyword = "package",
     .end_keyword = "endpackage",
     .top_level_start = true},
    {.keyword = "interface",
     .end_keyword = "endinterface",
     .top_level_start = true},
    {.keyword = "program",
     .end_keyword = "endprogram",
     .top_level_start = true},
    {.keyword = "primitive",
     .end_keyword = "endprimitive",
     .top_level_start = true},
    {.keyword = "config", .end_keyword = "endconfig", .top_level_start = true},
    {.keyword = "checker",
     .end_keyword = "endchecker",
     .top_level_start = true},
    {.keyword = "class", .end_keyword = "endclass", .top_level_start = true},
    {.keyword = "function",
     .end_keyword = "endfunction",
     .top_level_start = true},
    {.keyword = "task", .end_keyword = "endtask", .top_level_start = true},
    {.keyword = "specify",
     .end_keyword = "endspecify",
     .top_level_start = false},
    {.keyword = "clocking",
     .end_keyword = "endclocking",
     .top_level_start = false},
    {.keyword = "property",
     .end_keyword = "endproperty",
     .top_level_start = false},
    {.keyword = "sequence",
     .end_keyword = "endsequence",
     .top_level_start = false},
    {.keyword = "covergroup",
     .end_keyword = "endgroup",
     .top_level_start = false},
})};

auto constexpr kBinaryOperatorPrecedences{
    std::to_array<BinaryOperatorPrecedence>({
        {.lexeme = "||", .precedence = 2},  {.lexeme = "&&", .precedence = 3},
        {.lexeme = "|", .precedence = 4},   {.lexeme = "^", .precedence = 5},
        {.lexeme = "^~", .precedence = 5},  {.lexeme = "~^", .precedence = 5},
        {.lexeme = "&", .precedence = 6},   {.lexeme = "==", .precedence = 7},
        {.lexeme = "!=", .precedence = 7},  {.lexeme = "===", .precedence = 7},
        {.lexeme = "!==", .precedence = 7}, {.lexeme = "==?", .precedence = 7},
        {.lexeme = "!=?", .precedence = 7}, {.lexeme = "<", .precedence = 8},
        {.lexeme = ">", .precedence = 8},   {.lexeme = "<=", .precedence = 8},
        {.lexeme = ">=", .precedence = 8},  {.lexeme = "<<", .precedence = 9},
        {.lexeme = ">>", .precedence = 9},  {.lexeme = "<<<", .precedence = 9},
        {.lexeme = ">>>", .precedence = 9}, {.lexeme = "+", .precedence = 10},
        {.lexeme = "-", .precedence = 10},  {.lexeme = "*", .precedence = 11},
        {.lexeme = "/", .precedence = 11},  {.lexeme = "%", .precedence = 11},
        {.lexeme = "**", .precedence = 12},
    })};

inline auto IsParameterDeclarationPrefix(tokens_t const tokens) -> bool {
  return tokens.front().lexeme == "parameter" or
         tokens.front().lexeme == "localparam";
}

inline auto TokenMatchesKeyword(tokens_t::iterator const token_iterator,
                                std::string_view const lexeme) -> bool {
  return token_iterator->IsKeyword(lexeme);
}

inline auto IsAttributeInstanceStart(tokens_t tokens) -> bool {
  // NOTE(): unbounded iterator access in below rng::next() is fine
  // as we know the iterator has more than 1 elements
  return std::cmp_greater(rng::size(tokens), 1) and
         tokens.front().type == TokenType::kLParen and
         rng::next(rng::cbegin(tokens))->lexeme == "*";
}

inline auto IsPackageScopeNameStart(tokens_t::iterator const token_iterator,
                                    tokens_t::iterator const end_iterator)
    -> bool {
  if (token_iterator == end_iterator) {
    return false;
  }

  auto const scope_separator_iterator{
      rng::next(token_iterator, 1, end_iterator)};
  auto const name_iterator{rng::next(token_iterator, 2, end_iterator)};
  return name_iterator != end_iterator and
         token_iterator->IsPackageScopeNamePart() and
         scope_separator_iterator->lexeme == "::" and
         name_iterator->IsPackageScopeNamePart();
}

inline auto IsPackageScopeImportStart(tokens_t::iterator const token_iterator,
                                      tokens_t::iterator const end_iterator)
    -> bool {
  return token_iterator->IsKeyword("import") and
         ::IsPackageScopeNameStart(rng::next(token_iterator, 1, end_iterator),
                                   end_iterator);
}

inline auto IsPackageScopeExportStart(tokens_t::iterator const token_iterator,
                                      tokens_t::iterator const end_iterator)
    -> bool {
  return token_iterator->IsKeyword("export") and
         ::IsPackageScopeNameStart(rng::next(token_iterator, 1, end_iterator),
                                   end_iterator);
}

inline auto AdvancePastOptionalBlockLabel(tokens_t::iterator& token_iterator,
                                          tokens_t::iterator const end_iterator)
    -> void {
  if (token_iterator != end_iterator and
      token_iterator->type == TokenType::kColon) {
    rng::advance(token_iterator, 1, end_iterator);
    if (token_iterator != end_iterator and
        token_iterator->type == TokenType::kIdentifier) {
      rng::advance(token_iterator, 1, end_iterator);
    }
  }
}

auto AdvancePastAttributeInstances(tokens_t::iterator& token_iterator,
                                   tokens_t::iterator const end_iterator)
    -> void {
  while (::IsAttributeInstanceStart({token_iterator, end_iterator})) {
    rng::advance(token_iterator, 2, end_iterator);
    while (token_iterator->type != TokenType::kEndOfFile) {
      if (token_iterator->lexeme == "*" and
          rng::next(token_iterator, 1, end_iterator)->type ==
              TokenType::kRParen) {
        rng::advance(token_iterator, 2, end_iterator);
        break;
      }

      rng::advance(token_iterator, 1, end_iterator);
    }
  }
}

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
    } else if (token_iterator->IsOpeningDelimiter()) {
      delimiter_depth++;
    } else if (token_iterator->IsClosingDelimiter()) {
      if (std::cmp_equal(delimiter_depth, 0UZ)) {
        if (stop_at_unmatched_closing_delimiter) {
          return;
        }
      } else {
        delimiter_depth--;
      }
    } else if (std::cmp_equal(delimiter_depth, 0UZ) and
               std::cmp_equal(block_depth, 0UZ) and
               is_top_level_boundary(token_iterator)) {
      return;
    }

    rng::advance(token_iterator, 1, end_iterator);
  }

  if (end_behavior == BoundaryEndBehavior::kThrow) {
    throw std::runtime_error{fmt::format(
        "[Parser] unexpected end-of-file while parsing {} at ({}, {})", context,
        token_iterator->location.row, token_iterator->location.column)};
  }
}

inline auto AdvanceToTopLevelBoundary(
    tokens_t::iterator& token_iterator, tokens_t::iterator const end_iterator,
    auto should_stop, auto is_top_level_boundary,
    bool const stop_at_unmatched_closing_delimiter,
    BoundaryEndBehavior const end_behavior, std::string_view const context)
    -> void {
  AdvanceToTopLevelBoundary(
      token_iterator, end_iterator, should_stop, is_top_level_boundary,
      stop_at_unmatched_closing_delimiter, 0UZ,
      [](tokens_t::iterator const) -> bool { return false; },
      [](tokens_t::iterator const) -> bool { return false; }, end_behavior,
      context);
}

inline auto FindTopLevelAssignmentOperator(
    tokens_t::iterator token_iterator, tokens_t::iterator const end_iterator)
    -> tokens_t::iterator {
  ::AdvanceToTopLevelBoundary(
      token_iterator, end_iterator,
      [](tokens_t::iterator const) -> bool { return false; },
      [](tokens_t::iterator const boundary_iterator) -> bool {
        return boundary_iterator->type == TokenType::kEquals or
               boundary_iterator->lexeme == "<=";
      },
      false, BoundaryEndBehavior::kStopAtEnd, "assignment statement");
  return token_iterator;
}

inline auto FindTopLevelStatementEnd(tokens_t::iterator token_iterator,
                                     tokens_t::iterator const end_iterator)
    -> tokens_t::iterator {
  AdvanceToTopLevelBoundary(
      token_iterator, end_iterator,
      [](tokens_t::iterator const) -> bool { return false; },
      [](tokens_t::iterator const boundary_iterator) -> bool {
        return boundary_iterator->type == TokenType::kSemicolon or
               (boundary_iterator->type == TokenType::kKeyword and
                (boundary_iterator->lexeme == "end" or
                 boundary_iterator->lexeme == "endcase" or
                 boundary_iterator->lexeme == "join" or
                 boundary_iterator->lexeme == "join_any" or
                 boundary_iterator->lexeme == "join_none" or
                 boundary_iterator->lexeme == "else"));
      },
      true, BoundaryEndBehavior::kStopAtEnd, "statement");
  return token_iterator;
}

auto SplitTopLevelSeparatedTokens(tokens_t const tokens,
                                  TokenType const separator,
                                  std::string_view const context,
                                  bool const keep_empty_items)
    -> std::vector<tokens_t> {
  std::vector<tokens_t> result{};
  auto item_begin{rng::cbegin(tokens)};
  auto token_iterator{rng::cbegin(tokens)};
  while (token_iterator != rng::cend(tokens) and
         token_iterator->type != TokenType::kEndOfFile) {
    AdvanceToTopLevelBoundary(
        token_iterator, rng::cend(tokens),
        [](tokens_t::iterator const) -> bool { return false; },
        [separator](tokens_t::iterator const boundary_iterator) -> bool {
          return boundary_iterator->type == separator;
        },
        false, BoundaryEndBehavior::kStopAtEnd, context);

    if (keep_empty_items or item_begin != token_iterator) {
      result.emplace_back(item_begin, token_iterator);
    }
    if (token_iterator == rng::cend(tokens) or
        token_iterator->type == TokenType::kEndOfFile) {
      break;
    }
    rng::advance(token_iterator, 1, rng::cend(tokens));
    item_begin = token_iterator;
  }
  return result;
}

inline auto AdvanceToMatchingEndKeyword(tokens_t::iterator& token_iterator,
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

    rng::advance(token_iterator, 1, end_iterator);
  }

  return end_iterator;
}

auto ConsumeBalancedDelimitedTokens(tokens_t::iterator token_iterator,
                                    tokens_t::iterator const end_iterator,
                                    TokenType const opening_token,
                                    TokenType const closing_token,
                                    std::string_view const context)
    -> tokens_t {
  if (token_iterator == end_iterator or token_iterator->type != opening_token)
      [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected token while parsing {}", context)};
  }

  auto const closing_iterator{FindMatchingDelimiter(
      token_iterator, end_iterator, opening_token, closing_token)};
  if (closing_iterator != end_iterator) {
    return tokens_t{rng::next(token_iterator, 1, closing_iterator),
                    closing_iterator};
  }

  throw std::runtime_error{fmt::format(
      "[Parser] expected closing delimiter while parsing {} at ({}, "
      "{})",
      context, token_iterator->location.row, token_iterator->location.column)};
}

constexpr auto HashLexeme(std::string_view const lexeme) -> std::uint64_t {
  constexpr auto kFnvOffsetBasis{14695981039346656037ULL};
  constexpr auto kFnvPrime{1099511628211ULL};

  auto result{kFnvOffsetBasis};
  for (auto const character : lexeme) {
    result ^= static_cast<unsigned char>(character);
    result *= kFnvPrime;
  }
  return result;
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

template <bool kRequireTopLevelStart>
inline auto FindKeywordEnd(std::string_view const keyword) -> std::string_view {
  if (
      // NOLINTBEGIN(readability-qualified-auto)
      auto const match_iterator{
          rng::find(kKeywordEndPairs, keyword, &KeywordEndPair::keyword)};
      // NOLINTEND(readability-qualified-auto)
      match_iterator != rng::cend(kKeywordEndPairs) and
      (not kRequireTopLevelStart or match_iterator->top_level_start)) {
    return match_iterator->end_keyword;
  }

  throw std::runtime_error{fmt::format(
      "[Parser] unexpected keyword '{}' while parsing module item", keyword)};
}

inline auto MatchingModuleItemEndKeyword(
    tokens_t::iterator const token_iterator) -> std::string_view {
  try {
    return FindKeywordEnd<false>(token_iterator->lexeme);
  } catch (std::runtime_error const&) {
    if ((token_iterator->lexeme == "default" or
         token_iterator->lexeme == "global") and
        rng::next(token_iterator, 1)->lexeme == "clocking") {
      return "endclocking";
    }

    throw;
  }
}

inline auto ParsePackageScopeName(tokens_t const tokens,
                                  std::string_view const context)
    -> PackageScopeName {
  if (std::cmp_not_equal(rng::size(tokens), 3) or
      not tokens[0].IsPackageScopeNamePart() or tokens[1].lexeme != "::" or
      not tokens[2].IsPackageScopeNamePart()) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected package-scope name while parsing {} at ({}, {})",
        context, tokens.empty() ? 0 : tokens.front().location.row,
        tokens.empty() ? 0 : tokens.front().location.column)};
  }

  return PackageScopeName{
      .scope = tokens[0].lexeme, .name = tokens[2].lexeme, .tokens = tokens};
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

auto BinaryPrecedence(Token const& token) -> std::size_t {
  if (token.type == TokenType::kEquals) {
    return 1;
  }

  if (token.type != TokenType::kOperator) {
    return 0;
  }

  // NOLINTBEGIN(readability-qualified-auto)
  auto const precedence_iterator{rng::find(kBinaryOperatorPrecedences,
                                           token.lexeme,
                                           &BinaryOperatorPrecedence::lexeme)};
  // NOLINTEND(readability-qualified-auto)

  return precedence_iterator == rng::cend(kBinaryOperatorPrecedences)
             ? 0UZ
             : precedence_iterator->precedence;
}

class ExpressionParser final : private TokenParserBase {
 public:
  explicit ExpressionParser(tokens_t const tokens) : TokenParserBase{tokens} {
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
  auto ParseConditionalExpression() -> ExpressionPtr {
    auto const begin_iterator{m_token_iterator};
    auto condition{ParseBinaryExpression(1)};
    if (not MatchToken(TokenType::kQuestion)) {
      return condition;
    }

    auto true_expression{ParseConditionalExpression()};
    ExpectToken(TokenType::kColon, "conditional expression");
    auto false_expression{ParseConditionalExpression()};
    return std::make_unique<Expression>(
        ExpressionNode{std::in_place_type<ConditionalExpression>,
                       std::move(condition), std::move(true_expression),
                       std::move(false_expression)},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseBinaryExpression(std::size_t const minimum_precedence)
      -> ExpressionPtr {
    auto expression{ParsePrefixExpression()};

    while (not AtEnd()) {
      auto const operator_token{*m_token_iterator};
      auto const precedence{::BinaryPrecedence(operator_token)};
      if (std::cmp_less(precedence, minimum_precedence)) {
        break;
      }

      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      auto const next_minimum_precedence{
          (operator_token.lexeme == "**" or
           operator_token.type == TokenType::kEquals)
              ? precedence
              : precedence + 1};
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
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
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

      if (MatchToken(TokenType::kLBracket)) {
        auto left{ParseConditionalExpression()};
        if (MatchToken(TokenType::kColon)) {
          auto right{ParseConditionalExpression()};
          ExpectToken(TokenType::kRBracket, "range select expression");
          expression = std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<RangeSelectExpression>,
                             std::move(expression), std::move(left),
                             std::move(right)},
              tokens_t{begin_iterator, m_token_iterator});
        } else {
          ExpectToken(TokenType::kRBracket, "index expression");
          expression = std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<IndexExpression>,
                             std::move(expression), std::move(left)},
              tokens_t{begin_iterator, m_token_iterator});
        }
      } else if (MatchToken(TokenType::kLParen)) {
        auto arguments{ParseExpressionList(TokenType::kRParen)};
        ExpectToken(TokenType::kRParen, "call expression");
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
    auto const match_lexeme{[this](std::string_view const lexeme) -> bool {
      if (not AtEnd() and m_token_iterator->lexeme == lexeme) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
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
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
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
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
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

    if (MatchToken(TokenType::kLParen)) {
      auto expression{ParseConditionalExpression()};
      ExpectToken(TokenType::kRParen, "parenthesized expression");
      expression->tokens = tokens_t{begin_iterator, m_token_iterator};
      return expression;
    }

    if (MatchToken(TokenType::kLBrace)) {
      if (not AtEnd() and m_token_iterator->type != TokenType::kRBrace) {
        auto first_expression{ParseConditionalExpression()};
        if (MatchToken(TokenType::kLBrace)) {
          auto replicated_expressions{ParseExpressionList(TokenType::kRBrace)};
          ExpectToken(TokenType::kRBrace, "replication expression");
          ExpectToken(TokenType::kRBrace, "replication expression");
          return std::make_unique<Expression>(
              ExpressionNode{std::in_place_type<ReplicationExpression>,
                             std::move(first_expression),
                             std::move(replicated_expressions)},
              tokens_t{begin_iterator, m_token_iterator});
        }

        std::vector<ExpressionPtr> expressions{};
        expressions.push_back(std::move(first_expression));
        while (MatchToken(TokenType::kComma)) {
          expressions.push_back(ParseConditionalExpression());
        }
        ExpectToken(TokenType::kRBrace, "concatenation expression");
        return std::make_unique<Expression>(
            ExpressionNode{std::in_place_type<ConcatenationExpression>,
                           std::move(expressions)},
            tokens_t{begin_iterator, m_token_iterator});
      }

      std::vector<ExpressionPtr> expressions{};
      ExpectToken(TokenType::kRBrace, "concatenation expression");
      return std::make_unique<Expression>(
          ExpressionNode{std::in_place_type<ConcatenationExpression>,
                         std::move(expressions)},
          tokens_t{begin_iterator, m_token_iterator});
    }

    if (match_lexeme("'{")) {
      auto expressions{ParseExpressionList(TokenType::kRBrace)};
      ExpectToken(TokenType::kRBrace, "assignment pattern expression");
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
      if (not MatchToken(TokenType::kComma)) {
        break;
      }
    }

    return expressions;
  }
};

inline auto ParseExpressionOrUnsupported(tokens_t const tokens)
    -> ExpressionPtr {
  try {
    return ExpressionParser{tokens}.Parse();
  } catch (std::runtime_error const&) {
    return std::make_unique<Expression>(
        ExpressionNode{std::in_place_type<UnsupportedExpression>, tokens},
        tokens);
  }
}

class StatementParser final : private TokenParserBase {
 public:
  explicit StatementParser(tokens_t const tokens) : TokenParserBase{tokens} {
  }

  auto Parse() -> StatementPtr {
    if (AtEnd()) {
      return std::make_unique<Statement>(
          StatementNode{std::in_place_type<UnsupportedStatement>, m_tokens},
          tokens_t{m_token_iterator, rng::cend(m_tokens)});
    }

    return ParseStatement();
  }

 private:
  auto ParseStatement() -> StatementPtr {
    if (m_token_iterator->IsKeyword("begin")) {
      return ParseBeginEndBlock();
    }
    if (m_token_iterator->IsKeyword("if")) {
      return ParseIfElseStatement();
    }
    if (m_token_iterator->IsKeyword("case") or
        m_token_iterator->IsKeyword("casex") or
        m_token_iterator->IsKeyword("casez")) {
      return ParseCaseStatement();
    }
    if ((m_token_iterator->lexeme == "unique" or
         m_token_iterator->lexeme == "unique0" or
         m_token_iterator->lexeme == "priority") and
        rng::next(m_token_iterator, 1, rng::cend(m_tokens)) !=
            rng::cend(m_tokens) and
        (rng::next(m_token_iterator, 1, rng::cend(m_tokens))->lexeme ==
             "case" or
         rng::next(m_token_iterator, 1, rng::cend(m_tokens))->lexeme ==
             "casex" or
         rng::next(m_token_iterator, 1, rng::cend(m_tokens))->lexeme ==
             "casez")) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      return ParseCaseStatement();
    }
    if (m_token_iterator->IsKeyword("for") or
        m_token_iterator->IsKeyword("while") or
        m_token_iterator->IsKeyword("repeat") or
        m_token_iterator->IsKeyword("foreach") or
        m_token_iterator->IsKeyword("forever")) {
      return ParseLoopStatement();
    }
    if (m_token_iterator->type == TokenType::kAt or
        m_token_iterator->type == TokenType::kHash) {
      return ParseTimingControlStatement();
    }
    if (m_token_iterator->IsKeyword("wait") or
        m_token_iterator->IsKeyword("wait_order")) {
      return ParseWaitStatement();
    }
    if (m_token_iterator->IsKeyword("fork")) {
      return ParseForkJoinStatement();
    }
    if (m_token_iterator->IsKeyword("assign") or
        m_token_iterator->IsKeyword("deassign") or
        m_token_iterator->IsKeyword("force") or
        m_token_iterator->IsKeyword("release")) {
      return ParseProceduralContinuousAssignStatement();
    }
    if (m_token_iterator->type == TokenType::kIdentifier and
        m_token_iterator->lexeme.starts_with("$")) {
      return ParseSystemTaskCallStatement();
    }

    return ParseAssignmentOrUnsupportedStatement();
  }

  auto ParseBeginEndBlock() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};

    MatchKeyword("begin");
    ::AdvancePastOptionalBlockLabel(m_token_iterator, rng::cend(m_tokens));

    std::vector<StatementPtr> statements{};
    while (not AtEnd() and not m_token_iterator->IsKeyword("end")) {
      auto const statement_begin_iterator{m_token_iterator};
      statements.push_back(ParseStatement());
      if (m_token_iterator == statement_begin_iterator) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      }
    }

    if (not MatchKeyword("end")) {
      throw std::runtime_error{fmt::format(
          "[Parser] expected 'end' while parsing begin-end block at ({}, {})",
          begin_iterator->location.row, begin_iterator->location.column)};
    }
    ::AdvancePastOptionalBlockLabel(m_token_iterator, rng::cend(m_tokens));

    return std::make_unique<Statement>(
        StatementNode{std::in_place_type<BeginEndBlockStatement>,
                      std::move(statements)},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseIfElseStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};
    MatchKeyword("if");
    auto const condition_tokens{::ConsumeBalancedDelimitedTokens(
        m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
        TokenType::kRParen, "if statement")};
    m_token_iterator =
        rng::next(rng::cend(condition_tokens), 1, rng::cend(m_tokens));
    auto condition{ParseExpressionOrUnsupported(condition_tokens)};
    auto then_statement{ParseStatement()};
    StatementPtr else_statement{};
    if (MatchKeyword("else")) {
      else_statement = ParseStatement();
    }

    return std::make_unique<Statement>(
        StatementNode{
            IfElseStatement{.condition = std::move(condition),
                            .then_statement = std::move(then_statement),
                            .else_statement = std::move(else_statement)}},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseCaseStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};

    auto const throw_expected_endcase{[this, begin_iterator]() -> void {
      auto const location_iterator{AtEnd() ? begin_iterator : m_token_iterator};
      throw std::runtime_error{fmt::format(
          "[Parser] expected 'endcase' while parsing case statement at ({}, "
          "{})",
          location_iterator->location.row, location_iterator->location.column)};
    }};

    auto const is_statement_block_end{
        [](tokens_t::iterator const token) -> bool {
          return token->IsKeyword("end") or token->IsKeyword("join") or
                 token->IsKeyword("join_any") or
                 token->IsKeyword("join_none") or token->IsKeyword("else");
        }};

    auto const kind{[this]() -> CaseKind {
      if (MatchKeyword("casex")) {
        return CaseKind::kCaseX;
      }
      if (MatchKeyword("casez")) {
        return CaseKind::kCaseZ;
      }
      MatchKeyword("case");
      return CaseKind::kCase;
    }()};

    auto const expression_tokens{::ConsumeBalancedDelimitedTokens(
        m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
        TokenType::kRParen, "case statement")};
    auto expression{ParseExpressionOrUnsupported(expression_tokens)};
    m_token_iterator =
        rng::next(rng::cend(expression_tokens), 1, rng::cend(m_tokens));

    std::vector<CaseItem> items{};
    while (not AtEnd() and not m_token_iterator->IsKeyword("endcase")) {
      if (is_statement_block_end(m_token_iterator)) {
        throw_expected_endcase();
      }

      auto const item_begin_iterator{m_token_iterator};

      if (not m_token_iterator->IsKeyword("default")) {
        auto colon_iterator{m_token_iterator};
        ::AdvanceToTopLevelBoundary(
            colon_iterator, rng::cend(m_tokens),
            [](tokens_t::iterator const token) -> bool {
              return token->IsKeyword("endcase");
            },
            [](tokens_t::iterator const token) -> bool {
              return token->type == TokenType::kColon;
            },
            false, BoundaryEndBehavior::kStopAtEnd, "case item");
        if (colon_iterator == rng::cend(m_tokens) or
            colon_iterator->IsKeyword("endcase")) {
          throw std::runtime_error{fmt::format(
              "[Parser] expected ':' while parsing case item at ({}, {})",
              item_begin_iterator->location.row,
              item_begin_iterator->location.column)};
        }

        m_token_iterator = rng::next(colon_iterator, 1, rng::cend(m_tokens));
        items.emplace_back(
            SplitTopLevelSeparatedTokens({item_begin_iterator, colon_iterator},
                                         TokenType::kComma,
                                         "comma-separated list", false) |
                rng::views::transform(ParseExpressionOrUnsupported) |
                rng::to<std::vector>(),
            ParseStatement(), false);
      } else {
        m_token_iterator =
            rng::next(item_begin_iterator, 1, rng::cend(m_tokens));
        if (not AtEnd() and m_token_iterator->type == TokenType::kColon) {
          rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
        }
        items.emplace_back(std::vector<ExpressionPtr>{}, ParseStatement(),
                           true);
      }
    }

    if (not MatchKeyword("endcase")) {
      throw_expected_endcase();
    }

    return std::make_unique<Statement>(
        StatementNode{std::in_place_type<CaseStatement>, kind,
                      std::move(expression), std::move(items)},
        tokens_t{begin_iterator, m_token_iterator});
  }

  static auto ParseLoopControl(LoopKind const kind,
                               tokens_t const control_tokens) -> LoopControl {
    auto parse_foreach_loop_control{
        [](tokens_t const control_tokens) -> LoopControl {
          LoopControl result{std::in_place_type<ForeachLoopControl>};
          auto& foreach_loop_control{std::get<ForeachLoopControl>(result)};

          auto open_bracket_iterator{rng::cbegin(control_tokens)};
          while (open_bracket_iterator != rng::cend(control_tokens) and
                 open_bracket_iterator->type != TokenType::kLBracket) {
            rng::advance(open_bracket_iterator, 1, rng::cend(control_tokens));
          }
          if (open_bracket_iterator == rng::cend(control_tokens)) {
            throw std::runtime_error{
                "[StatementParser] unable to parse foreach loop"};
          }

          foreach_loop_control.array_expression = ParseExpressionOrUnsupported(
              {rng::cbegin(control_tokens), open_bracket_iterator});

          auto const close_bracket_iterator{FindMatchingDelimiter(
              open_bracket_iterator, rng::cend(control_tokens),
              TokenType::kLBracket, TokenType::kRBracket)};
          if (close_bracket_iterator == rng::cend(control_tokens)) {
            throw std::runtime_error{
                "[StatementParser] unable to parse foreach loop"};
          }

          foreach_loop_control.loop_variables =
              SplitTopLevelSeparatedTokens(
                  {rng::next(open_bracket_iterator, 1, close_bracket_iterator),
                   close_bracket_iterator},
                  TokenType::kComma, "comma-separated list", false) |
              rng::views::filter([](tokens_t const variable) -> bool {
                return rng::size(variable) == 1UZ and
                       variable.front().type == TokenType::kIdentifier;
              }) |
              rng::views::transform(
                  [](tokens_t const variable) -> std::string_view {
                    return variable.front().lexeme;
                  }) |
              rng::to<std::vector>();

          return result;
        }};

    switch (kind) {
      case LoopKind::kFor: {
        auto const expressions{
            SplitTopLevelSeparatedTokens(control_tokens, TokenType::kSemicolon,
                                         "semicolon-separated list", true)};
        if (rng::size(expressions) == 3UZ) {
          return ForLoopControl{
              .initializer = ParseExpressionOrUnsupported(expressions[0]),
              .condition = ParseExpressionOrUnsupported(expressions[1]),
              .step = ParseExpressionOrUnsupported(expressions[2])};
        }
        // Preserve unsupported for-controls, such as declaration forms, until
        // the AST can model the full SystemVerilog for-control grammar.
        return ForLoopControl{
            .initializer = ParseExpressionOrUnsupported(control_tokens),
            .condition = {},
            .step = {}};
      }
      case LoopKind::kWhile:
        return WhileLoopControl{
            .condition = ParseExpressionOrUnsupported(control_tokens)};
      case LoopKind::kRepeat:
        return RepeatLoopControl{
            .count = ParseExpressionOrUnsupported(control_tokens)};
      case LoopKind::kForeach:
        return parse_foreach_loop_control(control_tokens);
      case LoopKind::kForever:
        return std::monostate{};
    }

    throw std::logic_error{"[StatementParser] unexpected loop kind"};
  }

  auto ParseLoopStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};

    auto result{std::make_unique<Statement>(
        StatementNode{std::in_place_type<LoopStatement>})};
    auto& loop_statement{std::get<LoopStatement>(result->node)};

    loop_statement.kind = [this]() -> LoopKind {
      if (MatchKeyword("for")) {
        return LoopKind::kFor;
      }
      if (MatchKeyword("while")) {
        return LoopKind::kWhile;
      }
      if (MatchKeyword("repeat")) {
        return LoopKind::kRepeat;
      }
      if (MatchKeyword("foreach")) {
        return LoopKind::kForeach;
      }
      MatchKeyword("forever");
      return LoopKind::kForever;
    }();

    if (loop_statement.kind != LoopKind::kForever) {
      auto const control_tokens = ::ConsumeBalancedDelimitedTokens(
          m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
          TokenType::kRParen, "loop statement");
      loop_statement.control =
          ParseLoopControl(loop_statement.kind, control_tokens);
      m_token_iterator =
          rng::next(rng::cend(control_tokens), 1, rng::cend(m_tokens));
    }

    loop_statement.body = ParseStatement();

    return result;
  }

  auto ParseTimingControlStatement() -> StatementPtr {
    auto result{std::make_unique<Statement>(
        StatementNode{std::in_place_type<TimingControlStatement>})};
    auto& timing_control_statement{
        std::get<TimingControlStatement>(result->node)};

    auto const begin_iterator{m_token_iterator};
    timing_control_statement.kind = (m_token_iterator->type == TokenType::kAt)
                                        ? TimingControlKind::kEvent
                                        : TimingControlKind::kDelay;
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

    if (not AtEnd() and m_token_iterator->type == TokenType::kLParen) {
      auto const control_tokens{::ConsumeBalancedDelimitedTokens(
          m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
          TokenType::kRParen, "timing control statement")};
      if (timing_control_statement.kind == TimingControlKind::kDelay) {
        timing_control_statement.control = DelayControl{
            .expression = ParseExpressionOrUnsupported(control_tokens)};
      } else {
        timing_control_statement.control = EventControl{
            .events =
                SplitTopLevelSeparatedTokens(control_tokens, TokenType::kComma,
                                             "comma-separated list", false) |
                rng::views::transform(ParseExpressionOrUnsupported) |
                rng::to<std::vector>()};
      }

      m_token_iterator =
          rng::next(rng::cend(control_tokens), 1, rng::cend(m_tokens));
    } else if (not AtEnd() and
               m_token_iterator->type != TokenType::kSemicolon) {
      // Handle simple unparenthesized timing controls such as `#5` and `@ev`.
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

      if (timing_control_statement.kind == TimingControlKind::kDelay) {
        timing_control_statement.control =
            DelayControl{.expression = ParseExpressionOrUnsupported(tokens_t{
                             rng::prev(m_token_iterator), m_token_iterator})};
      } else {
        timing_control_statement.control =
            EventControl{.events = std::vector<ExpressionPtr>{}};
        std::get<EventControl>(timing_control_statement.control)
            .events.push_back(ParseExpressionOrUnsupported(
                tokens_t{rng::prev(m_token_iterator), m_token_iterator}));
      }
    } else {
      throw std::runtime_error{fmt::format(
          "[Parser] expected timing control while parsing timing control "
          "statement at ({}, {})",
          begin_iterator->location.row, begin_iterator->location.column)};
    }

    timing_control_statement.statement = ParseStatement();

    result->tokens = {begin_iterator, m_token_iterator};

    return result;
  }

  auto ParseWaitStatement() -> StatementPtr {
    auto result{std::make_unique<Statement>(
        StatementNode{std::in_place_type<WaitStatement>})};
    auto& wait_statement{std::get<WaitStatement>(result->node)};

    auto const begin_iterator{m_token_iterator};

    wait_statement.kind = [this]() -> WaitKind {
      if (MatchKeyword("wait_order")) {
        return WaitKind::kWaitOrder;
      }
      MatchKeyword("wait");
      if (MatchKeyword("fork")) {
        return WaitKind::kWaitFork;
      }
      return WaitKind::kWait;
    }();

    if (wait_statement.kind == WaitKind::kWaitFork) {
      auto const end_iterator{
          ::FindTopLevelStatementEnd(m_token_iterator, rng::cend(m_tokens))};
      if (end_iterator != rng::cend(m_tokens) and
          end_iterator->type == TokenType::kSemicolon) {
        m_token_iterator = rng::next(end_iterator, 1, rng::cend(m_tokens));
      }
    } else {
      auto const control_tokens = ::ConsumeBalancedDelimitedTokens(
          m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
          TokenType::kRParen, "wait statement");

      if (wait_statement.kind == WaitKind::kWaitOrder) {
        wait_statement.control = WaitOrderControl{
            .events =
                SplitTopLevelSeparatedTokens(control_tokens, TokenType::kComma,
                                             "comma-separated list", false) |
                rng::views::transform(ParseExpressionOrUnsupported) |
                rng::to<std::vector>()};
      } else {
        wait_statement.control = WaitExpressionControl{
            .expression = ParseExpressionOrUnsupported(control_tokens)};
      }
      m_token_iterator =
          rng::next(control_tokens.end(), 1, rng::cend(m_tokens));
      wait_statement.statement = ParseStatement();
    }

    result->tokens = tokens_t{begin_iterator, m_token_iterator};

    return result;
  }

  auto ParseForkJoinStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};
    MatchKeyword("fork");
    ::AdvancePastOptionalBlockLabel(m_token_iterator, rng::cend(m_tokens));

    std::vector<StatementPtr> statements{};
    while (not AtEnd() and not m_token_iterator->IsKeyword("join") and
           not m_token_iterator->IsKeyword("join_any") and
           not m_token_iterator->IsKeyword("join_none")) {
      auto const statement_begin_iterator{m_token_iterator};
      statements.push_back(ParseStatement());
      if (m_token_iterator == statement_begin_iterator) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      }
    }

    auto const kind{[this]() -> ForkJoinKind {
      if (MatchKeyword("join_any")) {
        return ForkJoinKind::kJoinAny;
      }
      if (MatchKeyword("join_none")) {
        return ForkJoinKind::kJoinNone;
      }
      MatchKeyword("join");
      return ForkJoinKind::kJoin;
    }()};

    return std::make_unique<Statement>(
        StatementNode{ForkJoinStatement{.kind = kind,
                                        .statements = std::move(statements)}},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseProceduralContinuousAssignStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};

    auto const kind{[this]() -> ProceduralContinuousAssignKind {
      if (MatchKeyword("assign")) {
        return ProceduralContinuousAssignKind::kAssign;
      }
      if (MatchKeyword("deassign")) {
        return ProceduralContinuousAssignKind::kDeassign;
      }
      if (MatchKeyword("force")) {
        return ProceduralContinuousAssignKind::kForce;
      }
      MatchKeyword("release");
      return ProceduralContinuousAssignKind::kRelease;
    }()};

    auto const statement_end_iterator{
        ::FindTopLevelStatementEnd(m_token_iterator, rng::cend(m_tokens))};
    auto const assignment_operator_iterator{FindTopLevelAssignmentOperator(
        m_token_iterator, statement_end_iterator)};
    auto left_hand_side{ParseExpressionOrUnsupported(
        tokens_t{m_token_iterator, assignment_operator_iterator})};
    ExpressionPtr right_hand_side{};
    if (kind == ProceduralContinuousAssignKind::kAssign or
        kind == ProceduralContinuousAssignKind::kForce) {
      if (assignment_operator_iterator == statement_end_iterator or
          assignment_operator_iterator->type != TokenType::kEquals) {
        throw std::runtime_error{
            "[Parser] expected assignment operator while parsing procedural "
            "continuous assignment"};
      }
      right_hand_side = ParseExpressionOrUnsupported(tokens_t{
          rng::next(assignment_operator_iterator, 1, statement_end_iterator),
          statement_end_iterator});
    } else if (assignment_operator_iterator != statement_end_iterator) {
      throw std::runtime_error{
          "[Parser] unexpected assignment operator while parsing procedural "
          "continuous assignment"};
    }

    m_token_iterator = statement_end_iterator;
    if (not AtEnd() and m_token_iterator->type == TokenType::kSemicolon) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }

    return std::make_unique<Statement>(
        StatementNode{ProceduralContinuousAssignStatement{
            .kind = kind,
            .left_hand_side = std::move(left_hand_side),
            .right_hand_side = std::move(right_hand_side)}},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseSystemTaskCallStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};
    auto const name{m_token_iterator->lexeme};
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

    std::vector<ExpressionPtr> arguments{};
    if (not AtEnd() and m_token_iterator->type == TokenType::kLParen) {
      auto const argument_tokens{::ConsumeBalancedDelimitedTokens(
          m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
          TokenType::kRParen, "system task call statement")};

      arguments =
          SplitTopLevelSeparatedTokens(argument_tokens, TokenType::kComma,
                                       "comma-separated list", false) |
          rng::views::transform(ParseExpressionOrUnsupported) |
          rng::to<std::vector>();

      m_token_iterator =
          rng::next(rng::cend(argument_tokens), 1, rng::cend(m_tokens));
    }

    auto const statement_end_iterator{
        ::FindTopLevelStatementEnd(m_token_iterator, rng::cend(m_tokens))};
    m_token_iterator = statement_end_iterator;
    if (not AtEnd() and m_token_iterator->type == TokenType::kSemicolon) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }

    return std::make_unique<Statement>(
        StatementNode{SystemTaskCallStatement{
            .name = name, .arguments = std::move(arguments)}},
        tokens_t{begin_iterator, m_token_iterator});
  }

  auto ParseAssignmentOrUnsupportedStatement() -> StatementPtr {
    auto const begin_iterator{m_token_iterator};
    auto const statement_end_iterator{
        ::FindTopLevelStatementEnd(m_token_iterator, rng::cend(m_tokens))};
    auto const assignment_operator_iterator{
        FindTopLevelAssignmentOperator(begin_iterator, statement_end_iterator)};

    if (assignment_operator_iterator == statement_end_iterator) {
      m_token_iterator = statement_end_iterator;
      if (not AtEnd() and m_token_iterator->type == TokenType::kSemicolon) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      } else if (m_token_iterator == begin_iterator and not AtEnd()) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      }
      return std::make_unique<Statement>(
          StatementNode{UnsupportedStatement{
              .tokens = tokens_t{begin_iterator, m_token_iterator}}},
          tokens_t{begin_iterator, m_token_iterator});
    }

    auto const kind{assignment_operator_iterator->lexeme == "<="
                        ? AssignmentKind::kNonblocking
                        : AssignmentKind::kBlocking};
    auto left_hand_side{ParseExpressionOrUnsupported(
        tokens_t{begin_iterator, assignment_operator_iterator})};
    auto right_hand_side{ParseExpressionOrUnsupported(tokens_t{
        rng::next(assignment_operator_iterator, 1, statement_end_iterator),
        statement_end_iterator})};
    m_token_iterator = statement_end_iterator;
    if (not AtEnd() and m_token_iterator->type == TokenType::kSemicolon) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }

    return std::make_unique<Statement>(
        StatementNode{
            AssignmentStatement{.kind = kind,
                                .left_hand_side = std::move(left_hand_side),
                                .right_hand_side = std::move(right_hand_side)}},
        tokens_t{begin_iterator, m_token_iterator});
  }
};

auto ParsePackedDimensions(tokens_t const tokens)
    -> std::vector<PackedDimension> {
  std::vector<PackedDimension> result{};
  auto token_iterator{rng::cbegin(tokens)};
  while (token_iterator != rng::cend(tokens)) {
    if (token_iterator->type != TokenType::kLBracket) {
      rng::advance(token_iterator, 1, rng::cend(tokens));
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

auto ParsePackedDimensionTokens(tokens_t const tokens)
    -> std::vector<tokens_t> {
  std::vector<tokens_t> result{};
  auto token_iterator{rng::cbegin(tokens)};
  while (token_iterator != rng::cend(tokens)) {
    if (token_iterator->type != TokenType::kLBracket) {
      rng::advance(token_iterator, 1, rng::cend(tokens));
      continue;
    }

    auto const dimension_begin{token_iterator};
    auto const dimension_end{
        FindMatchingDelimiter(token_iterator, rng::cend(tokens),
                              TokenType::kLBracket, TokenType::kRBracket)};
    if (dimension_end == rng::cend(tokens)) {
      break;
    }

    result.emplace_back(dimension_begin,
                        rng::next(dimension_end, 1, rng::cend(tokens)));
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

auto ToGenerateItemPtrs(std::vector<GenerateItem> items)
    -> std::vector<GenerateItemPtr> {
  std::vector<GenerateItemPtr> result{};
  result.reserve(items.size());
  for (auto& item : items) {
    result.push_back(std::make_unique<GenerateItem>(std::move(item)));
  }
  return result;
}

class GenerateItemParser final : private TokenParserBase {
 public:
  using TokenParserBase::CurrentTokenIterator;

  explicit GenerateItemParser(tokens_t const tokens)
      : TokenParserBase{tokens, true} {
  }

  auto ParseAll() -> std::vector<GenerateItem> {
    std::vector<GenerateItem> result{};
    while (not AtEnd()) {
      result.push_back(ParseOne());
    }
    return result;
  }

  auto ParseOne() -> GenerateItem {
    auto const item_begin{m_token_iterator};
    switch (m_token_iterator->type) {
      case TokenType::kKeyword: {
        switch (::HashLexeme(m_token_iterator->lexeme)) {
          case ::HashLexeme("generate"):
            return ParseGenerateRegion();
          case ::HashLexeme("genvar"):
            return ParseGenvarDeclaration();
          case ::HashLexeme("for"):
            return ParseGenerateFor();
          case ::HashLexeme("if"):
            return ParseGenerateIf();
          case ::HashLexeme("case"):
            return ParseGenerateCase();
          case ::HashLexeme("wire"):
          case ::HashLexeme("logic"):
            return GenerateItem{
                .node = GenerateItemNode{std::in_place_type<NetDeclaration>,
                                         ParseNetDeclaration()},
                .tokens = {item_begin, m_token_iterator}};
          case ::HashLexeme("assign"):
            return GenerateItem{
                .node = GenerateItemNode{std::in_place_type<ContinuousAssign>,
                                         ParseContinuousAssign()},
                .tokens = {item_begin, m_token_iterator}};
          default:
            break;
        }

        break;
      }

      case TokenType::kIdentifier:
        return GenerateItem{
            .node = GenerateItemNode{std::in_place_type<ModuleInstantiation>,
                                     ParseModuleInstantiation()},
            .tokens = {item_begin, m_token_iterator}};

      default:
        break;
    }

    throw std::runtime_error{
        "[GenerateItemParser] unable to parse generate construct"};
  }

 private:
  struct Body {
    std::vector<GenerateItemPtr> items;
    std::string_view name;
    tokens_t tokens;
  };

  [[nodiscard]] auto FindHeaderCloseParen(tokens_t::iterator const keyword,
                                          std::string_view const context) const
      -> tokens_t::iterator {
    auto const open_paren{rng::next(keyword, 1, rng::cend(m_tokens))};
    if (open_paren == rng::cend(m_tokens) or
        open_paren->type != TokenType::kLParen) [[unlikely]] {
      throw std::runtime_error{fmt::format(
          "[Parser] expected '(' while parsing {} at ({}, {})", context,
          keyword->location.row, keyword->location.column)};
    }
    auto const close_paren{
        FindMatchingDelimiter(open_paren, rng::cend(m_tokens),
                              TokenType::kLParen, TokenType::kRParen)};
    if (close_paren == rng::cend(m_tokens)) [[unlikely]] {
      throw std::runtime_error{fmt::format(
          "[Parser] expected ')' while parsing {} expression at ({}, {})",
          context, keyword->location.row, keyword->location.column)};
    }
    return close_paren;
  }

  auto ParseGenerateRegion() -> GenerateItem {
    auto const region_begin_iterator{m_token_iterator};
    ExpectKeyword("generate", "generate block");

    auto const body_begin_iterator{m_token_iterator};
    ::AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                                  "generate", "endgenerate", 1UZ,
                                  TokenMatchesKeyword);
    auto const body_end_iterator{m_token_iterator};

    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

    return GenerateItem{
        .node = GenerateRegion{.items = ToGenerateItemPtrs(GenerateItemParser{
                                   tokens_t{body_begin_iterator,
                                            body_end_iterator}}.ParseAll())},
        .tokens = tokens_t{region_begin_iterator, m_token_iterator}};
  }

  auto ParseGenvarDeclaration() -> GenerateItem {
    auto const get_identifier_name{
        [](auto const declaration) -> std::string_view {
          auto const name_iterator{
              rng::find_if(declaration, [](Token const& token) -> bool {
                return token.type == TokenType::kIdentifier;
              })};
          if (name_iterator == rng::cend(declaration)) {
            throw std::runtime_error{
                fmt::format("[Parser] expected identifier while "
                            "parsing genvar declaration")};
          }
          return name_iterator->lexeme;
        }};

    auto const get_identifier_initializer{
        [this](auto const declaration) -> ExpressionPtr {
          auto const equals{
              rng::find_if(declaration, [](Token const& token) -> bool {
                return token.type == TokenType::kEquals;
              })};
          if (equals != rng::cend(declaration)) {
            return ParseExpressionOrUnsupported(
                {rng::next(equals, 1, rng::cend(declaration)),
                 rng::cend(declaration)});
          }
          return {};
        }};

    auto const begin_iterator{m_token_iterator};
    ExpectKeyword("genvar", "genvar declaration");

    auto const declaration_begin{m_token_iterator};

    auto declaration_end{m_token_iterator};
    ::AdvanceToTopLevelBoundary(
        declaration_end, rng::cend(m_tokens),
        [](tokens_t::iterator const) -> bool { return false; },
        [](tokens_t::iterator const token) -> bool {
          return token->type == TokenType::kSemicolon;
        },
        false, BoundaryEndBehavior::kThrow, "genvar declaration");
    m_token_iterator = rng::next(declaration_end, 1, rng::cend(m_tokens));

    return {.node =
                GenerateItemNode{
                    std::in_place_type<GenvarDeclaration>,
                    SplitTopLevelSeparatedTokens(
                        {declaration_begin, declaration_end}, TokenType::kComma,
                        "comma-separated list", false) |
                        rng::views::transform(
                            [this, &get_identifier_name,
                             &get_identifier_initializer](
                                auto const declaration) -> GenvarIdentifier {
                              return GenvarIdentifier{
                                  .name = get_identifier_name(declaration),
                                  .initializer =
                                      get_identifier_initializer(declaration),
                                  .tokens = declaration};
                            }) |
                        rng::to<std::vector>()},
            .tokens = {begin_iterator, m_token_iterator}};
  }

  auto ParseGenerateFor() -> GenerateItem {
    auto const get_initialization_without_genvar{
        [](tokens_t const tokens) -> tokens_t {
          if (not rng::empty(tokens) and tokens.front().lexeme == "genvar") {
            return {rng::next(rng::cbegin(tokens), 1, rng::cend(tokens)),
                    rng::cend(tokens)};
          }
          return tokens;
        }};

    auto const get_loop_variable{[](tokens_t const tokens) -> std::string_view {
      auto const loop_variable{
          rng::find_if(tokens, [](Token const& token) -> bool {
            return token.type == TokenType::kIdentifier;
          })};
      return loop_variable == rng::cend(tokens) ? std::string_view{}
                                                : loop_variable->lexeme;
    }};

    auto const for_begin_iterator{m_token_iterator};
    auto const close_paren_iterator{
        FindHeaderCloseParen(for_begin_iterator, "generate for")};
    auto const for_expressions{SplitTopLevelSemicolonExpressions(
        tokens_t{rng::next(for_begin_iterator, 2, close_paren_iterator),
                 close_paren_iterator})};
    if (rng::size(for_expressions) != 3) [[unlikely]] {
      throw std::runtime_error{
          fmt::format("[Parser] expected three expressions while parsing "
                      "generate for at ({}, {})",
                      for_begin_iterator->location.row,
                      for_begin_iterator->location.column)};
    }

    auto const initialization_tokens{
        get_initialization_without_genvar(for_expressions[0])};
    m_token_iterator = rng::next(close_paren_iterator, 1, rng::cend(m_tokens));
    auto body{ParseGenerateBody()};

    return GenerateItem{
        .node =
            GenerateItemNode{
                std::in_place_type<GenerateFor>,
                get_loop_variable(initialization_tokens),
                ParseExpressionOrUnsupported(initialization_tokens),
                ParseExpressionOrUnsupported(for_expressions[1]),
                ParseExpressionOrUnsupported(for_expressions[2]),
                std::move(body.items), body.name},
        .tokens = tokens_t{for_begin_iterator, m_token_iterator}};
  }

  auto ParseGenerateIf() -> GenerateItem {
    auto const parse_optional_else_body{[this]() -> Body {
      if (not AtEnd() and m_token_iterator->IsKeyword("else")) {
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
        return ParseGenerateBody();
      }
      return {};
    }};

    auto const if_begin_iterator{m_token_iterator};
    auto const close_paren_iterator{
        FindHeaderCloseParen(if_begin_iterator, "generate if")};
    auto const condition_tokens{
        tokens_t{rng::next(if_begin_iterator, 2, close_paren_iterator),
                 close_paren_iterator}};
    m_token_iterator = rng::next(close_paren_iterator, 1, rng::cend(m_tokens));

    auto then_body{ParseGenerateBody()};
    auto else_body{parse_optional_else_body()};

    return GenerateItem{
        .node = GenerateIf{.condition =
                               ParseExpressionOrUnsupported(condition_tokens),
                           .then_body = std::move(then_body.items),
                           .else_body = std::move(else_body.items),
                           .then_block_name = then_body.name,
                           .else_block_name = else_body.name},
        .tokens = tokens_t{if_begin_iterator, m_token_iterator}};
  }

  auto ParseGenerateCase() -> GenerateItem {
    auto const case_begin{m_token_iterator};
    auto const close_paren{FindHeaderCloseParen(case_begin, "generate case")};
    auto const expression_tokens{
        tokens_t{rng::next(case_begin, 2, close_paren), close_paren}};
    m_token_iterator = rng::next(close_paren, 1, rng::cend(m_tokens));

    std::vector<GenerateCaseItem> case_items{};
    while (not AtEnd() and not m_token_iterator->IsKeyword("endcase")) {
      auto const item_begin{m_token_iterator};
      bool const is_default{m_token_iterator->IsKeyword("default")};
      auto colon{m_token_iterator};
      ::AdvanceToTopLevelBoundary(
          colon, rng::cend(m_tokens),
          [](tokens_t::iterator const token) -> bool {
            return token->IsKeyword("endcase");
          },
          [](tokens_t::iterator const token) -> bool {
            return token->type == TokenType::kColon;
          },
          false, BoundaryEndBehavior::kStopAtEnd, "generate case item");
      if (colon == rng::cend(m_tokens) or colon->IsKeyword("endcase")) {
        break;
      }

      auto labels =
          is_default ? std::vector<ExpressionPtr>{}
                     : SplitTopLevelSeparatedTokens(
                           {item_begin, colon}, TokenType::kComma,
                           "comma-separated list", false) |
                           rng::views::transform(
                               [this](tokens_t const label) -> ExpressionPtr {
                                 return ParseExpressionOrUnsupported(label);
                               }) |
                           rng::to<std::vector>();
      m_token_iterator = rng::next(colon, 1, rng::cend(m_tokens));

      auto body{ParseGenerateBody()};

      case_items.push_back(
          GenerateCaseItem{.expressions = std::move(labels),
                           .is_default = is_default,
                           .body = std::move(body.items),
                           .tokens = {item_begin, m_token_iterator}});
    }

    ExpectKeyword("endcase", "generate case");

    return GenerateItem{
        .node = GenerateCase{.expression = ParseExpressionOrUnsupported(
                                 expression_tokens),
                             .items = std::move(case_items)},
        .tokens = {case_begin, m_token_iterator}};
  }

  auto ParseGenerateBody() -> Body {
    if (not AtEnd() and m_token_iterator->IsKeyword("begin")) {
      return ParseBeginEndBody();
    }

    auto const body_begin{m_token_iterator};
    return Body{.items = [this]() -> std::vector<GenerateItemPtr> {
                  std::vector<GenerateItemPtr> result(1);
                  result.back() = std::make_unique<GenerateItem>(ParseOne());
                  return result;
                }(),
                .tokens = {body_begin, m_token_iterator}};
  }

  auto ParseBeginEndBody() -> Body {
    auto const block_begin_iterator{m_token_iterator};
    ExpectKeyword("begin", "generate block");
    std::string_view name{};
    if (not AtEnd() and m_token_iterator->type == TokenType::kColon) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      if (m_token_iterator->type == TokenType::kIdentifier) {
        name = m_token_iterator->lexeme;
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
      }
    }

    auto const body_begin{m_token_iterator};
    ::AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                                  "begin", "end", 1UZ, TokenMatchesKeyword);
    auto const body_end{m_token_iterator};
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

    ::AdvancePastOptionalBlockLabel(m_token_iterator, rng::cend(m_tokens));

    return Body{
        .items =
            GenerateItemParser{tokens_t{body_begin, body_end}}.ParseAll() |
            rng::views::transform([](GenerateItem& item) -> GenerateItemPtr {
              return std::make_unique<GenerateItem>(std::move(item));
            }) |
            rng::to<std::vector>(),
        .name = name,
        .tokens = {block_begin_iterator, m_token_iterator}};
  }
};

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
      tokens_t{rng::next(parameter_begin_iterator, 1, rng::cend(tokens)),
               rng::cend(tokens)},
      [](Token const& token) -> bool {
        return token.type == TokenType::kEquals;
      })};

  auto const tokens_slice{
      tokens_t{parameter_begin_iterator,
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

      value_parameter.type_specifier = tokens_t{
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

auto StripLeadingAttributes(tokens_t port_tokens) -> tokens_t {
  while (
      not rng::empty(port_tokens) and
      port_tokens.front().type == TokenType::kLParen and
      rng::size(port_tokens) > 1 and
      rng::next(rng::cbegin(port_tokens), 1, rng::cend(port_tokens))->lexeme ==
          "*") {
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

      rng::advance(token_iterator, 1, rng::cend(port_tokens));
    }

    if (token_iterator == rng::cend(port_tokens)) {
      return {};
    }
  }

  return port_tokens;
}

auto constexpr kVariableTypes{std::array{
    std::pair{"reg", VariableType::kReg}, std::pair{"int", VariableType::kInt},
    std::pair{"integer", VariableType::kInteger},
    std::pair{"shortint", VariableType::kShortint},
    std::pair{"longint", VariableType::kLongint},
    std::pair{"byte", VariableType::kByte},
    std::pair{"bit", VariableType::kBit},
    std::pair{"real", VariableType::kReal},
    std::pair{"time", VariableType::kTime},
    std::pair{"shortreal", VariableType::kShortreal},
    std::pair{"chandle", VariableType::kChandle},
    std::pair{"realtime", VariableType::kRealtime},
    std::pair{"event", VariableType::kEvent},
    std::pair{"string", VariableType::kString}}};

inline auto FindVariableType(std::string_view const lexeme)
    -> std::optional<VariableType> {
  auto const iterator{
      rng::find_if(kVariableTypes, [lexeme](auto const& variable_type) -> bool {
        return variable_type.first == lexeme;
      })};
  if (iterator == rng::cend(kVariableTypes)) {
    return std::nullopt;
  }
  return iterator->second;
}

auto ParsePortDeclaration(
    tokens_t const port_tokens,
    std::optional<PortDirection> const& previous_direction) -> ModulePort {
  auto is_port_direction_token{[](Token const& token) -> bool {
    return token.lexeme == "input" or token.lexeme == "output" or
           token.lexeme == "inout" or token.lexeme == "ref";
  }};

  auto parse_port_direction{[](Token const& token) -> PortDirection {
    if (token.lexeme == "input") {
      return PortDirection::kInput;
    }

    if (token.lexeme == "output") {
      return PortDirection::kOutput;
    }

    if (token.lexeme == "inout") {
      return PortDirection::kInout;
    }

    if (token.lexeme == "ref") {
      return PortDirection::kRef;
    }

    throw std::runtime_error{
        fmt::format("[Parser] expected port direction at ({}, {})",
                    token.location.row, token.location.column)};
  }};

  if (rng::empty(port_tokens)) [[unlikely]] {
    throw std::runtime_error{"[Parser] expected port declaration"};
  }

  auto const stripped_port_tokens{StripLeadingAttributes(port_tokens)};
  if (rng::empty(stripped_port_tokens)) [[unlikely]] {
    throw std::runtime_error{"[Parser] expected port declaration"};
  }

  auto const has_explicit_direction{
      is_port_direction_token(stripped_port_tokens.front())};
  auto const is_interface_port{stripped_port_tokens.front().lexeme ==
                               "interface"};
  if (not has_explicit_direction and not is_interface_port and
      not previous_direction.has_value()) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected port direction at ({}, {})",
                    stripped_port_tokens.front().location.row,
                    stripped_port_tokens.front().location.column)};
  }

  auto const default_begin_iterator{
      rng::find_if(stripped_port_tokens, [](Token const& token) -> bool {
        return token.type == TokenType::kEquals;
      })};
  auto const declaration_end_iterator{default_begin_iterator ==
                                              rng::cend(stripped_port_tokens)
                                          ? rng::cend(stripped_port_tokens)
                                          : default_begin_iterator};
  auto const declaration_tokens{
      tokens_t{rng::cbegin(stripped_port_tokens), declaration_end_iterator}};
  auto const port_name_iterator{
      rng::find_if(declaration_tokens | rng::views::reverse,
                   [&is_port_direction_token](Token const& token) -> bool {
                     return token.type == TokenType::kIdentifier and
                            not is_port_direction_token(token);
                   })};
  if (port_name_iterator == rng::crend(declaration_tokens)) [[unlikely]] {
    throw std::runtime_error{
        fmt::format("[Parser] expected port name at ({}, {})",
                    stripped_port_tokens.front().location.row,
                    stripped_port_tokens.front().location.column)};
  }
  auto const name_iterator{rng::prev(port_name_iterator.base())};
  auto const attributes_end_iterator{rng::cbegin(stripped_port_tokens)};
  auto const type_begin_iterator{
      has_explicit_direction ? rng::next(rng::cbegin(stripped_port_tokens), 1,
                                         rng::cend(stripped_port_tokens))
                             : rng::cbegin(stripped_port_tokens)};
  auto const unpacked_dimensions_end_iterator{
      declaration_end_iterator == rng::cend(stripped_port_tokens)
          ? rng::cend(stripped_port_tokens)
          : declaration_end_iterator};

  ModulePort port{};
  port.kind = ModulePortKind::kImplicit;
  port.tokens = port_tokens;
  port.name = name_iterator->lexeme;
  port.attributes = tokens_t{rng::cbegin(port_tokens), attributes_end_iterator};
  if (has_explicit_direction) {
    port.direction = parse_port_direction(stripped_port_tokens.front());
  } else if (previous_direction.has_value() and not is_interface_port) {
    port.direction = previous_direction.value();
  }
  if (is_interface_port) {
    port.interface_type = tokens_t{type_begin_iterator, name_iterator};
  } else {
    port.type_specifier = tokens_t{type_begin_iterator, name_iterator};
    port.packed_dimensions = ParsePackedDimensionTokens(port.type_specifier);
  }
  port.unpacked_dimensions =
      tokens_t{rng::next(name_iterator, 1, unpacked_dimensions_end_iterator),
               unpacked_dimensions_end_iterator};
  if (default_begin_iterator != rng::cend(stripped_port_tokens)) {
    port.default_value = tokens_t{
        rng::next(default_begin_iterator, 1, rng::cend(stripped_port_tokens)),
        rng::cend(stripped_port_tokens)};
  }
  return port;
}

auto TryParsePort(tokens_t const port_tokens,
                  std::optional<PortDirection> const previous_direction)
    -> std::optional<ModulePort> {
  auto const stripped_port_tokens{StripLeadingAttributes(port_tokens)};
  if (rng::empty(stripped_port_tokens)) {
    return std::nullopt;
  }

  if (stripped_port_tokens.front().type == TokenType::kDot) {
    return std::nullopt;
  }

  if (auto const has_explicit_direction{
          stripped_port_tokens.front().lexeme == "input" or
          stripped_port_tokens.front().lexeme == "output" or
          stripped_port_tokens.front().lexeme == "inout" or
          stripped_port_tokens.front().lexeme == "ref" or
          stripped_port_tokens.front().lexeme == "interface"};
      not has_explicit_direction and not previous_direction.has_value()) {
    return std::nullopt;
  }

  return ParsePortDeclaration(port_tokens, previous_direction);
}

auto ParseModulePort(tokens_t const port_tokens) -> ModulePort {
  ModulePort port{};
  port.tokens = port_tokens;

  auto const stripped_port_tokens{StripLeadingAttributes(port_tokens)};
  if (rng::empty(stripped_port_tokens)) {
    port.kind = ModulePortKind::kEmpty;
    return port;
  }

  if (stripped_port_tokens.front().type == TokenType::kDot) {
    port.kind = ModulePortKind::kExplicitNamed;
    auto const name_iterator{
        rng::find_if(stripped_port_tokens, [](Token const& token) -> bool {
          return token.type == TokenType::kIdentifier;
        })};
    if (name_iterator != rng::cend(stripped_port_tokens)) {
      port.name = name_iterator->lexeme;
    }
    return port;
  }

  port.kind = ModulePortKind::kImplicit;
  auto const name_iterator{
      rng::find_if(stripped_port_tokens | rng::views::reverse,
                   [](Token const& token) -> bool {
                     return token.type == TokenType::kIdentifier;
                   })};
  if (name_iterator != rng::crend(stripped_port_tokens)) {
    port.name = name_iterator->lexeme;
  }
  return port;
}

inline auto PreviousPortDirection(std::vector<ModulePort> const& ports)
    -> std::optional<PortDirection> {
  for (auto port_iterator{rng::crbegin(ports)};
       port_iterator != rng::crend(ports); ++port_iterator) {
    if (port_iterator->direction.has_value()) {
      return port_iterator->direction;
    }
  }

  return std::nullopt;
}

inline auto FindUndirectedImplicitPort(std::vector<ModulePort>& ports,
                                       ModulePort const& declaration)
    -> std::vector<ModulePort>::iterator {
  if (rng::empty(declaration.name)) {
    return rng::end(ports);
  }

  return rng::find_if(ports, [&declaration](ModulePort const& port) -> bool {
    return port.kind == ModulePortKind::kImplicit and
           port.name == declaration.name and not port.direction.has_value();
  });
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

auto GenerateRegionBodyTokens(GenerateItem const& item) -> tokens_t {
  if (rng::size(item.tokens) < 2UZ) {
    return {};
  }
  return {rng::next(rng::cbegin(item.tokens), 1, rng::cend(item.tokens)),
          rng::prev(rng::cend(item.tokens))};
}

auto ToString(PortDirection const direction) -> std::string_view {
  switch (direction) {
    case PortDirection::kInput:
      return "input";
    case PortDirection::kOutput:
      return "output";
    case PortDirection::kInout:
      return "inout";
    case PortDirection::kRef:
      return "ref";
  }

  std::unreachable();
}

auto ToString(ModulePortKind const kind) -> std::string_view {
  switch (kind) {
    case ModulePortKind::kImplicit:
      return "implicit";
    case ModulePortKind::kExplicitNamed:
      return "explicit named";
    case ModulePortKind::kEmpty:
      return "empty";
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

auto ToString(VariableType const type) -> std::string_view {
  switch (type) {
    case VariableType::kReg:
      return "reg";
    case VariableType::kInt:
      return "int";
    case VariableType::kInteger:
      return "integer";
    case VariableType::kShortint:
      return "shortint";
    case VariableType::kLongint:
      return "longint";
    case VariableType::kByte:
      return "byte";
    case VariableType::kBit:
      return "bit";
    case VariableType::kReal:
      return "real";
    case VariableType::kTime:
      return "time";
    case VariableType::kShortreal:
      return "shortreal";
    case VariableType::kChandle:
      return "chandle";
    case VariableType::kRealtime:
      return "realtime";
    case VariableType::kEvent:
      return "event";
    case VariableType::kString:
      return "string";
  }
  std::unreachable();
}

auto ToString(TimeDeclarationKind const kind) -> std::string_view {
  switch (kind) {
    case TimeDeclarationKind::kTimeUnit:
      return "timeunit";
    case TimeDeclarationKind::kTimePrecision:
      return "timeprecision";
  }

  std::unreachable();
}

auto PrintTimeDeclaration(TimeDeclaration const& declaration,
                          std::string_view const indent = {}) -> void {
  if (rng::empty(declaration.precision_value)) {
    fmt::println("{}{} {}", indent, ToString(declaration.kind),
                 JoinLexemes(declaration.time_value));
  } else {
    fmt::println("{}{} {} / {}", indent, ToString(declaration.kind),
                 JoinLexemes(declaration.time_value),
                 JoinLexemes(declaration.precision_value));
  }
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

auto JoinPackageScopeNames(std::vector<PackageScopeName> const& names)
    -> std::string {
  return names | rng::views::transform([](PackageScopeName const& name) {
           return JoinLexemes(name.tokens);
         }) |
         rng::views::join_with(std::string_view{", "}) | rng::to<std::string>();
}

auto PrintImportDeclaration(ImportDeclaration const& declaration,
                            std::string_view const indent = {}) -> void {
  fmt::println("{}import {}", indent, JoinPackageScopeNames(declaration.names));
}

auto PrintExportDeclaration(ExportDeclaration const& declaration,
                            std::string_view const indent = {}) -> void {
  fmt::println("{}export {}", indent, JoinPackageScopeNames(declaration.names));
}

auto PrintPackage(PackageDeclaration const& package_declaration) -> void {
  fmt::println("package {}", package_declaration.name);

  if (not rng::empty(package_declaration.items)) {
    fmt::println("  items:");
    for (auto const& item : package_declaration.items) {
      std::visit(
          [](auto const& resolved_item) -> void {
            if constexpr (std::same_as<
                              std::remove_cvref_t<decltype(resolved_item)>,
                              TimeDeclaration>) {
              PrintTimeDeclaration(resolved_item, "    ");
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              ParameterDeclaration>) {
              PrintParameter(resolved_item);
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              PackageImportDeclaration>) {
              PrintImportDeclaration(resolved_item, "    ");
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              PackageExportDeclaration>) {
              PrintExportDeclaration(resolved_item, "    ");
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              UnsupportedPackageItem>) {
              fmt::println("    {} <unsupported>", resolved_item.kind);
            }
          },
          item);
    }
  }
}

auto PrintModule(ModuleDeclaration const& module_declaration) -> void {
  if (rng::empty(module_declaration.lifetime)) {
    fmt::println("module {}", module_declaration.name);
  } else {
    fmt::println("module {} {}", module_declaration.lifetime,
                 module_declaration.name);
  }

  if (not rng::empty(module_declaration.parameters)) {
    fmt::println("  parameters:");
    for (auto const& parameter : module_declaration.parameters) {
      PrintParameter(parameter);
    }
  }

  if (not rng::empty(module_declaration.imports)) {
    fmt::println("  imports:");
    for (auto const& import_declaration : module_declaration.imports) {
      PrintImportDeclaration(import_declaration, "    ");
    }
  }

  if (not rng::empty(module_declaration.ports)) {
    fmt::println("  ports:");
    for (auto const& port : module_declaration.ports) {
      auto line{fmt::format("    {}", ToString(port.kind))};
      if (not rng::empty(port.attributes)) {
        line += fmt::format(" {}", JoinLexemes(port.attributes));
      }
      if (port.direction.has_value()) {
        line += fmt::format(" {}", ToString(port.direction.value()));
      }
      if (not rng::empty(port.interface_type)) {
        line += fmt::format(" {}", JoinLexemes(port.interface_type));
      }
      if (not rng::empty(port.type_specifier)) {
        line += fmt::format(" {}", JoinLexemes(port.type_specifier));
      }
      if (not rng::empty(port.name)) {
        line += fmt::format(" {}", port.name);
      }
      if (not rng::empty(port.unpacked_dimensions)) {
        line += fmt::format(" {}", JoinLexemes(port.unpacked_dimensions));
      }
      if (not rng::empty(port.default_value)) {
        line += fmt::format(" = {}", JoinLexemes(port.default_value));
      }
      fmt::println("{}", line);
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
                                              VariableDeclaration>) {
              for (auto const& declarator : resolved_item.declarators) {
                auto line{fmt::format("    {}", ToString(resolved_item.type))};
                if (not rng::empty(resolved_item.type_specifier)) {
                  line += fmt::format(
                      " {}", JoinLexemes(resolved_item.type_specifier));
                }
                line += fmt::format(" {}", declarator.name);
                if (not rng::empty(declarator.unpacked_dimensions)) {
                  line += fmt::format(
                      " {}", JoinLexemes(declarator.unpacked_dimensions));
                }
                if (not rng::empty(declarator.initializer)) {
                  line +=
                      fmt::format(" = {}", JoinLexemes(declarator.initializer));
                }
                fmt::println("{}", line);
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
              fmt::println("    always {}",
                           JoinLexemes(resolved_item.statement->tokens));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              InitialBlock>) {
              fmt::println("    initial {}",
                           JoinLexemes(resolved_item.statement->tokens));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              FinalBlock>) {
              fmt::println("    final {}",
                           JoinLexemes(resolved_item.statement->tokens));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              GenerateItem>) {
              std::visit(
                  [&resolved_item](auto const& generate_item) -> void {
                    if constexpr (std::same_as<std::remove_cvref_t<
                                                   decltype(generate_item)>,
                                               GenerateRegion>) {
                      fmt::println(
                          "    generate {}",
                          JoinLexemes(GenerateRegionBodyTokens(resolved_item)));
                    } else if constexpr (std::same_as<
                                             std::remove_cvref_t<
                                                 decltype(generate_item)>,
                                             UnsupportedGenerateItem>) {
                      fmt::println("    {} <unsupported>", generate_item.kind);
                    } else {
                      fmt::println("    generate item {}",
                                   JoinLexemes(resolved_item.tokens));
                    }
                  },
                  resolved_item.node);
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              ModuleInstantiation>) {
              fmt::println("    {} {} #({}) ({})", resolved_item.module_name,
                           resolved_item.instance_name,
                           JoinLexemes(resolved_item.parameter_overrides),
                           JoinLexemes(resolved_item.port_connections));
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              TimeDeclaration>) {
              PrintTimeDeclaration(resolved_item, "    ");
            } else if constexpr (std::same_as<std::remove_cvref_t<
                                                  decltype(resolved_item)>,
                                              ImportDeclaration>) {
              PrintImportDeclaration(resolved_item, "    ");
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

TokenParserBase::TokenParserBase(tokens_t const tokens, bool const allow_empty)
    : m_tokens{tokens}, m_token_iterator{rng::cbegin(m_tokens)} {
  if (not allow_empty and rng::empty(m_tokens)) {
    throw std::runtime_error{"[Parser] expected expression"};
  }
}

auto TokenParserBase::AtEnd() const -> bool {
  return m_token_iterator == rng::cend(m_tokens) or
         m_token_iterator->type == TokenType::kEndOfFile;
}

auto TokenParserBase::CurrentTokenIterator() const -> tokens_t::iterator {
  return m_token_iterator;
}

auto TokenParserBase::MatchToken(TokenType const token_type) -> bool {
  if (not AtEnd() and m_token_iterator->type == token_type) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    return true;
  }

  return false;
}

auto TokenParserBase::MatchKeyword(std::string_view const keyword) -> bool {
  if (not AtEnd() and m_token_iterator->IsKeyword(keyword)) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    return true;
  }

  return false;
}

auto TokenParserBase::ExpectToken(TokenType const expected_type,
                                  std::string_view const context) -> void {
  if (m_token_iterator->type != expected_type) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] unexpected token '{}' while parsing {} at ({}, {})",
        m_token_iterator->lexeme, context, m_token_iterator->location.row,
        m_token_iterator->location.column)};
  }

  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
}

auto TokenParserBase::ExpectKeyword(std::string_view const lexeme,
                                    std::string_view const context) -> void {
  if (not m_token_iterator->IsKeyword(lexeme)) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected '{}' while parsing {} at ({}, {})", lexeme, context,
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
}

auto Parser::ParseVariableDeclaration() -> VariableDeclaration {
  auto const declaration_begin_iterator{m_token_iterator};
  auto const declaration_end_iterator{
      rng::find_if(std::span{m_token_iterator, rng::cend(m_tokens)},
                   [](Token const& token) -> bool {
                     return token.type == TokenType::kSemicolon or
                            token.type == TokenType::kEndOfFile;
                   })};
  if (declaration_end_iterator == rng::cend(m_tokens) or
      declaration_end_iterator->type == TokenType::kEndOfFile) {
    throw std::runtime_error{
        "[Parser] expected ';' while parsing variable declaration"};
  }

  auto const declaration_tokens{
      tokens_t{declaration_begin_iterator, declaration_end_iterator}};
  auto const stripped_tokens{StripLeadingAttributes(declaration_tokens)};
  if (rng::empty(stripped_tokens)) {
    throw std::runtime_error{"[Parser] expected variable type"};
  }

  auto const type{FindVariableType(stripped_tokens.front().lexeme)};
  if (not type.has_value()) {
    throw std::runtime_error{"[Parser] expected variable type"};
  }
  auto type_begin_iterator{
      rng::next(rng::cbegin(stripped_tokens), 1, rng::cend(stripped_tokens))};
  auto packed_end_iterator{type_begin_iterator};
  while (packed_end_iterator != rng::cend(stripped_tokens) and
         packed_end_iterator->type == TokenType::kLBracket) {
    auto const close_iterator{
        FindMatchingDelimiter(packed_end_iterator, rng::cend(stripped_tokens),
                              TokenType::kLBracket, TokenType::kRBracket)};
    if (close_iterator == rng::cend(stripped_tokens)) {
      throw std::runtime_error{
          "[Parser] expected ']' while parsing variable declaration"};
    }
    packed_end_iterator =
        rng::next(close_iterator, 1, rng::cend(stripped_tokens));
  }

  auto const type_specifier{tokens_t{type_begin_iterator, packed_end_iterator}};
  auto const declarator_tokens{
      tokens_t{packed_end_iterator, rng::cend(stripped_tokens)}};
  auto const declarators{SplitTopLevelSeparatedTokens(
      declarator_tokens, TokenType::kComma, "variable declaration", false)};
  if (rng::empty(declarators)) {
    throw std::runtime_error{"[Parser] expected variable declarator"};
  }

  VariableDeclaration declaration{};
  declaration.type = type.value();
  declaration.attributes =
      tokens_t{rng::cbegin(declaration_tokens), rng::cbegin(stripped_tokens)};
  declaration.type_specifier = type_specifier;
  declaration.packed_dimensions = ParsePackedDimensions(type_specifier);

  for (auto const declarator_tokens : declarators) {
    if (rng::empty(declarator_tokens)) {
      throw std::runtime_error{"[Parser] expected variable declarator"};
    }

    auto const equals_iterator{FindTopLevelAssignmentOperator(
        rng::cbegin(declarator_tokens), rng::cend(declarator_tokens))};
    auto const declarator_end_iterator{equals_iterator ==
                                               rng::cend(declarator_tokens)
                                           ? rng::cend(declarator_tokens)
                                           : equals_iterator};
    auto const name_iterator{
        rng::find_if(rng::cbegin(declarator_tokens), declarator_end_iterator,
                     [](Token const& token) -> bool {
                       return token.type == TokenType::kIdentifier;
                     })};
    if (name_iterator == declarator_end_iterator) {
      throw std::runtime_error{"[Parser] expected variable name"};
    }

    VariableDeclarator declarator{};
    declarator.name = name_iterator->lexeme;
    declarator.unpacked_dimensions =
        tokens_t{rng::next(name_iterator, 1, declarator_end_iterator),
                 declarator_end_iterator};
    if (equals_iterator != rng::cend(declarator_tokens)) {
      declarator.initializer =
          tokens_t{rng::next(equals_iterator, 1, rng::cend(declarator_tokens)),
                   rng::cend(declarator_tokens)};
    }
    declarator.tokens = declarator_tokens;
    declaration.declarators.push_back(declarator);
  }

  m_token_iterator = declaration_end_iterator;
  ExpectToken(TokenType::kSemicolon, "variable declaration");
  declaration.tokens = tokens_t{declaration_begin_iterator, m_token_iterator};
  return declaration;
}

auto TokenParserBase::ParseNetDeclaration() -> NetDeclaration {
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

  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

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
      ::ParsePackedDimensions(net_declaration.type_specifier);

  m_token_iterator = declaration_end_iterator;
  ExpectToken(TokenType::kSemicolon, "net declaration");

  return net_declaration;
}

auto TokenParserBase::ParseContinuousAssign() -> ContinuousAssign {
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

auto TokenParserBase::ParseModuleInstantiation() -> ModuleInstantiation {
  auto const module_name_token{*m_token_iterator};
  ExpectToken(TokenType::kIdentifier, "module instantiation");

  tokens_t parameter_overrides{};
  if (m_token_iterator->type == TokenType::kHash) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    parameter_overrides = ::ConsumeBalancedDelimitedTokens(
        m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
        TokenType::kRParen, "module instantiation parameter override");
    m_token_iterator =
        rng::next(parameter_overrides.end(), 1, rng::cend(m_tokens));
  }

  auto const instance_name_token{*m_token_iterator};
  ExpectToken(TokenType::kIdentifier, "module instantiation instance name");
  auto const port_connections = ::ConsumeBalancedDelimitedTokens(
      m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
      TokenType::kRParen, "module instantiation port connections");
  m_token_iterator = rng::next(port_connections.end(), 1, rng::cend(m_tokens));
  ExpectToken(TokenType::kSemicolon, "module instantiation");

  return ModuleInstantiation{.module_name = module_name_token.lexeme,
                             .instance_name = instance_name_token.lexeme,
                             .parameter_overrides = parameter_overrides,
                             .port_connections = port_connections};
}

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

    while (::IsBasedLiteralDigit(Peek())) {
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
      "alias",         "always",      "always_comb",  "always_ff",
      "always_latch",  "assign",      "assume",       "automatic",
      "begin",         "bind",        "bit",          "case",
      "casex",         "casez",       "chandle",      "checker",
      "class",         "clocking",    "config",       "constraint",
      "cover",         "covergroup",  "deassign",     "default",
      "defparam",      "disable",     "else",         "end",
      "endchecker",    "endclass",    "endclocking",  "endconfig",
      "endfunction",   "endgenerate", "endgroup",     "endinterface",
      "endmodule",     "endpackage",  "endprimitive", "endprogram",
      "endproperty",   "endsequence", "endcase",      "endspecify",
      "endtask",       "event",       "export",       "extern",
      "final",         "for",         "force",        "foreach",
      "forever",       "fork",        "function",     "generate",
      "genvar",        "global",      "if",           "import",
      "initial",       "input",       "int",          "integer",
      "interface",     "inout",       "join",         "join_any",
      "join_none",     "let",         "localparam",   "logic",
      "longint",       "macromodule", "module",       "nettype",
      "output",        "package",     "parameter",    "primitive",
      "program",       "property",    "real",         "realtime",
      "ref",           "reg",         "release",      "repeat",
      "restrict",      "sequence",    "shortint",     "shortreal",
      "specify",       "static",      "task",         "time",
      "timeprecision", "timeunit",    "typedef",      "wait",
      "wait_order",    "wand",        "while",        "wire",
      "wor",
  })};

  auto const start_position{m_position - 1};

  while (::IsIdentifierBodyCharacter(Peek())) {
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

  while (::IsIdentifierBodyCharacter(Peek())) {
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

    if (::IsHorizontalWhiteSpace(next_character)) {
      SkipHorizontalWhiteSpace();
      continue;
    }

    if (::IsLineBreak(next_character)) {
      SkipLineBreak();
      continue;
    }

    if (::StartsLineComment(next_character, Peek<1>())) {
      SkipLineComment();
      continue;
    }

    if (::StartsBlockComment(next_character, Peek<1>())) {
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
    : m_lexer{std::move(sv_source_code)} {
  m_tokens = m_lexer.Tokens();
  m_token_iterator = rng::cbegin(m_tokens);
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
            ::PrintModule(resolved_node);
          } else if constexpr (std::same_as<
                                   std::remove_cvref_t<decltype(resolved_node)>,
                                   PackageDeclaration>) {
            ::PrintPackage(resolved_node);
          } else if constexpr (std::same_as<
                                   std::remove_cvref_t<decltype(resolved_node)>,
                                   TimeDeclaration>) {
            PrintTimeDeclaration(resolved_node);
          } else if constexpr (std::same_as<
                                   std::remove_cvref_t<decltype(resolved_node)>,
                                   ImportDeclaration>) {
            PrintImportDeclaration(resolved_node);
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
  auto dispatch_iterator{m_token_iterator};
  ::AdvancePastAttributeInstances(dispatch_iterator, rng::cend(m_tokens));

  if (dispatch_iterator->type == TokenType::kKeyword) {
    switch (::HashLexeme(dispatch_iterator->lexeme)) {
      case ::HashLexeme("module"):
        m_token_iterator = dispatch_iterator;
        rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
        return ParseModuleDeclaration();
      case ::HashLexeme("package"):
        return ParsePackageDeclaration();
      case ::HashLexeme("timeunit"):
      case ::HashLexeme("timeprecision"):
        m_token_iterator = dispatch_iterator;
        return ParseTimeDeclaration();
      case ::HashLexeme("import"):
        if (::IsPackageScopeImportStart(dispatch_iterator,
                                        rng::cend(m_tokens))) {
          m_token_iterator = dispatch_iterator;
          return ParseImportDeclaration();
        }
        break;
      default:
        break;
    }
  }

  return ParseUnsupportedDesignElement();
}

auto Parser::ParseTimeDeclaration() -> TimeDeclaration {
  auto const declaration_begin_iterator{m_token_iterator};
  auto const kind{m_token_iterator->lexeme == "timeunit"
                      ? TimeDeclarationKind::kTimeUnit
                      : TimeDeclarationKind::kTimePrecision};

  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  auto const time_begin_iterator{m_token_iterator};

  auto semicolon_iterator{
      rng::find_if(tokens_t{m_token_iterator, rng::cend(m_tokens)},
                   [](Token const& token) -> bool {
                     return token.type == TokenType::kSemicolon;
                   })};
  if (semicolon_iterator == rng::cend(m_tokens)) {
    throw std::runtime_error{"[Parser] expected time declaration"};
  }

  auto slash_iterator{rng::find_if(
      tokens_t{time_begin_iterator, semicolon_iterator},
      [](Token const& token) -> bool { return token.lexeme == "/"; })};
  if (time_begin_iterator == slash_iterator) {
    throw std::runtime_error{"[Parser] expected time value"};
  }

  tokens_t precision_value{};
  if (slash_iterator != semicolon_iterator) {
    auto const precision_begin_iterator{
        rng::next(slash_iterator, 1, rng::cend(m_tokens))};
    if (precision_begin_iterator == semicolon_iterator) {
      throw std::runtime_error{"[Parser] expected precision value"};
    }
    precision_value = {precision_begin_iterator, semicolon_iterator};
  }

  m_token_iterator = rng::next(semicolon_iterator, 1, rng::cend(m_tokens));

  return TimeDeclaration{
      .kind = kind,
      .time_value = slash_iterator != semicolon_iterator
                        ? tokens_t{time_begin_iterator, slash_iterator}
                        : tokens_t{time_begin_iterator, semicolon_iterator},
      .precision_value = precision_value,
      .tokens = {declaration_begin_iterator, m_token_iterator}};
}

auto Parser::ParseImportDeclaration() -> ImportDeclaration {
  auto const import_begin_iterator{m_token_iterator};
  ExpectKeyword("import", "import declaration");

  ImportDeclaration declaration{};
  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->type != TokenType::kSemicolon) {
    auto const name_begin_iterator{m_token_iterator};
    ::AdvanceToTopLevelBoundary(
        m_token_iterator, rng::cend(m_tokens),
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kSemicolon;
        },
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kComma or
                 token_iterator->type == TokenType::kSemicolon;
        },
        false, BoundaryEndBehavior::kThrow, "import declaration");

    declaration.names.push_back(::ParsePackageScopeName(
        tokens_t{name_begin_iterator, m_token_iterator}, "import declaration"));

    if (m_token_iterator->type == TokenType::kComma) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }
  }

  ExpectToken(TokenType::kSemicolon, "import declaration");
  declaration.tokens = {import_begin_iterator, m_token_iterator};
  return declaration;
}

auto Parser::ParseExportDeclaration() -> ExportDeclaration {
  auto const export_begin_iterator{m_token_iterator};
  ExpectKeyword("export", "export declaration");

  ExportDeclaration declaration{};
  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->type != TokenType::kSemicolon) {
    auto const name_begin_iterator{m_token_iterator};
    ::AdvanceToTopLevelBoundary(
        m_token_iterator, rng::cend(m_tokens),
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kSemicolon;
        },
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kComma or
                 token_iterator->type == TokenType::kSemicolon;
        },
        false, BoundaryEndBehavior::kThrow, "export declaration");

    declaration.names.push_back(::ParsePackageScopeName(
        tokens_t{name_begin_iterator, m_token_iterator}, "export declaration"));

    if (m_token_iterator->type == TokenType::kComma) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }
  }

  ExpectToken(TokenType::kSemicolon, "export declaration");
  declaration.tokens = {export_begin_iterator, m_token_iterator};
  return declaration;
}

auto Parser::ParsePackageDeclaration() -> PackageDeclaration {
  auto const package_begin_iterator{m_token_iterator};
  ::AdvancePastAttributeInstances(m_token_iterator, rng::cend(m_tokens));
  ExpectKeyword("package", "package declaration");

  PackageDeclaration package_declaration{};

  if (m_token_iterator->IsKeyword("automatic") or
      m_token_iterator->IsKeyword("static")) {
    package_declaration.lifetime = m_token_iterator->lexeme;
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }

  if (m_token_iterator->type != TokenType::kIdentifier) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected package name at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  package_declaration.name = m_token_iterator->lexeme;
  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  ExpectToken(TokenType::kSemicolon, "package declaration");

  package_declaration.items = ParsePackageItems();

  ExpectKeyword("endpackage", "package declaration");

  package_declaration.tokens = {package_begin_iterator, m_token_iterator};
  return package_declaration;
}

auto Parser::ParsePackageItems() -> std::vector<PackageItem> {
  std::vector<PackageItem> items{};

  while (m_token_iterator->type != TokenType::kEndOfFile and
         not m_token_iterator->IsKeyword("endpackage")) {
    if (m_token_iterator->lexeme == "timeunit" or
        m_token_iterator->lexeme == "timeprecision") {
      items.emplace_back(std::in_place_type<TimeDeclaration>,
                         ParseTimeDeclaration());
      continue;
    }

    if (m_token_iterator->IsKeyword("parameter") or
        m_token_iterator->IsKeyword("localparam")) {
      rng::transform(ParseParameters(TokenType::kSemicolon,
                                     "package parameter declaration"),
                     std::back_inserter(items),
                     [](ParameterDeclaration& parameter) -> PackageItem {
                       return PackageItem{
                           std::in_place_type<ParameterDeclaration>,
                           std::move(parameter)};
                     });
      continue;
    }

    if (::IsPackageScopeImportStart(m_token_iterator, rng::cend(m_tokens))) {
      items.emplace_back(std::in_place_type<ImportDeclaration>,
                         ParseImportDeclaration());
      continue;
    }

    if (::IsPackageScopeExportStart(m_token_iterator, rng::cend(m_tokens))) {
      items.emplace_back(std::in_place_type<ExportDeclaration>,
                         ParseExportDeclaration());
      continue;
    }

    items.emplace_back(std::in_place_type<UnsupportedPackageItem>,
                       ParseUnsupportedPackageItem());
  }

  return items;
}

auto Parser::ParseUnsupportedPackageItem() -> UnsupportedPackageItem {
  auto const package_item_begin_iterator{m_token_iterator};

  std::string_view end_keyword{};
  try {
    end_keyword = ::MatchingModuleItemEndKeyword(m_token_iterator);
  } catch (std::runtime_error const&) {
  }

  if (not rng::empty(end_keyword)) {
    SkipUnsupportedElementToMatchingEnd(
        m_token_iterator->lexeme, end_keyword,
        {.match_keyword_tokens_only = false,
         .require_end_keyword = true,
         .trailing_label_must_be_identifier = false,
         .context = "unsupported package item"});
  } else {
    SkipUnsupportedElementToSemicolon("endpackage");
  }

  return UnsupportedPackageItem{
      .kind = package_item_begin_iterator->lexeme,
      .tokens = {package_item_begin_iterator, m_token_iterator}};
}

auto Parser::ParseUnsupportedDesignElement() -> UnsupportedDesignElement {
  auto const element_begin_iterator{m_token_iterator};
  ::AdvancePastAttributeInstances(m_token_iterator, rng::cend(m_tokens));

  if (m_token_iterator->type == TokenType::kKeyword) {
    try {
      SkipUnsupportedElementToMatchingEnd(
          m_token_iterator->lexeme,
          FindKeywordEnd<true>(m_token_iterator->lexeme), {});
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
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }

  return UnsupportedDesignElement{
      .kind = element_begin_iterator->lexeme,
      .tokens = std::span{element_begin_iterator, m_token_iterator}};
}

auto Parser::SkipUnsupportedElementToSemicolon(
    std::string_view const stop_keyword) -> void {
  ::AdvanceToTopLevelBoundary(
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
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }
}

auto Parser::SkipUnsupportedElementToMatchingEnd(
    std::string_view const start_keyword, std::string_view const end_keyword,
    UnsupportedElementEndOptions const& options) -> void {
  auto matches_keyword{[&options](tokens_t::iterator const token_iterator,
                                  std::string_view const keyword) -> bool {
    if (options.match_keyword_tokens_only) {
      return token_iterator->IsKeyword(keyword);
    }

    return token_iterator->lexeme == keyword;
  }};

  ::AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                                start_keyword, end_keyword, 0UZ,
                                matches_keyword);
  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  if (m_token_iterator->type == TokenType::kColon) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    if ((options.trailing_label_must_be_identifier and
         m_token_iterator->type == TokenType::kIdentifier) or
        (not options.trailing_label_must_be_identifier and
         m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->type != TokenType::kSemicolon)) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }
  }
}

auto Parser::ParseModuleDeclaration() -> ModuleDeclaration {
  ModuleDeclaration module_declaration{};

  if (m_token_iterator->IsKeyword("automatic") or
      m_token_iterator->IsKeyword("static")) {
    module_declaration.lifetime = m_token_iterator->lexeme;
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }

  if (m_token_iterator->type != TokenType::kIdentifier) [[unlikely]] {
    throw std::runtime_error{fmt::format(
        "[Parser] expected module name at ({}, {})",
        m_token_iterator->location.row, m_token_iterator->location.column)};
  }

  module_declaration.name = m_token_iterator->lexeme;
  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));

  while (::IsPackageScopeImportStart(m_token_iterator, rng::cend(m_tokens))) {
    module_declaration.imports.push_back(ParseImportDeclaration());
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
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    module_declaration.parameters = ParseParameters();
  }

  if (m_token_iterator->type == TokenType::kLParen) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    module_declaration.ports = ParsePorts();
  }

  if (m_token_iterator->type == TokenType::kSemicolon) {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }

  // now parsing module items
  while (m_token_iterator->type != TokenType::kEndOfFile and
         m_token_iterator->lexeme != "endmodule") {
    if (m_token_iterator->IsKeyword("input") or
        m_token_iterator->IsKeyword("output") or
        m_token_iterator->IsKeyword("inout") or
        m_token_iterator->IsKeyword("ref")) {
      for (auto const& port_declaration : ParseModulePortDeclarations()) {
        auto const matching_header_port_iterator{::FindUndirectedImplicitPort(
            module_declaration.ports, port_declaration)};
        if (matching_header_port_iterator !=
            rng::end(module_declaration.ports)) {
          matching_header_port_iterator->direction = port_declaration.direction;
          continue;
        }

        module_declaration.ports.push_back(port_declaration);
      }

      continue;
    }

    module_declaration.items.push_back(ParseModuleItem());
  }

  if (m_token_iterator->lexeme == "endmodule") {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }
  // TODO(): if not shouldn't we throw here i.e. why dont we have
  // ExpectKeyword for endmodule before?

  return module_declaration;
}

auto Parser::ParseModulePortDeclarations() -> std::vector<ModulePort> {
  std::vector<ModulePort> ports{};

  while (m_token_iterator->type != TokenType::kSemicolon) {
    auto const port_begin{m_token_iterator};

    ::AdvanceToTopLevelBoundary(
        m_token_iterator, rng::cend(m_tokens),
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kSemicolon;
        },
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->type == TokenType::kComma or
                 token_iterator->type == TokenType::kSemicolon;
        },
        true, BoundaryEndBehavior::kThrow, "module port declaration");

    auto const port_tokens{tokens_t{port_begin, m_token_iterator}};
    auto const previous_direction{::PreviousPortDirection(ports)};
    ports.push_back(::ParsePortDeclaration(port_tokens, previous_direction));

    if (m_token_iterator->type == TokenType::kComma) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }
  }

  ExpectToken(TokenType::kSemicolon, "module port declaration");

  return ports;
}

auto Parser::ParseModuleItem() -> ModuleItem {
  auto const parse_procedural_statement{
      [this](tokens_t const statement_tokens) -> StatementPtr {
        return StatementParser{statement_tokens}.Parse();
      }};

  auto const parse_procedural_body{
      [this, &parse_procedural_statement](
          std::string_view const context) -> StatementPtr {
        auto const statement_begin_iterator{m_token_iterator};
        switch (::HashLexeme(m_token_iterator->lexeme)) {
          case ::HashLexeme("begin"):
            ParseKeywordBlockBody("begin", "end", context);
            break;
          default:
            ParseSingleStatementBody(context);
            break;
        }
        return parse_procedural_statement(
            tokens_t{statement_begin_iterator, m_token_iterator});
      }};

  auto const parse_initial_block{[this,
                                  &parse_procedural_body]() -> ModuleItem {
    ExpectToken(TokenType::kKeyword, "initial block");
    auto statement{parse_procedural_body("initial block")};
    return ModuleItem{std::in_place_type<InitialBlock>, std::move(statement)};
  }};

  auto const parse_final_block{[this, &parse_procedural_body]() -> ModuleItem {
    ExpectToken(TokenType::kKeyword, "final block");
    auto statement{parse_procedural_body("final block")};
    return ModuleItem{std::in_place_type<FinalBlock>, std::move(statement)};
  }};

  auto const parse_always_block{
      [this, &parse_procedural_statement](
          ProceduralBlockKind const kind) -> ModuleItem {
        ExpectToken(TokenType::kKeyword, "always block");
        auto const statement_begin_iterator{m_token_iterator};
        ParseAlwaysEventControl();
        switch (::HashLexeme(m_token_iterator->lexeme)) {
          case ::HashLexeme("begin"):
            ParseKeywordBlockBody("begin", "end", "always block");
            break;
          default:
            ParseSingleStatementBody("always block");
            break;
        }
        auto statement{parse_procedural_statement(
            tokens_t{statement_begin_iterator, m_token_iterator})};
        return ModuleItem{std::in_place_type<AlwaysBlock>, kind,
                          std::move(statement)};
      }};

  auto const parse_generate_item{[this]() -> ModuleItem {
    GenerateItemParser parser{{m_token_iterator, rng::cend(m_tokens)}};
    auto result{
        ModuleItem{std::in_place_type<GenerateItem>, parser.ParseOne()}};
    m_token_iterator = parser.CurrentTokenIterator();
    return result;
  }};

  auto const declaration_start{
      StripLeadingAttributes(tokens_t{m_token_iterator, rng::cend(m_tokens)})};
  if (not rng::empty(declaration_start) and
      FindVariableType(declaration_start.front().lexeme).has_value()) {
    return ModuleItem{std::in_place_type<VariableDeclaration>,
                      ParseVariableDeclaration()};
  }

  switch (m_token_iterator->type) {
    case TokenType::kKeyword: {
      switch (::HashLexeme(m_token_iterator->lexeme)) {
        case ::HashLexeme("timeunit"):
        case ::HashLexeme("timeprecision"):
          return ModuleItem{std::in_place_type<TimeDeclaration>,
                            ParseTimeDeclaration()};
        case ::HashLexeme("wire"):
        case ::HashLexeme("logic"):
          return ModuleItem{std::in_place_type<NetDeclaration>,
                            ParseNetDeclaration()};
        case ::HashLexeme("assign"):
          return ModuleItem{std::in_place_type<ContinuousAssign>,
                            ParseContinuousAssign()};
        case ::HashLexeme("import"):
          if (::IsPackageScopeImportStart(m_token_iterator,
                                          rng::cend(m_tokens))) {
            return ModuleItem{std::in_place_type<ImportDeclaration>,
                              ParseImportDeclaration()};
          }
          break;
        case ::HashLexeme("always"):
          return parse_always_block(ProceduralBlockKind::kAlways);
        case ::HashLexeme("always_ff"):
          return parse_always_block(ProceduralBlockKind::kAlwaysFf);
        case ::HashLexeme("always_comb"):
          return parse_always_block(ProceduralBlockKind::kAlwaysComb);
        case ::HashLexeme("always_latch"):
          return parse_always_block(ProceduralBlockKind::kAlwaysLatch);
        case ::HashLexeme("initial"):
          return parse_initial_block();
        case ::HashLexeme("final"):
          return parse_final_block();
        case ::HashLexeme("genvar"):
        case ::HashLexeme("for"):
        case ::HashLexeme("if"):
        case ::HashLexeme("case"):
        case ::HashLexeme("generate"):
          return parse_generate_item();
        default:
          break;
      }
      break;
    }

    case TokenType::kIdentifier: {
      auto const module_item_begin_iterator{m_token_iterator};
      try {
        return ModuleItem{std::in_place_type<ModuleInstantiation>,
                          ParseModuleInstantiation()};
      } catch (std::runtime_error const&) {
        m_token_iterator = module_item_begin_iterator;
        return ModuleItem{std::in_place_type<UnsupportedModuleItem>,
                          ParseUnsupportedModuleItem()};
      }
    }

    default: {
      break;
    }
  }

  // TODO(): this pattern is not nice, but not doing refactor as
  // this function will anyways be needed later
  auto const matching_end_keyword{[this]() -> std::string_view {
    try {
      return ::MatchingModuleItemEndKeyword(m_token_iterator);
    } catch (std::runtime_error const&) {
      return {};
    }
  }()};

  auto const is_attribute_instance_start{
      m_token_iterator->type == TokenType::kLParen and
      rng::next(m_token_iterator, 1, rng::cend(m_tokens)) !=
          rng::cend(m_tokens) and
      rng::next(m_token_iterator, 1, rng::cend(m_tokens))->lexeme == "*"};

  if (not rng::empty(matching_end_keyword) or is_attribute_instance_start or
      rng::contains(kUnsupportedModuleItemKeywords, m_token_iterator->lexeme) or
      rng::contains(kUnsupportedModuleItemNetTypes, m_token_iterator->lexeme) or
      m_token_iterator->IsKeyword("begin")) {
    return ModuleItem{std::in_place_type<UnsupportedModuleItem>,
                      ParseUnsupportedModuleItem()};
  }

  throw std::runtime_error{
      fmt::format("[Parser] unexpected token '{}' while parsing module item",
                  m_token_iterator->lexeme)};
}

auto Parser::ParseUnsupportedModuleItem() -> UnsupportedModuleItem {
  auto const module_item_begin_iterator{m_token_iterator};

  // TODO(): this pattern is not nice, but not doing refactor as
  // this function will anyways be needed later
  std::string_view end_keyword{};
  try {
    end_keyword = ::MatchingModuleItemEndKeyword(m_token_iterator);
  } catch (std::runtime_error const&) {
  }

  if (not rng::empty(end_keyword)) {
    SkipUnsupportedElementToMatchingEnd(
        m_token_iterator->lexeme, end_keyword,
        {.match_keyword_tokens_only = false,
         .require_end_keyword = true,
         .trailing_label_must_be_identifier = false,
         .context = "unsupported module item"});
  } else if (m_token_iterator->lexeme == "final") {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    if (m_token_iterator->lexeme == "begin") {
      ParseKeywordBlockBody("begin", "end", "final block");
    } else {
      ParseSingleStatementBody("final block");
    }
  } else if (m_token_iterator->lexeme == "begin") {
    ParseKeywordBlockBody("begin", "end", "unsupported module item");
    ::AdvancePastOptionalBlockLabel(m_token_iterator, rng::cend(m_tokens));
  } else {
    SkipUnsupportedElementToSemicolon("endmodule");
  }

  return UnsupportedModuleItem{
      .kind = module_item_begin_iterator->lexeme,
      .tokens = std::span{module_item_begin_iterator, m_token_iterator}};
}

auto Parser::ParseKeywordBlockBody(std::string_view const start_keyword,
                                   std::string_view const end_keyword,
                                   std::string_view const context) -> tokens_t {
  ExpectToken(TokenType::kKeyword, context);

  auto const body_begin_iterator{m_token_iterator};
  ::AdvanceToMatchingEndKeyword(m_token_iterator, rng::cend(m_tokens),
                                start_keyword, end_keyword, 1UZ,
                                TokenMatchesKeyword);
  auto const body{tokens_t{body_begin_iterator, m_token_iterator}};
  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  return body;
}

auto Parser::ParseAlwaysEventControl() -> void {
  if (m_token_iterator->type != TokenType::kAt) {
    return;
  }

  auto const event_begin_iterator{m_token_iterator};

  rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  if (m_token_iterator->type == TokenType::kLParen) {
    auto const event_tokens{::ConsumeBalancedDelimitedTokens(
        m_token_iterator, rng::cend(m_tokens), TokenType::kLParen,
        TokenType::kRParen, "always event control")};
    m_token_iterator = rng::next(event_tokens.end(), 1, rng::cend(m_tokens));
  } else {
    rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
  }
}

auto Parser::ParseSingleStatementBody(std::string_view const context) -> void {
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

  m_token_iterator = body_end_iterator;
  ExpectToken(TokenType::kSemicolon, context);
}

auto Parser::ParseParameters(TokenType const end_token,
                             std::string_view const context)
    -> std::vector<ParameterDeclaration> {
  auto is_parameter_list_end{[this, end_token]() -> bool {
    return m_token_iterator->type == end_token;
  }};

  std::vector<ParameterDeclaration> result{};

  if (end_token == TokenType::kRParen) {
    ExpectToken(TokenType::kLParen, context);
  }

  while (not is_parameter_list_end()) {
    auto const parameter_tokens{ParseParameterTokens(end_token, context)};

    std::optional<ParameterDeclaration const&> previous_parameter{};
    if (not rng::empty(result)) {
      previous_parameter = std::cref(result.back());
    }

    result.push_back(
        ::ParseParameterDeclaration(parameter_tokens, previous_parameter));

    if (m_token_iterator->type == TokenType::kComma) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    } else if (not is_parameter_list_end()) [[unlikely]] {
      throw std::runtime_error{fmt::format(
          "[Parser] expected ',' while parsing {} at ({}, {})", context,
          m_token_iterator->location.row, m_token_iterator->location.column)};
    }
  }

  ExpectToken(end_token, context);

  return result;
}

auto Parser::ParseParameterTokens(TokenType const end_token,
                                  std::string_view const context) -> tokens_t {
  auto const parameter_begin{m_token_iterator};

  ::AdvanceToTopLevelBoundary(
      m_token_iterator, rng::cend(m_tokens),
      [end_token](tokens_t::iterator const token_iterator) -> bool {
        return token_iterator->type == end_token;
      },
      [end_token](tokens_t::iterator const token_iterator) -> bool {
        return token_iterator->type == TokenType::kComma or
               token_iterator->type == end_token;
      },
      true, BoundaryEndBehavior::kThrow, context);

  return {parameter_begin, m_token_iterator};
}

auto Parser::ParsePorts() -> std::vector<ModulePort> {
  std::vector<ModulePort> result{};

  auto const is_port_list_end{[this]() -> bool {
    return m_token_iterator->type == TokenType::kRParen;
  }};

  while (not is_port_list_end()) {
    auto const port_begin{m_token_iterator};

    ::AdvanceToTopLevelBoundary(
        m_token_iterator, rng::cend(m_tokens),
        [](tokens_t::iterator const) -> bool { return false; },
        [](tokens_t::iterator const token_iterator) -> bool {
          return token_iterator->IsListSeparator();
        },
        true, BoundaryEndBehavior::kThrow, "port list");

    // In the following code,
    // TryParsePort() succeeds when the entry can be modeled as a port with a
    // known input/ output direction.
    //
    // ```
    // module m(input int a, output logic b);
    // endmodule
    // ```
    //
    // Result comes from TryParsePort():
    //
    // implicit input a
    // implicit output b
    //
    // It also succeeds for inherited direction:
    //
    // ```
    // module m(input a, b, c);
    // endmodule
    // ```
    //
    // input a gives the previous direction, then b and c inherit it:
    //
    // implicit input a
    // implicit input b
    // implicit input c
    //
    // ParseModulePort() runs only when TryParsePort() fails, mostly for
    // non-declaration or not-yet-supported forms:
    //
    // module m(a, , .e(), ref c, interface.mod d);
    // endmodule
    //
    // Fallback results:
    //
    // implicit a
    // empty
    // explicit named e
    // implicit c
    // implicit d
    //
    // There is also an intended merge case:
    //
    // module m(a);
    //   input a;
    // endmodule
    //
    // The header a is first parsed by ParseModulePort() as implicit a. Later,
    // body input a; is parsed as a declaration and merged onto the existing
    // port, giving:
    //
    // implicit input a
    //
    // See ParseModuleItem() for this special merge case

    auto const port_tokens{tokens_t{port_begin, m_token_iterator}};
    if (auto declaration_opt{
            ::TryParsePort(port_tokens, ::PreviousPortDirection(result))};
        declaration_opt.has_value()) {
      auto const matching_header_port_iterator{
          ::FindUndirectedImplicitPort(result, declaration_opt.value())};
      if (matching_header_port_iterator != rng::end(result)) {
        matching_header_port_iterator->direction =
            declaration_opt.value().direction;
      } else {
        result.push_back(declaration_opt.value());
      }
    } else {
      result.push_back(::ParseModulePort(port_tokens));
    }

    if (m_token_iterator->IsListSeparator()) {
      rng::advance(m_token_iterator, 1, rng::cend(m_tokens));
    }
  }

  ExpectToken(TokenType::kRParen, "port list");

  return result;
}

}  // namespace svt::core
