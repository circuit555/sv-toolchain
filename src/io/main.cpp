// SPDX-License-Identifier: MIT

import svt.core.parser;
import std;

auto ReadSystemVerilogSourceFile(const char* sv_source_code_path)
    -> std::string {
  std::ifstream file_stream{sv_source_code_path,
                            std::ios::binary | std::ios::ate};
  if (not file_stream.is_open()) {
    throw std::runtime_error{"sv file could not be opened"};
  }

  std::string sv_code{};
  sv_code.resize(file_stream.tellg());
  file_stream.seekg(0);
  file_stream.read(sv_code.data(),
                   static_cast<std::streamsize>(std::ranges::size(sv_code)));
  return sv_code;
}

auto main(int const argc, char const* const* argv) -> int {
  if (argc != 2) {
    std::println(std::cerr, "usage: {} <source.sv>", argv[0]);
    return 1;
  }

  try {
    svt::core::Parser parser{ReadSystemVerilogSourceFile(argv[1])};
    auto translation_unit = parser.Parse();
    svt::core::Print(translation_unit);
  } catch (std::exception const& exception) {
    std::println(std::cerr, "error: {}", exception.what());
    return 1;
  }

  return 0;
}
