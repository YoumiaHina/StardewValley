#pragma once

#include <memory>
#include "Game/Inventory.h"

namespace Controllers {

class ToolUpgradeSystem {
public:
    static ToolUpgradeSystem& getInstance();

    int toolLevel(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const;
    bool upgradeToolOnce(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const;
    bool nextUpgradeCost(const std::shared_ptr<Game::Inventory>& inv,
                         Game::ToolKind kind,
                         long long& goldCost,
                         Game::ItemType& materialType,
                         int& materialQty,
                         bool& affordable) const;

private:
    ToolUpgradeSystem() = default;
};

}
