#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 种地技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class FarmingSkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::Farming; }
    // 返回显示名称。
    const char* displayName() const override { return "Farming"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {101, "Precision Watering", 1},
            {102, "Bountiful Harvest", 3},
            {103, "Efficient Tilling", 5},
            {104, "Greenhouse Expert", 7},
        };
        return kNodes;
    }
};

// 获取种地技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& farmingSkillTree() {
    static FarmingSkillTree inst;
    return inst;
}

}
