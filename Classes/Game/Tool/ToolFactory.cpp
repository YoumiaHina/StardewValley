#include "Game/Tool/ToolFactory.h"
#include "Game/Tool/Axe.h"
#include "Game/Tool/Hoe.h"
#include "Game/Tool/Pickaxe.h"
#include "Game/Tool/WaterCan.h"
#include "Game/Tool/FishingRod.h"

namespace Game {

std::shared_ptr<ToolBase> makeTool(ToolKind kind) {
    switch (kind) {
        case ToolKind::Axe: return std::make_shared<Axe>();
        case ToolKind::Hoe: return std::make_shared<Hoe>();
        case ToolKind::Pickaxe: return std::make_shared<Pickaxe>();
        case ToolKind::WaterCan: return std::make_shared<WaterCan>();
        case ToolKind::FishingRod: return std::make_shared<FishingRod>();
        default: return std::make_shared<Axe>();
    }
}

}

