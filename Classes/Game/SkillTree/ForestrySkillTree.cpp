#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 砍树技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class ForestrySkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::Forestry; }
    // 返回显示名称。
    const char* displayName() const override { return "Forestry"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {301, "Extra Wood", 1},
            {302, "Sap Yield", 3},
            {303, "Durable Axe", 5},
            {304, "Rare Drop", 7},
        };
        return kNodes;
    }
};

// 获取砍树技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& forestrySkillTree() {
    static ForestrySkillTree inst;
    return inst;
}

}
