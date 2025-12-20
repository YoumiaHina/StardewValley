#include "Controllers/Systems/ToolUpgradeSystem.h"

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
    return lvl;
}

bool ToolUpgradeSystem::upgradeToolOnce(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const {
    if (!inv) return false;
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
    return false;
}

}

