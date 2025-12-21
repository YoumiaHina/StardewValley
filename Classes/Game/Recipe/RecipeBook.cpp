#include "Game/Recipe/RecipeBook.h"
#include "Game/Recipe/SimpleRecipe.h"
#include "Game/Item.h"

namespace Game {

namespace {

bool isMineralLike(ItemType t) {
    switch (t) {
        case ItemType::Coal:
        case ItemType::CopperGrain:
        case ItemType::CopperIngot:
        case ItemType::IronGrain:
        case ItemType::IronIngot:
        case ItemType::GoldGrain:
        case ItemType::GoldIngot:
            return true;
        default:
            return false;
    }
}

bool isMaterialLike(ItemType t) {
    switch (t) {
        case ItemType::Wood:
        case ItemType::Stone:
        case ItemType::Fiber:
        case ItemType::Coal:
            return true;
        default:
            return false;
    }
}

std::vector<std::shared_ptr<RecipeBase>> buildAllRecipes() {
    std::vector<std::shared_ptr<RecipeBase>> out;
    out.reserve(16);

    out.push_back(std::make_shared<SimpleRecipe>(
        "placeable_chest",
        "Chest",
        ItemType::Chest,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Wood, 50 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "placeable_furnace",
        "Furnace",
        ItemType::Furnace,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Stone, 25 },
            RecipeBase::Ingredient{ ItemType::CopperGrain, 20 }
        }));

    return out;
}

} // namespace

const std::vector<std::shared_ptr<RecipeBase>>& RecipeBook::all() {
    static std::vector<std::shared_ptr<RecipeBase>> recipes = buildAllRecipes();
    return recipes;
}

std::vector<std::shared_ptr<RecipeBase>> RecipeBook::filtered(const IRecipeFilter& filter) {
    std::vector<std::shared_ptr<RecipeBase>> out;
    const auto& src = all();
    out.reserve(src.size());
    for (const auto& r : src) {
        if (r && filter.accept(*r)) {
            out.push_back(r);
        }
    }
    return out;
}

RecipeCategory RecipeBook::categoryForOutput(ItemType out) {
    if (out == ItemType::Chest || out == ItemType::Furnace) {
        return RecipeCategory::Placeable;
    }
    if (isMineralLike(out)) {
        return RecipeCategory::Mineral;
    }
    if (itemEdible(out)) {
        return RecipeCategory::Food;
    }
    if (isMaterialLike(out)) {
        return RecipeCategory::Material;
    }
    return RecipeCategory::All;
}

bool CategoryRecipeFilter::accept(const RecipeBase& recipe) const {
    if (_cat == RecipeCategory::All) {
        return true;
    }
    RecipeCategory c = RecipeBook::categoryForOutput(recipe.outputType());
    return c == _cat;
}

} // namespace Game
