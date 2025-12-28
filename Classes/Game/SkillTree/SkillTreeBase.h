#pragma once

#include <vector>

namespace Game {

// 技能树类型：用于索引不同技能树的静态定义（各 SkillTreeBase 派生类）。
enum class SkillTreeType {
    Farming,
    AnimalHusbandry,
    Forestry,
    Fishing,
    Mining,
    Combat
};

// 技能节点定义：
// - 描述一个可解锁节点的 id、显示名与解锁等级门槛。
// - 协作对象：SkillTreeBase 使用该结构提供查找与解锁判定；进度侧保存已解锁 id 集合。
struct SkillNode {
    int id = 0;
    const char* displayName = "";
    int requiredLevel = 0;
};

// 技能树基类：
// - 提供等级/经验的通用计算。
// - 协作对象：SkillTreeSystem 负责持久化与业务加成查询；本类只做规则计算。
class SkillTreeBase {
public:
    // 虚析构：允许通过基类引用销毁派生实例（当前派生为静态单例）。
    virtual ~SkillTreeBase() = default;

    // 返回技能树类型枚举，用于被上层索引。
    virtual SkillTreeType type() const = 0;
    // 返回技能树显示名称（用于 UI 标题等）。
    virtual const char* displayName() const = 0;
    // 返回最大等级（默认 10）。
    virtual int maxLevel() const { return 10; }
    // 从 currentLevel 升到 currentLevel+1 所需经验（默认采用通用曲线）。
    virtual int xpForNextLevel(int currentLevel) const;
    // 返回该技能树的所有节点定义（引用长期有效）。
    virtual const std::vector<SkillNode>& nodes() const = 0;

    // 获取达到指定等级所需的累计经验（level=0 返回 0）。
    int totalXpRequiredForLevel(int level) const;
    // 由累计经验反推等级（上限为 maxLevel）。
    int levelFromTotalXp(int totalXp) const;
    // 返回距离下一等级还差多少经验（满级返回 0）。
    int xpToNextLevelFromTotalXp(int totalXp) const;
};

// 获取各技能树静态定义单例：由上层通过 type 或直接引用使用。
const SkillTreeBase& farmingSkillTree();
const SkillTreeBase& animalHusbandrySkillTree();
const SkillTreeBase& forestrySkillTree();
const SkillTreeBase& fishingSkillTree();
const SkillTreeBase& miningSkillTree();
const SkillTreeBase& combatSkillTree();
const SkillTreeBase& skillTreeByType(SkillTreeType t);

}
