#include "Controllers/Systems/ToolUpgradeSystem.h"
#include "Game/WorldState.h"

namespace Controllers {

ToolUpgradeSystem& ToolUpgradeSystem::getInstance() {
    static ToolUpgradeSystem inst;
    return inst;
}

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
    if (currentLevel < 0 || currentLevel >= 3) {
        return false;
    }
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
    affordable = (ws.gold >= goldCost && have >= materialQty);
    return true;
}

}
