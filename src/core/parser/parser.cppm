

export module svt.core.parser;

import std;
import svt.model.token;

namespace svt::core {

export class Lexer final {
  using SourceLocation = ::svt::model::SourceLocation;
  using TokenType = ::svt::model::TokenType;
  using Token = ::svt::model::Token;

 public:
  explicit Lexer(std::string_view sv_source_code);
  [[nodiscard]] auto Next() -> Token;

 private:
  template <std::size_t kOffset = 0>
  [[nodiscard]] auto Peek() const -> char;
  auto SkipWhiteSpaceAndComments() -> void;
  auto ScanString(SourceLocation const& token_source_location) -> Token;
  auto ScanIdentifierOrKeyword(SourceLocation const& token_source_location)
      -> Token;
  auto ScanNumber(SourceLocation const& token_source_location) -> Token;

  std::size_t m_position{0};
  std::string_view m_sv_source_code_view;
  SourceLocation m_source_location{};
};

export class Parser final {
 public:
  explicit Parser(std::string_view sv_source_code);

 private:
  Lexer m_lexer;
};

}  // namespace svt::core
