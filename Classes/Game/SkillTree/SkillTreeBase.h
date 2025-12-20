#pragma once

#include <unordered_set>
#include <vector>

namespace Game {

// 技能树类型：用于索引不同技能树的静态定义（各 SkillTreeBase 派生类）。
enum class SkillTreeType {
    Farming,
    AnimalHusbandry,
    Forestry,
    Fishing
};

// 技能节点定义：
// - 描述一个可解锁节点的 id、显示名与解锁等级门槛。
// - 协作对象：SkillTreeBase 使用该结构提供查找与解锁判定；进度侧保存已解锁 id 集合。
struct SkillNode {
    int id = 0;
    const char* displayName = "";
    int requiredLevel = 0;
};

// 技能树进度（运行时状态）：
// - 由上层系统（例如 SkillTreeSystem）作为唯一来源持有并存档。
// - SkillTreeBase 仅提供“静态定义 + 规则计算”，不直接持有进度实例。
struct SkillProgress {
    int level = 0;
    int totalXp = 0;
    int unspentPoints = 0;
    std::unordered_set<int> unlockedNodeIds;
};

// 技能树基类：
// - 提供等级/经验/点数的通用计算，以及节点解锁判定。
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
    // 每次升级获得的技能点数（默认 1）。
    virtual int pointsPerLevel() const { return 1; }
    // 从 currentLevel 升到 currentLevel+1 所需经验（默认采用通用曲线）。
    virtual int xpForNextLevel(int currentLevel) const;
    // 返回该技能树的所有节点定义（引用长期有效）。
    virtual const std::vector<SkillNode>& nodes() const = 0;

    // 获取达到指定等级所需的累计经验（level=0 返回 0）。
    int totalXpRequiredForLevel(int level) const;
    // 由累计经验反推等级（上限为 maxLevel）。
    int levelFromTotalXp(int totalXp) const;
    // 刷新进度中的 level 与 unspentPoints（依据 totalXp 变化自动补发点数）。
    void syncProgress(SkillProgress& progress) const;
    // 增加经验并同步等级/点数；outLevelsGained 非空时返回本次提升的等级数。
    void addXp(SkillProgress& progress, int deltaXp, int* outLevelsGained = nullptr) const;
    // 返回距离下一等级还差多少经验（满级返回 0）。
    int xpToNextLevel(const SkillProgress& progress) const;

    // 查找节点定义；找不到返回 nullptr。
    const SkillNode* findNode(int nodeId) const;
    // 判断节点是否已解锁。
    bool isNodeUnlocked(const SkillProgress& progress, int nodeId) const;
    // 判断当前进度是否满足解锁条件（等级门槛 + 点数 + 未解锁）。
    bool canUnlockNode(const SkillProgress& progress, int nodeId) const;
    // 尝试解锁节点：成功则扣点并记录解锁 id。
    bool unlockNode(SkillProgress& progress, int nodeId) const;
};

// 获取各技能树静态定义单例：由上层通过 type 或直接引用使用。
const SkillTreeBase& farmingSkillTree();
const SkillTreeBase& animalHusbandrySkillTree();
const SkillTreeBase& forestrySkillTree();
const SkillTreeBase& fishingSkillTree();
const SkillTreeBase& skillTreeByType(SkillTreeType t);

}

