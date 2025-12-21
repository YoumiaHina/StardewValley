#include "Game/Recipe/RecipeBook.h"
#include "Game/Recipe/SimpleRecipe.h"
#include "Game/Item.h"

namespace Game {

namespace {

std::vector<std::shared_ptr<RecipeBase>> buildAllRecipes() {
    std::vector<std::shared_ptr<RecipeBase>> out;
    out.reserve(32);

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

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_omelet",
        "Omelet",
        ItemType::Omelet,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Egg, 3 },
            RecipeBase::Ingredient{ ItemType::Milk, 3 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_parsnip_soup",
        "Parsnip Soup",
        ItemType::ParsnipSoup,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Parsnip, 7 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_blueberry_strawberry",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Blueberry, 10 },
            RecipeBase::Ingredient{ ItemType::Strawberry, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_blueberry_parsnip",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Blueberry, 10 },
            RecipeBase::Ingredient{ ItemType::Parsnip, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_blueberry_eggplant",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Blueberry, 10 },
            RecipeBase::Ingredient{ ItemType::Eggplant, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_strawberry_parsnip",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Strawberry, 10 },
            RecipeBase::Ingredient{ ItemType::Parsnip, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_strawberry_eggplant",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Strawberry, 10 },
            RecipeBase::Ingredient{ ItemType::Eggplant, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_salad_parsnip_eggplant",
        "Salad",
        ItemType::Salad,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Parsnip, 10 },
            RecipeBase::Ingredient{ ItemType::Eggplant, 10 },
            RecipeBase::Ingredient{ ItemType::Milk, 1 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_fried_egg",
        "Fried Egg",
        ItemType::FriedEgg,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Egg, 2 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_tortilla",
        "Tortilla",
        ItemType::Tortilla,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Corn, 3 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_blueberry_tart",
        "Blueberry Tart",
        ItemType::BlueberryTart,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Milk, 1 },
            RecipeBase::Ingredient{ ItemType::Blueberry, 10 },
            RecipeBase::Ingredient{ ItemType::Egg, 3 }
        }));

    out.push_back(std::make_shared<SimpleRecipe>(
        "food_eggplant_parmesan",
        "Eggplant Parmesan",
        ItemType::EggplantParmesan,
        1,
        std::vector<RecipeBase::Ingredient>{
            RecipeBase::Ingredient{ ItemType::Milk, 2 },
            RecipeBase::Ingredient{ ItemType::Eggplant, 4 }
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

bool CategoryRecipeFilter::accept(const RecipeBase& recipe) const {
    auto out = recipe.outputType();
    switch (_cat) {
        case RecipeCategory::Placeable:
            return out == ItemType::Chest || out == ItemType::Furnace;
        case RecipeCategory::Food:
            return itemEdible(out);
        default:
            return false;
    }
}

} // namespace Game
