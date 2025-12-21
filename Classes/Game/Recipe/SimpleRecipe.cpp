#include "Game/Recipe/SimpleRecipe.h"

namespace Game {

SimpleRecipe::SimpleRecipe(std::string id,
                           std::string name,
                           ItemType outType,
                           int outQty,
                           std::vector<Ingredient> ingredients)
    : _id(std::move(id))
    , _name(std::move(name))
    , _outType(outType)
    , _outQty(outQty)
    , _ingredients(std::move(ingredients)) {
}

const char* SimpleRecipe::id() const {
    return _id.c_str();
}

const char* SimpleRecipe::displayName() const {
    return _name.c_str();
}

ItemType SimpleRecipe::outputType() const {
    return _outType;
}

int SimpleRecipe::outputQty() const {
    return _outQty;
}

const std::vector<RecipeBase::Ingredient>& SimpleRecipe::ingredients() const {
    return _ingredients;
}

} // namespace Game

