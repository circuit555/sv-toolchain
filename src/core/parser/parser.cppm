

export module svt.core.parser;

import std;
import svt.model.token;
import svt.model.ast;

namespace svt::core {

/// @brief Lexical analyzer for SystemVerilog source text.
export class Lexer final {
 public:
  explicit Lexer(std::string&& sv_source_code);
  [[nodiscard]] auto Next() -> ::svt::model::Token;

 private:
  template <std::size_t kOffset = 0>
  [[nodiscard]] auto Peek() const -> char;
  auto SkipWhiteSpaceAndComments() -> void;
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
};

/// @brief Recursive-descent parser for SystemVerilog source text.
///
/// The parser tokenizes input internally and produces an AST translation unit.
export class Parser final {
  using TranslationUnit = std::vector<::svt::model::AstNode>;

 public:
  /// @brief Construct a parser over a source-code view.
  /// @param sv_source_code SystemVerilog source to parse.
  explicit Parser(std::string&& sv_source_code);
  /// @brief Parse the input and produce a translation unit AST.
  /// @return Parsed translation unit.
  [[nodiscard]] auto Parse() -> TranslationUnit;

 private:
  auto Peek(std::size_t offset = 0) -> ::svt::model::Token;
  auto ConsumeToken() -> void;
  auto ParseDeclaration() -> ::svt::model::AstNode;
  auto ParseModuleDeclaration() -> ::svt::model::AstNode;
  auto ParseParameters() -> std::vector<::svt::model::ParameterDeclaration>;
  auto ParsePorts() -> ::svt::model::PortDeclaration;

  Lexer m_lexer;
  std::queue<::svt::model::Token> m_lookahead_buffer;
};

}  // namespace svt::core
