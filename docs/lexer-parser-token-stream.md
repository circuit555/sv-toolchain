# Lexer and Parser Token Stream Ownership

## Decision

`Lexer` performs eager lexing. When constructed with source text, it scans the
entire input and stores the resulting token stream internally.

`Parser` owns a `Lexer`, obtains a non-owning span through `Lexer::Tokens()`,
and keeps its own cursor into that span while parsing.

## Rationale

The lexer already owns the complete source string, so lazy token production does
not provide true streaming behavior in the current architecture. Eager lexing
keeps the parser simpler because token lookahead is just indexed access into a
stable token stream.

This avoids a separate parser-side lookahead buffer and the state bugs that come
with it. For example, a FIFO queue can support `Peek()` but not `Peek(offset)`
without extra machinery. With a token span, parser lookahead is direct:

```cpp
m_tokens.at(m_position + offset)
```

## API Shape

`Lexer` exposes:

```cpp
auto Tokens() const -> std::span<Token const>;
```

It does not expose a `Next()` API. Sequential token consumption is parser state,
not lexer state. This lets multiple consumers inspect the same token stream
without mutating the lexer.

`Parser` stores:

```cpp
Lexer m_lexer;
std::span<Token const> m_tokens;
std::size_t m_position;
```

The span is valid because `m_lexer` owns the token vector and outlives the span.
The parser cursor is independent of the lexer.

## Consequences

- Lexing errors are reported during `Lexer` construction.
- Tests inspect `Lexer::Tokens()` directly.
- Parser helper functions can pass `std::span<Token const>` slices instead of
  copying token vectors.
- Memory use includes the full token stream, which is acceptable for the current
  file-oriented workflow and simplifies parser implementation.

If the project later needs true streaming over very large inputs, the lexer and
parser boundary can be revisited. Until then, eager lexing is the simpler and
more explicit model.
