#pragma once

#include <vector>
#include <string>
#include "Game/Item.h"

namespace Game {

class Inventory;

// RecipeBase：合成配方抽象基类。
// - 职责：描述“多种输入材料 -> 输出物品”的配方，并提供可合成判定与执行接口。
// - 协作对象：Inventory 作为物品数量的唯一来源；Controller/UI 仅通过本接口查询与触发合成。
class RecipeBase {
public:
    // Ingredient：配方材料条目。
    struct Ingredient {
        ItemType type = ItemType::Wood; // 材料物品类型
        int qty = 0; // 所需数量（必须 >0 才视为有效）
    };

    virtual ~RecipeBase() = default;

    // 获取配方稳定标识（用于保存/排序/去重）。
    virtual const char* id() const = 0;
    // 获取配方显示名称（用于 UI 展示）。
    virtual const char* displayName() const = 0;
    // 获取输出物品类型。
    virtual ItemType outputType() const = 0;
    // 获取输出物品数量。
    virtual int outputQty() const = 0;
    // 获取材料列表。
    virtual const std::vector<Ingredient>& ingredients() const = 0;

    // 判断背包内材料是否满足（不修改背包）。
    virtual bool canCraft(const Inventory& inv) const;
    // 执行合成：扣除材料并添加产物。
    virtual bool craft(Inventory& inv) const;
};

} // namespace Game
