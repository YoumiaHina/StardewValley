#pragma once

#include "Game/Item.h"
#include "Game/Crop.h"

namespace Game {

class SeedBase {
public:
    virtual ~SeedBase() = default;
    virtual ItemType item() const = 0;
    virtual CropType cropType() const = 0;
    virtual const char* name() const = 0;
    virtual int price() const = 0;
};

}
