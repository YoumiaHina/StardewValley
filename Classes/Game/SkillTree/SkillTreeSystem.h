#pragma once

#include <cstddef>
#include <string>
#include "Game/Item.h"
#include "Game/WorldState.h"
#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// SkillTreeSystem：技能经验、等级、点数与加成的统一入口。
// - 职责边界：只负责“技能树进度数据”的读写、规则计算与加成查询；不直接驱动 UI 和输入。
// - 协作对象：WorldState 作为唯一持久化来源；工具/系统（如 Hoe）通过本系统增加经验并获取加成；UI 通过本系统查询展示数据。
class SkillTreeSystem {
public:
    // 获取全局技能树系统单例（无额外状态，内部直接读写 WorldState）。
    static SkillTreeSystem& getInstance();

    // 返回指定技能树的静态定义（节点、等级曲线等）。
    const SkillTreeBase& definition(SkillTreeType type) const;

    // 返回指定技能树的当前等级（由累计经验计算）。
    int level(SkillTreeType type) const;
    // 返回指定技能树的累计经验。
    int totalXp(SkillTreeType type) const;
    // 返回升到下一等级还差多少经验（满级返回 0）。
    int xpToNextLevel(SkillTreeType type) const;
    // 返回指定技能树的可用技能点数（由等级与已解锁节点推导）。
    int unspentPoints(SkillTreeType type) const;

    // 判断节点是否已解锁。
    bool isNodeUnlocked(SkillTreeType type, int nodeId) const;
    // 尝试解锁节点：满足等级门槛且有点数则扣点并记录。
    bool unlockNode(SkillTreeType type, int nodeId);
    // 解锁“当前可解锁的第一个节点”（按节点列表顺序）；成功时可返回解锁的 nodeId。
    bool unlockFirstAvailableNode(SkillTreeType type, int* outUnlockedNodeId = nullptr);

    // 增加指定技能树经验；会自动处理升级与点数变化。
    void addXp(SkillTreeType type, int deltaXp, int* outLevelsGained = nullptr);

    // 获取技能树当前等级的加成描述文本（用于 UI 展示）。
    std::string bonusDescription(SkillTreeType type) const;

    // 种地加成：收获时根据当前技能树概率额外 +1 产物数量。
    int adjustHarvestQuantityForFarming(Game::ItemType produce, int baseQty) const;
    // 种地经验：一次收获应获得的经验（yields=true 时调用）。
    int xpForFarmingHarvest(Game::ItemType produce, int harvestedQty) const;

    // 砍树加成：掉落木材时根据技能树概率额外 +1 木材数量。
    int adjustWoodDropQuantityForForestry(int baseQty) const;
    // 砍树经验：一次砍倒树应获得的经验。
    int xpForForestryChop(int woodQty) const;

    // 钓鱼加成：钓到鱼时根据技能树概率额外 +1 鱼数量。
    int adjustFishCatchQuantityForFishing(int baseQty) const;
    // 钓鱼经验：一次钓鱼成功应获得的经验。
    int xpForFishingCatch(int fishQty) const;

    // 养殖加成：动物产物生成时根据技能树概率额外 +1 产物数量。
    int adjustAnimalProductQuantityForHusbandry(Game::ItemType product, int baseQty) const;
    // 养殖经验：一次动物产物生成应获得的经验。
    int xpForAnimalProduct(Game::ItemType product, int qty) const;

private:
    SkillTreeSystem() = default;
    std::size_t indexFromType(SkillTreeType type) const;
    void normalizeProgress(SkillTreeType type) const;
    float farmingExtraProduceChance() const;
    float forestryExtraWoodChance() const;
    float fishingExtraFishChance() const;
    float husbandryExtraProductChance() const;
};

}
