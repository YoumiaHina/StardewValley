#pragma once

#include "cocos2d.h"
#include "Game/Crop.h"

namespace Game {

inline int cropBaseRow(CropType t) {
    switch (t) {
        case CropType::Parsnip: return 1;
        case CropType::Blueberry: return 23;
        case CropType::Eggplant: return 9;
        default: return 0;
    }
}

inline int cropStartCol(CropType t) {
    switch (t) {
        case CropType::Parsnip: return 8;
        case CropType::Blueberry: return 8;
        case CropType::Eggplant: return 0;
        default: return 0;
    }
}

inline cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight) {
    int row = cropBaseRow(t);
    int maxStage = cropMaxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = cropStartCol(t) + s;
    float yTop = textureHeight - (row + 1) * 32.0f;
    return cocos2d::Rect(col * 16.0f, yTop + 16.0f, 16.0f, 16.0f);
}

inline cocos2d::Rect cropRectTopHalf(CropType t, int stage, float textureHeight) {
    int row = cropBaseRow(t);
    int maxStage = cropMaxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = cropStartCol(t) + s;
    float yTop = textureHeight - (row + 1) * 32.0f;
    return cocos2d::Rect(col * 16.0f, yTop, 16.0f, 16.0f);
}

}
