#pragma once

#include <memory>
#include "Game/Inventory.h"

namespace Controllers {

// 工具升级系统（System 层）：
// - 负责统一计算工具等级、升级消耗和实际升级操作；
// - 是“工具升级规则”的唯一来源，UI 只调用这里的接口，不直接改背包内容。
// 设计为单例（singleton），通过 getInstance() 获取唯一实例。
class ToolUpgradeSystem {
public:
    // 获取全局唯一实例。
    static ToolUpgradeSystem& getInstance();

    // 查询某个背包中指定工具的等级：
    // - 先在传入的 inv 背包中查找；
    // - 若未找到且 inv 恰好是全局背包，则会继续在全局箱子中查。
    int toolLevel(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const;
    // 尝试将某种工具升级 1 级：
    // - 会检查是否还有下一阶段、玩家金钱和材料是否足够；
    // - 成功时从背包扣除材料、扣钱，并把工具等级 +1。
    bool upgradeToolOnce(const std::shared_ptr<Game::Inventory>& inv, Game::ToolKind kind) const;
    // 计算下一次升级所需的花费：
    // - 若已经满级，则返回 false；
    // - 若还有下一阶段，则通过引用参数返回金币消耗、材料类型、材料数量，
    //   并设置 affordable 表示当前背包和金币是否足够。
    bool nextUpgradeCost(const std::shared_ptr<Game::Inventory>& inv,
                         Game::ToolKind kind,
                         long long& goldCost,
                         Game::ItemType& materialType,
                         int& materialQty,
                         bool& affordable) const;

private:
    // 构造函数设为 private，禁止在外部直接构造，只能通过 getInstance 获取实例。
    ToolUpgradeSystem() = default;
};

}
