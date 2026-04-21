

export module svt.model.token;

import std;

namespace svt::model {

export struct SourceLocation {
  std::size_t row{1};
  std::size_t column{1};
};

export enum class TokenType : std::uint8_t {
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

export struct Token {
  TokenType type;
  std::string_view lexeme;
  SourceLocation location;
};

export using token_stream_t = std::vector<Token>;

}  // namespace svt::model
