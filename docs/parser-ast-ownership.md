# Parser and AST Ownership Semantics

## Decision

`Parser` owns the source text and token stream for AST nodes produced by a parse.
AST nodes may store non-owning references into parser-owned storage, including
`std::string_view` values that point into source text and `std::span<Token const>`
values that point into the token stream.

Code that consumes a parser-produced AST must keep the originating `Parser`
alive for at least as long as the AST is inspected.

## Rationale

The parser is currently used as a file-oriented frontend where parsing and AST
inspection happen together. In that workflow, copying every token slice or
lexeme into AST-owned storage adds noise and memory churn without providing a
needed lifetime boundary.

The same non-owning model is already used for declaration names:

```cpp
struct Declaration {
  std::string_view name;
};
```

Using token spans for contiguous syntax fragments follows that model. It lets
the AST preserve token identity and source locations without reconstructing or
copying lexeme lists.

## API Shape

`Parser` owns:

```cpp
Lexer m_lexer;
std::span<Token const> m_tokens;
std::span<Token const>::iterator m_token_iterator;
```

`Lexer` owns the source string and token vector. The AST may store views into
both through fields such as:

```cpp
std::string_view name;
std::span<Token const> default_value;
std::span<Token const> left_hand_side;
std::span<Token const> right_hand_side;
```

These fields are references, not ownership boundaries.

## Consequences

- Parser-produced AST objects are tied to the lifetime of their parser.
- AST nodes can cheaply represent contiguous token ranges.
- Parser helpers should prefer passing and storing `std::span<Token const>` for
  contiguous token groups.
- Tests that inspect token spans should compare token lexemes or token metadata
  explicitly.
- Moving AST objects away from the parser lifetime is invalid unless the AST is
  first converted into an owning representation.

If the project later needs ASTs that outlive the parser, the ownership boundary
should move to a parsed-source object that owns source text, tokens, and AST
together, or the AST should be changed to own copied strings and token data.
