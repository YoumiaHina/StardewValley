#include "Game/Animals/AnimalBase.h"

namespace Game {

class ChickenAnimal : public AnimalBase {
public:
    AnimalType type() const override { return AnimalType::Chicken; }
    float moveSpeed() const override { return 45.0f; }
    float wanderRadius() const override { return 2.5f; }
    const char* texturePath() const override { return "animal/chicken.png"; }
};

const AnimalBase& chickenAnimalBehavior() {
    static ChickenAnimal inst;
    return inst;
}

}

