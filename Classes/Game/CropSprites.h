#pragma once

#include "cocos2d.h"
#include "Game/Crop.h"

namespace Game {

// 计算作物下半块纹理切片矩形（按16像素为单位；baseRow16 为自底向上的行索引）
inline cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (bottomRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

// 计算作物上半块纹理切片矩形（上半块位于下半块之上一个16像素行）
inline cocos2d::Rect cropRectTopHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int topRow16 = bottomRow16 + 1;
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (topRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

}
