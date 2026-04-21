import svt.core.parser;
import std;

auto ReadSystemVerilogSourceFile(const char* sv_source_code_path)
    -> std::string {
  // read sample system-verilog source-code file
  std::ifstream file_stream{sv_source_code_path,
                            std::ios::binary | std::ios::ate};
  if (not file_stream.is_open()) {
    throw std::runtime_error{"sv file could not be opened."};
  }

  std::string sv_code{};
  sv_code.resize(file_stream.tellg());
  file_stream.seekg(0);
  file_stream.read(sv_code.data(),
                   static_cast<std::streamsize>(std::ranges::size(sv_code)));
  return sv_code;
}

auto main() -> int {
  svt::core::Parser parser{ReadSystemVerilogSourceFile(
      "/stuff/work/sv-toolchain/example/lexer/foo.sv")};
  auto foo = parser.Parse();
}
