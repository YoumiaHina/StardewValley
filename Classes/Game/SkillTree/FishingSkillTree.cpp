#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 钓鱼技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class FishingSkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::Fishing; }
    // 返回显示名称。
    const char* displayName() const override { return "钓鱼"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {401, "抛竿距离", 1},
            {402, "咬钩率", 3},
            {403, "收线速度", 5},
            {404, "宝箱概率", 7},
        };
        return kNodes;
    }
};

// 获取钓鱼技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& fishingSkillTree() {
    static FishingSkillTree inst;
    return inst;
}

}

