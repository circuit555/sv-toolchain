// SPDX-License-Identifier: MIT

export module svt.model.ast;

import std;
import svt.model.token;

namespace svt::model {

export enum class PortDirection : std::uint8_t { kInput, kOutput };

export enum class NetType : std::uint8_t { kWire, kLogic };

struct Declaration {
  std::string_view name;
};

export struct PortDeclaration : Declaration {
  PortDirection direction{};
};

export struct NetDeclaration : Declaration {
  NetType type;
  std::span<Token const> type_specifier;
};

export struct ParameterTypeDeclaration : Declaration {
  std::span<Token const> default_type;
};

export struct ParameterValueDeclaration : Declaration {
  std::span<Token const> type_specifier;
  std::span<Token const> default_value;
};

export using ParameterDeclaration =
    std::variant<ParameterTypeDeclaration, ParameterValueDeclaration>;

export struct ContinuousAssign {
  std::span<Token const> left_hand_side;
  std::span<Token const> right_hand_side;
};

export struct AlwaysBlock {
  std::span<Token const> event_control;
  std::span<Token const> body;
};

export struct InitialBlock {
  std::span<Token const> body;
};

export struct GenerateBlock {
  std::span<Token const> body;
};

export using ModuleItem =
    std::variant<NetDeclaration, ContinuousAssign, AlwaysBlock, InitialBlock,
                 GenerateBlock>;

export struct ModuleDeclaration : Declaration {
  std::vector<ParameterDeclaration> parameters;
  std::vector<PortDeclaration> ports;
  std::vector<ModuleItem> items;
};

/// @brief Top-level SystemVerilog design element.
export using DesignElement = std::variant<ModuleDeclaration>;

}  // namespace svt::model
