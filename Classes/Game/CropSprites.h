// 作物贴图裁切工具：
// - 统一以 16px 为基本单位，从整张纹理中裁切“下半块”和“上半块”
// - 纹理坐标系以左下角为(0,0)；baseRow16 为自底向上的行索引
// - 某些作物贴图排布有差异（如蓝莓），其 baseRow16/startCol 由 CropDefs 提供
// - 若出现白线锯齿，建议对纹理使用邻近点采样：texture->setAliasTexParameters()
#pragma once

#include "cocos2d.h"
#include "Game/Crop.h"

namespace Game {

// 计算作物下半块纹理切片矩形（按16像素为单位）
inline cocos2d::Rect cropRectBottomHalf(CropType t, int stage, float textureHeight) {
    int bottomRow16 = CropDefs::baseRow16(t);
    int maxStage = CropDefs::maxStage(t);
    int s = std::max(0, std::min(stage, maxStage));
    int col = CropDefs::startCol(t) + s;
    float y = textureHeight - (bottomRow16 + 1) * 16.0f;
    return cocos2d::Rect(col * 16.0f, y, 16.0f, 16.0f);
}

// 计算作物上半块纹理切片矩形（位于下半块之上一个16像素行）
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
