#pragma once

#include <memory>
#include "Game/Inventory.h"
#include "Game/Recipe/RecipeBase.h"

namespace Controllers {

// CraftingController：合成控制器。
// - 职责：负责合成时背包物品的扣除与产物添加；不负责 UI 绘制与交互。
// - 协作对象：Inventory 提供物品数量与增删；RecipeBase 描述配方输入输出。
class CraftingController {
public:
    // 构造：持有背包引用，用于合成时修改物品数量。
    explicit CraftingController(std::shared_ptr<Game::Inventory> inventory);

    // 判定该配方当前是否可合成（不修改背包）。
    bool canCraft(const Game::RecipeBase& recipe) const;
    // 执行合成：扣除材料并添加产物。
    bool craft(const Game::RecipeBase& recipe);

private:
    std::shared_ptr<Game::Inventory> _inventory;
};

} // namespace Controllers

