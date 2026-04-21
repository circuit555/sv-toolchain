

export module svt.core.parser;

import std;
import svt.model.token;
import svt.model.ast;

namespace svt::core {

/// @brief Lexical analyzer for SystemVerilog source text.
class Lexer final {
 public:
  explicit Lexer(std::string_view sv_source_code);
  [[nodiscard]] auto Next() -> ::svt::model::Token;
  [[nodiscard]] auto Lex() -> ::svt::model::token_stream_t;

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

  std::size_t m_position{0};
  std::string_view m_sv_source_code_view;
  ::svt::model::SourceLocation m_source_location{};
};

/// @brief Recursive-descent parser for SystemVerilog source text.
///
/// The parser tokenizes input internally and produces an AST translation unit.
export class Parser final {
 public:
  /// @brief Construct a parser over a source-code view.
  /// @param sv_source_code SystemVerilog source to parse.
  explicit Parser(std::string_view sv_source_code);
  /// @brief Parse the input and produce a translation unit AST.
  /// @return Parsed translation unit.
  [[nodiscard]] auto Parse() -> ::svt::model::TranslationUnit;

 private:
  auto Peek(std::size_t offset = 0) -> ::svt::model::Token;
  auto ParseDeclaration() -> ::svt::model::AstNodePointer;
  auto ParseModuleDeclaration() -> ::svt::model::AstNodePointer;

  Lexer m_lexer;
  std::deque<::svt::model::Token> m_lookahead_buffer;
};

}  // namespace svt::core
