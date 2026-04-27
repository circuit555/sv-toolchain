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
  [[nodiscard]] auto Peek() const -> char;
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
  using TranslationUnit = std::vector<::svt::model::AstNode>;

  /// @brief Construct a parser over a source-code view.
  /// @param sv_source_code SystemVerilog source to parse.
  explicit Parser(std::string&& sv_source_code);
  /// @brief Parse the input and produce a translation unit AST.
  /// @return Parsed translation unit.
  [[nodiscard]] auto Parse() -> TranslationUnit;

 private:
  auto ExpectToken(::svt::model::TokenType expected_type,
                   std::string_view context) -> void;
  auto ParseDeclaration() -> ::svt::model::AstNode;
  auto ParseModuleDeclaration() -> ::svt::model::ModuleDeclaration;
  auto ParseModuleItems() -> std::vector<::svt::model::ModuleItem>;
  auto ParseContinuousAssign() -> ::svt::model::ContinuousAssign;
  auto ParseParameterTokens() -> std::span<::svt::model::Token const>;
  auto ParseParameters() -> std::vector<::svt::model::ParameterDeclaration>;
  auto ParsePorts() -> std::vector<::svt::model::PortDeclaration>;

  Lexer m_lexer;
  std::span<::svt::model::Token const> m_tokens;
  std::span<::svt::model::Token const>::iterator m_token_iterator;
};

/// @brief Print a parsed translation unit to stdout.
export auto Print(Parser::TranslationUnit const& translation_unit) -> void;

}  // namespace svt::core
