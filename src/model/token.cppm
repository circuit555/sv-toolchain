// SPDX-License-Identifier: MIT

export module svt.model.token;

import std;

namespace svt::model {

/// @brief 1-based source position in the input text.
export struct SourceLocation {
  /// Source line number, starting at 1.
  std::size_t row{1};
  /// Source column number, starting at 1.
  std::size_t column{1};
};

/// @brief Token category produced by lexical analysis.
export enum class TokenType : std::uint8_t {
  /// End-of-file sentinel token.
  kEndOfFile,
  /// User-defined identifier.
  kIdentifier,
  /// Integer numeric literal.
  kIntegerLiteral,
  /// Real/floating-point numeric literal.
  kRealLiteral,
  /// String literal.
  kStringLiteral,
  /// Language keyword.
  kKeyword,
  /// Operator token.
  kOperator,
  /// `(` token.
  kLParen,
  /// `)` token.
  kRParen,
  /// `[` token.
  kLBracket,
  /// `]` token.
  kRBracket,
  /// `{` token.
  kLBrace,
  /// `}` token.
  kRBrace,
  /// `;` token.
  kSemicolon,
  /// `,` token.
  kComma,
  /// `#` token.
  kHash,
  /// `:` token.
  kColon,
  /// `?` token.
  kQuestion,
  /// Comment token.
  kComment
};

/// @brief A single lexical token with type, text, and position.
export struct Token {
  /// Token category.
  TokenType type;
  /// Source slice for the token text.
  std::string_view lexeme;
  /// Source location of the token start.
  SourceLocation location;
};

/// @brief Sequence of tokens.
export using token_stream_t = std::vector<Token>;

}  // namespace svt::model
