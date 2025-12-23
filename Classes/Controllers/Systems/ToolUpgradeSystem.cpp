#include "Controllers/Systems/ToolUpgradeSystem.h"
#include "Game/WorldState.h"

namespace Controllers {

// 单例实现：使用函数内部的 static 局部变量。
// C++11 之后保证线程安全，只会构造一次。
ToolUpgradeSystem& ToolUpgradeSystem::getInstance() {
    static ToolUpgradeSystem inst;
    return inst;
}

// 查询某个背包中指定工具的等级。
// 逻辑：
// 1. 在传入的 inv 中顺序查找第一个 kind 匹配的工具；
// 2. 若找不到且 inv 恰好指向全局 WorldState::inventory，
//    则继续在全局箱子 globalChest 里查找；
// 3. 默认等级为 0，表示没有该工具。
int ToolUpgradeSystem::toolLevel(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const {
    if (!inv) return 0;
    int lvl = 0;
    std::size_t sz = inv->size();
    for (std::size_t i = 0; i < sz; ++i) {
        const Game::ToolBase* t = inv->toolAt(i);
        if (t && t->kind() == kind) {
            lvl = t->level();
            break;
        }
    }
    if (lvl == 0) {
        auto& ws = Game::globalState();
        const Game::Inventory* wsInv = ws.inventory.get();
        if (inv.get() == wsInv) {
            for (const auto& s : ws.globalChest.slots) {
                if (s.kind == Game::SlotKind::Tool && s.tool && s.tool->kind() == kind) {
                    lvl = s.tool->level();
                    break;
                }
            }
        }
    }
    return lvl;
}

// 尝试把某种工具升级 1 级。
// 主要步骤：
// 1. 调用 nextUpgradeCost 计算下一阶段的费用与材料，并检查是否可负担；
// 2. 从 WorldState 扣钱，从背包 inv 中扣除对应材料；
// 3. 优先在背包中寻找该工具并把 level +1；
// 4. 若背包中没有但在全局箱子中存在，也会在那里升级。
bool ToolUpgradeSystem::upgradeToolOnce(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const {
    if (!inv) return false;
    long long goldCost = 0;
    Game::ItemType materialType = Game::ItemType::CopperIngot;
    int materialQty = 0;
    bool affordable = false;
    bool hasNext = nextUpgradeCost(inv, kind, goldCost, materialType, materialQty, affordable);
    if (!hasNext || !affordable) {
        return false;
    }
    auto& ws = Game::globalState();
    if (ws.gold < goldCost) {
        return false;
    }
    bool removed = inv->removeItems(materialType, materialQty);
    if (!removed) {
        return false;
    }
    ws.gold -= goldCost;
    std::size_t sz = inv->size();
    for (std::size_t i = 0; i < sz; ++i) {
        Game::ToolBase* t = inv->toolAtMutable(i);
        if (t && t->kind() == kind) {
            int lv = t->level();
            if (lv >= 3) {
                return false;
            }
            t->setLevel(lv + 1);
            return true;
        }
    }
    Game::Inventory* wsInv = ws.inventory.get();
    if (inv.get() == wsInv) {
        for (auto& s : ws.globalChest.slots) {
            if (s.kind == Game::SlotKind::Tool && s.tool && s.tool->kind() == kind) {
                int lv = s.tool->level();
                if (lv >= 3) {
                    return false;
                }
                s.tool->setLevel(lv + 1);
                return true;
            }
        }
    }
    return false;
}

// 计算下一次升级所需的花费：
// - 返回值：true 表示存在下一等级（即当前未满级），false 表示已经满级或没有该工具；
// - goldCost：升级一次需要的金币数量；
// - materialType / materialQty：升级所需的材料类型与数量；
// - affordable：当前金币与材料是否足够（仅做一次静态判断，不会修改任何状态）。
bool ToolUpgradeSystem::nextUpgradeCost(const std::shared_ptr<Game::Inventory>& inv,
                                        Game::ToolKind kind,
                                        long long& goldCost,
                                        Game::ItemType& materialType,
                                        int& materialQty,
                                        bool& affordable) const {
    goldCost = 0;
    materialType = Game::ItemType::CopperIngot;
    materialQty = 0;
    affordable = false;
    if (!inv) return false;
    // 先尝试在背包中找到该工具，记录其当前等级。
    int currentLevel = -1;
    std::size_t sz = inv->size();
    for (std::size_t i = 0; i < sz; ++i) {
        const Game::ToolBase* t = inv->toolAt(i);
        if (t && t->kind() == kind) {
            currentLevel = t->level();
            break;
        }
    }
    if (currentLevel < 0) {
        auto& ws = Game::globalState();
        const Game::Inventory* wsInv = ws.inventory.get();
        if (inv.get() == wsInv) {
            for (const auto& s : ws.globalChest.slots) {
                if (s.kind == Game::SlotKind::Tool && s.tool && s.tool->kind() == kind) {
                    currentLevel = s.tool->level();
                    break;
                }
            }
        }
    }
    // 若没找到工具，或等级已经达到上限（这里约定 Max=3），则没有下一等级。
    if (currentLevel < 0 || currentLevel >= 3) {
        return false;
    }
    // 根据当前等级决定下一次升级所需的金币与材料类型：
    // level 0 -> 铜锭，level 1 -> 铁锭，level 2 -> 金锭。
    if (currentLevel == 0) {
        goldCost = 2000;
        materialType = Game::ItemType::CopperIngot;
        materialQty = 5;
    } else if (currentLevel == 1) {
        goldCost = 5000;
        materialType = Game::ItemType::IronIngot;
        materialQty = 5;
    } else if (currentLevel == 2) {
        goldCost = 10000;
        materialType = Game::ItemType::GoldIngot;
        materialQty = 5;
    }
    auto& ws = Game::globalState();
    int have = inv->countItems(materialType);
    // affordable 只做“当前状态下是否足够”的标记，不会修改任何数据。
    affordable = (ws.gold >= goldCost && have >= materialQty);
    return true;
}

}
