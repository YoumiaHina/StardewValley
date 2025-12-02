#pragma once

#include <vector>
#include "Game/Item.h"

namespace Game {

enum class CropType { Parsnip };

struct Crop {
    int c = 0;
    int r = 0;
    CropType type = CropType::Parsnip;
    int stage = 0;
    int progress = 0;
    int maxStage = 0;
};

inline std::vector<int> cropStageDays(CropType t) {
    switch (t) {
        case CropType::Parsnip: return {1,1,1,1};
        default: return {1,1,1};
    }
}

inline int cropMaxStage(CropType t) {
    auto v = cropStageDays(t);
    return static_cast<int>(v.size()) - 1;
}

inline Game::ItemType seedItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return Game::ItemType::ParsnipSeed;
        default: return Game::ItemType::ParsnipSeed;
    }
}

inline Game::ItemType produceItemFor(CropType t) {
    switch (t) {
        case CropType::Parsnip: return Game::ItemType::Parsnip;
        default: return Game::ItemType::Parsnip;
    }
}

inline bool isSeed(Game::ItemType t) {
    return t == Game::ItemType::ParsnipSeed;
}

inline CropType cropTypeFromSeed(Game::ItemType t) {
    switch (t) {
        case Game::ItemType::ParsnipSeed: return CropType::Parsnip;
        default: return CropType::Parsnip;
    }
}

}
