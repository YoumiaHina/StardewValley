#pragma once

#include "Game/Item.h"
#include "Game/Crop.h"

namespace Game {

class VegetableBase {
public:
    virtual ~VegetableBase() = default;
    virtual ItemType item() const = 0;
    virtual CropType cropType() const = 0;
    virtual const char* name() const = 0;
    virtual int sellPrice() const = 0;
    virtual int energyRestore() const = 0;
    virtual int hpRestore() const = 0;
};

}
