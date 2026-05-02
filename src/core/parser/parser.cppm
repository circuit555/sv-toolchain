// SPDX-License-Identifier: MIT

export module svt.core.parser;

import std;
import svt.model.token;
import svt.model.ast;

namespace svt::core {

/// @brief Lexical analyzer for SystemVerilog source text.
export class Lexer final {
 public:
  explicit Lexer(std::string&& sv_source_code);
  [[nodiscard]] auto Tokens() const -> std::span<::svt::model::Token const>;

 private:
  template <std::size_t kOffset = 0>
  [[nodiscard]] auto Peek() const -> unsigned char;
  auto ScanNext() -> ::svt::model::Token;
  auto SkipWhiteSpaceAndComments() -> void;
  auto SkipHorizontalWhiteSpace() -> void;
  auto SkipLineBreak() -> void;
  auto SkipLineComment() -> void;
  auto SkipBlockComment() -> void;
  auto ScanString(::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;
  auto ScanIdentifierOrKeyword(
      ::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;
  auto ScanSystemIdentifier(
      ::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;
  auto ScanEscapedIdentifier(
      ::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;
  auto ScanApostropheToken(
      ::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;
  auto ScanNumber(::svt::model::SourceLocation const& token_source_location)
      -> ::svt::model::Token;

  std::string m_sv_source_code;
  std::string_view m_sv_source_code_view;
  std::size_t m_position{0};
  ::svt::model::SourceLocation m_source_location{};
  ::svt::model::token_stream_t m_tokens;
};

/// @brief Recursive-descent parser for SystemVerilog source text.
///
/// The parser tokenizes input internally and produces an AST translation unit.
export class Parser final {
 public:
  using TranslationUnit = std::vector<::svt::model::DesignElement>;
  using tokens_t = std::span<::svt::model::Token const>;

  /// @brief Construct a parser over a source-code view.
  /// @param sv_source_code SystemVerilog source to parse.
  explicit Parser(std::string&& sv_source_code);
  /// @brief Parse the input and produce a translation unit AST.
  /// @return Parsed translation unit.
  [[nodiscard]] auto Parse() -> TranslationUnit;

 private:
  struct UnsupportedElementEndOptions {
    bool match_keyword_tokens_only{true};
    bool require_end_keyword{false};
    bool trailing_label_must_be_identifier{true};
    std::string_view context;
  };

  auto ExpectToken(::svt::model::TokenType expected_type,
                   std::string_view context) -> void;
  auto ParseDesignElement() -> ::svt::model::DesignElement;
  auto ParseUnsupportedDesignElement()
      -> ::svt::model::UnsupportedDesignElement;
  auto SkipAttributeInstances() -> void;
  auto SkipUnsupportedElementToSemicolon(std::string_view stop_keyword = {})
      -> void;
  auto SkipUnsupportedElementToMatchingEnd(
      std::string_view start_keyword, std::string_view end_keyword,
      UnsupportedElementEndOptions const& options) -> void;
  auto ParseModuleDeclaration() -> ::svt::model::ModuleDeclaration;
  auto ParseModuleItems() -> std::vector<::svt::model::ModuleItem>;
  auto ParseModuleItem() -> ::svt::model::ModuleItem;
  auto ParseUnsupportedModuleItem() -> ::svt::model::UnsupportedModuleItem;
  auto ParseNetDeclaration() -> ::svt::model::NetDeclaration;
  auto ParseContinuousAssign() -> ::svt::model::ContinuousAssign;
  auto ParseModuleInstantiation() -> ::svt::model::ModuleInstantiation;
  auto ParseAlwaysEventControl() -> tokens_t;
  auto ParseGenerateBlock() -> ::svt::model::GenerateBlock;
  auto ConsumeBalancedDelimitedTokens(::svt::model::TokenType opening_token,
                                      ::svt::model::TokenType closing_token,
                                      std::string_view context) -> tokens_t;
  auto ParseKeywordBlockBody(std::string_view start_keyword,
                             std::string_view end_keyword,
                             std::string_view context) -> tokens_t;
  auto AdvanceToTopLevelListBoundary(std::string_view context) -> void;
  auto ParseBeginEndBlockBody(std::string_view context) -> tokens_t;
  auto ParseSingleStatementBody(std::string_view context) -> tokens_t;
  auto ParseParameterTokens() -> tokens_t;
  auto ParseParameters() -> std::vector<::svt::model::ParameterDeclaration>;
  auto ParsePorts() -> std::vector<::svt::model::PortDeclaration>;

  Lexer m_lexer;
  tokens_t m_tokens;
  tokens_t::iterator m_token_iterator;
};

/// @brief Print a parsed translation unit to stdout.
export auto Print(Parser::TranslationUnit const& translation_unit) -> void;

}  // namespace svt::core
