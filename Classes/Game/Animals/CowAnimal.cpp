#include "Game/Animals/AnimalBase.h"

namespace Game {

class CowAnimal : public AnimalBase {
public:
    AnimalType type() const override { return AnimalType::Cow; }
    float moveSpeed() const override { return 30.0f; }
    float wanderRadius() const override { return 3.0f; }
    const char* texturePath() const override { return "animal/cow.png"; }
};

const AnimalBase& cowAnimalBehavior() {
    static CowAnimal inst;
    return inst;
}

}

