#include "Controllers/Crafting/CraftingController.h"

namespace Controllers {

CraftingController::CraftingController(std::shared_ptr<Game::Inventory> inventory)
    : _inventory(std::move(inventory)) {
}

bool CraftingController::canCraft(const Game::RecipeBase& recipe) const {
    if (!_inventory) {
        return false;
    }
    return recipe.canCraft(*_inventory);
}

bool CraftingController::craft(const Game::RecipeBase& recipe) {
    if (!_inventory) {
        return false;
    }
    return recipe.craft(*_inventory);
}

} // namespace Controllers

