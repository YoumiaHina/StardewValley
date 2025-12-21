#pragma once

#include "Game/Recipe/RecipeBase.h"
#include <string>
#include <vector>

namespace Game {

// SimpleRecipe：数据驱动的基础配方实现。
// - 职责：存放配方元数据（id/名称/材料/产物）并复用 RecipeBase 的通用执行逻辑。
// - 协作对象：RecipeBook 负责集中创建与持有；CraftingController/UI 只通过 RecipeBase 接口使用。
class SimpleRecipe final : public RecipeBase {
public:
    // 构造：写入配方的 id/名称/产物与材料列表。
    SimpleRecipe(std::string id,
                 std::string name,
                 ItemType outType,
                 int outQty,
                 std::vector<Ingredient> ingredients);

    // 获取配方稳定标识（用于保存/排序/去重）。
    const char* id() const override;
    // 获取配方显示名称（用于 UI 展示）。
    const char* displayName() const override;
    // 获取输出物品类型。
    ItemType outputType() const override;
    // 获取输出物品数量。
    int outputQty() const override;
    // 获取材料列表。
    const std::vector<Ingredient>& ingredients() const override;

private:
    std::string _id;
    std::string _name;
    ItemType _outType = ItemType::Wood;
    int _outQty = 1;
    std::vector<Ingredient> _ingredients;
};

} // namespace Game

