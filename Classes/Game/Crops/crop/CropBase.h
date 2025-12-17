#pragma once

#include "cocos2d.h"
#include <array>
#include <vector>
#include "Game/Item.h"

namespace Game {

enum class CropType { Parsnip, Blueberry, Eggplant, Corn, Strawberry };

struct Crop {
    int c = 0;
    int r = 0;
    CropType type = CropType::Parsnip;
    int stage = 0;
    int progress = 0;
    int maxStage = 0;
    bool wateredToday = false;
};

struct CropDef {
    int baseRow16 = 0;
    int startCol = 0;
    std::vector<int> stageDays;
    std::array<bool, 4> seasons;
};

class CropDefs {
public:
    static const CropDef& get(CropType t);
    static const std::vector<int>& stageDays(CropType t);
    static int maxStage(CropType t);
    static int startCol(CropType t);
    static int baseRow16(CropType t);
    static bool isSeasonAllowed(CropType t, int seasonIndex);
    static bool isRegrow(CropType t);
};

ItemType seedItemFor(CropType t);
ItemType produceItemFor(CropType t);
bool isSeed(ItemType t);
CropType cropTypeFromSeed(ItemType t);

cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight);
cocos2d::Rect cropRectTopHalf(CropType t, int stage, float textureHeight);

class CropBase {
public:
    virtual ~CropBase() = default;

    virtual CropType cropType() const = 0;

    CropDef def_{};
    ItemType seedItem_ = ItemType::ParsnipSeed;
    ItemType produceItem_ = ItemType::Parsnip;
    bool regrow_ = false;

    const CropDef& def() const { return def_; }
    ItemType seedItem() const { return seedItem_; }
    ItemType produceItem() const { return produceItem_; }
    bool regrow() const { return regrow_; }
};

}
