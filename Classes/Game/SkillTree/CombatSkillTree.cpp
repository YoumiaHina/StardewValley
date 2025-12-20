#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 战斗技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class CombatSkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::Combat; }
    // 返回显示名称。
    const char* displayName() const override { return "Combat"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {601, "Extra Gold", 1},
            {602, "Bounty Hunter", 3},
            {603, "Power Strike", 5},
            {604, "Survivor", 7},
        };
        return kNodes;
    }
};

// 获取战斗技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& combatSkillTree() {
    static CombatSkillTree inst;
    return inst;
}

}

