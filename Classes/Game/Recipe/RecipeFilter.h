#pragma once

#include "Game/Recipe/RecipeBase.h"

namespace Game {

// RecipeCategory：配方筛选分类（用于 UI Tab/筛选器）。
enum class RecipeCategory {
    All = 0,
    Placeable,
    Mineral,
    Food,
    Material
};

// IRecipeFilter：配方筛选器接口。
// - 职责：定义“是否接受某条配方”的判断，供 UI/Controller 做列表过滤。
// - 协作对象：RecipeBook 提供配方源；UI 选择具体筛选器实现。
class IRecipeFilter {
public:
    virtual ~IRecipeFilter() = default;
    // 判断是否接受该配方。
    virtual bool accept(const RecipeBase& recipe) const = 0;
};

// CategoryRecipeFilter：按分类筛选的默认实现。
// - 职责：根据配方输出物品类型，将配方归入指定分类。
// - 协作对象：RecipeBook 负责给出 categoryForOutput 的统一规则。
class CategoryRecipeFilter final : public IRecipeFilter {
public:
    // 构造：指定目标分类。
    explicit CategoryRecipeFilter(RecipeCategory cat) : _cat(cat) {}
    // 判断是否接受该配方。
    bool accept(const RecipeBase& recipe) const override;
    // 获取当前分类。
    RecipeCategory category() const { return _cat; }

private:
    RecipeCategory _cat = RecipeCategory::All;
};

} // namespace Game
