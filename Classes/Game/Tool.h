/**
 * Tool: defines tool types and basic tool model.
 */
#pragma once

#include <string>

namespace Game {

enum class ToolType {
    Axe,
    WateringCan,
    Pickaxe,
    Hoe,
    FishingRod
};

struct Tool {
    ToolType type;
    std::string name;

    Tool() : type(ToolType::Axe), name("Axe") {}
    Tool(ToolType t, std::string n) : type(t), name(std::move(n)) {}
};

inline Tool makeTool(ToolType type) {
    switch (type) {
        case ToolType::Axe:          return Tool(type, "Axe");
        case ToolType::WateringCan:  return Tool(type, "Water Can");
        case ToolType::Pickaxe:      return Tool(type, "Pickaxe");
        case ToolType::Hoe:          return Tool(type, "Hoe");
        case ToolType::FishingRod:   return Tool(type, "Fishing Rod");
        default:                     return Tool(ToolType::Axe, "Axe");
    }
}

} // namespace Game
