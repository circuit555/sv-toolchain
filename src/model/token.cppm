

export module svt.model.token;

import std;

export namespace svt::model {

struct SourceLocation {
  std::size_t row{1};
  std::size_t column{1};
};

enum class TokenType : std::uint8_t {
  kEndOfFile,
  kIdentifier,
  kIntegerLiteral,
  kRealLiteral,
  kStringLiteral,
  kKeyword,
  kOperator,
  kPunctuation,
  kComment
};

struct Token {
  TokenType type;
  std::string_view lexeme;
  SourceLocation location;
};

}  // namespace svt::model
