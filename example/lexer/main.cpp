

import std;
import fmt;
import svt.core.parser;
import svt.model.token;

namespace fs = std::filesystem;
namespace rng = std::ranges;

auto main() -> int {
  // read sample system-verilog source-code file
  std::ifstream file_stream{fs::path(__FILE__).parent_path() / "foo.sv",
                            std::ios::binary | std::ios::ate};
  if (not file_stream.is_open()) {
    fmt::println("foo.sv could not be opened.");
  }
  std::string sv_code{};
  sv_code.resize(file_stream.tellg());
  file_stream.seekg(0);
  file_stream.read(sv_code.data(), rng::size(sv_code));

  // extract token-stream using Lexer
  svt::core::Lexer lexer{std::move(sv_code)};
  auto const token_stream{lexer.Tokens()};

  // print out token-stream
  fmt::println("Token-Stream for foo.sv:");
  fmt::println("");
  for (auto const& token : token_stream) {
    if (token.type == svt::model::TokenType::kEndOfFile) {
      break;
    }

    fmt::println("({}, {}) `{}`", token.location.row, token.location.column,
                 token.lexeme);
  }
}
