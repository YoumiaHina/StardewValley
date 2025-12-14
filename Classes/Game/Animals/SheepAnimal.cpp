#include "Game/Animals/AnimalBase.h"

namespace Game {

class SheepAnimal : public AnimalBase {
public:
    AnimalType type() const override { return AnimalType::Sheep; }
    float moveSpeed() const override { return 35.0f; }
    float wanderRadius() const override { return 2.8f; }
    const char* texturePath() const override { return "animal/sheep.png"; }
};

const AnimalBase& sheepAnimalBehavior() {
    static SheepAnimal inst;
    return inst;
}

}

