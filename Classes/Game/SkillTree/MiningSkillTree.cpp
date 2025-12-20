#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 挖矿技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class MiningSkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::Mining; }
    // 返回显示名称。
    const char* displayName() const override { return "Mining"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {501, "Extra Ore", 1},
            {502, "Geologist", 3},
            {503, "Durable Pickaxe", 5},
            {504, "Rare Finds", 7},
        };
        return kNodes;
    }
};

// 获取挖矿技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& miningSkillTree() {
    static MiningSkillTree inst;
    return inst;
}

}

