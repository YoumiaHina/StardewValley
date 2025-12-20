#include "Game/SkillTree/SkillTreeBase.h"

namespace Game {

// 养殖技能树静态定义：提供节点列表与显示名等常量，不持有运行时进度。
class AnimalHusbandrySkillTree : public SkillTreeBase {
public:
    // 返回技能树类型。
    SkillTreeType type() const override { return SkillTreeType::AnimalHusbandry; }
    // 返回显示名称。
    const char* displayName() const override { return "养殖"; }
    // 返回节点列表（引用长期有效）。
    const std::vector<SkillNode>& nodes() const override {
        static const std::vector<SkillNode> kNodes = {
            {201, "快速成长", 1},
            {202, "双倍产物", 4},
            {203, "节省饲料", 6},
            {204, "亲和驯养", 8},
        };
        return kNodes;
    }
};

// 获取养殖技能树单例：由上层通过 skillTreeByType 或直接引用使用。
const SkillTreeBase& animalHusbandrySkillTree() {
    static AnimalHusbandrySkillTree inst;
    return inst;
}

}

