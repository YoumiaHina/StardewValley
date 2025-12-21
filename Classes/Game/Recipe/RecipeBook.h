#pragma once

#include <memory>
#include <vector>
#include "Game/Recipe/RecipeBase.h"
#include "Game/Recipe/RecipeFilter.h"

namespace Game {

// RecipeBook：合成配方集中存放处（便于增删）。
// - 职责：统一创建并持有所有 RecipeBase 实例；提供列表访问。
// - 协作对象：CraftingController/UI 通过本接口读取配方与分类；不直接分散在多个模块维护配方表。
class RecipeBook {
public:
    // 获取全部配方（只读引用）。
    static const std::vector<std::shared_ptr<RecipeBase>>& all();
    // 获取指定分类的配方列表（按当前规则过滤）。
    static std::vector<std::shared_ptr<RecipeBase>> filtered(const IRecipeFilter& filter);
};

} // namespace Game
