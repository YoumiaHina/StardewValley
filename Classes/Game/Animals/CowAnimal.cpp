#include "Game/Animals/AnimalBase.h"

namespace Game {

// 奶牛动物静态行为：提供移动速度、游走半径与贴图路径等常量。
class CowAnimal : public AnimalBase {
public:
    // 返回动物类型枚举。
    AnimalType type() const override { return AnimalType::Cow; }
    // 返回基础移动速度。
    float moveSpeed() const override { return 30.0f; }
    // 返回游走半径（tile）。
    float wanderRadius() const override { return 3.0f; }
    // 返回贴图资源路径。
    const char* texturePath() const override { return "animal/cow.png"; }
};

// 获取奶牛行为单例：由系统侧通过 type 索引转发引用。
const AnimalBase& cowAnimalBehavior() {
    static CowAnimal inst;
    return inst;
}

}
