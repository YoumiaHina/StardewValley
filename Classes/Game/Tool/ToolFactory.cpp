#include "Game/Tool/ToolFactory.h"
#include "Game/Tool/Axe.h"
#include "Game/Tool/Hoe.h"
#include "Game/Tool/Pickaxe.h"
#include "Game/Tool/WaterCan.h"
#include "Game/Tool/FishingRod.h"
#include "Game/Tool/Sword.h"
#include "Game/Tool/Scythe.h"

namespace Game {

// 根据传入的 ToolKind 枚举创建不同的工具对象：
// - 使用 std::make_shared<T>() 在堆上分配对象，并返回 shared_ptr；
// - default 分支给一个合理的默认值（这里选择 Axe），避免出现空指针。
std::shared_ptr<ToolBase> makeTool(ToolKind kind) {
    switch (kind) {
        case ToolKind::Axe: return std::make_shared<Axe>();
        case ToolKind::Hoe: return std::make_shared<Hoe>();
        case ToolKind::Pickaxe: return std::make_shared<Pickaxe>();
        case ToolKind::WaterCan: return std::make_shared<WaterCan>();
        case ToolKind::FishingRod: return std::make_shared<FishingRod>();
        case ToolKind::Sword: return std::make_shared<Sword>();
        case ToolKind::Scythe: return std::make_shared<Scythe>();
        default: return std::make_shared<Axe>();
    }
}

}
