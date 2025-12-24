#pragma once

#include "Game/View/IPlayerView.h"

namespace Game {

// 玩家视图实现：
// - 作用：用多层 Sprite 组合角色（身体/衣服/裤子/头发/手臂/工具），并根据朝向/移动/工具动作更新贴图。
// - 职责边界：仅负责渲染与动画推进；角色位置/速度/交互等由控制器负责。
// - 主要协作对象：通过 IPlayerView 接口被 PlayerController/Scene 驱动；外观参数通常由 UI/存档系统提供。
class PlayerView : public IPlayerView {
public:
    // 工厂创建：创建并返回 PlayerView 实例。
    static PlayerView* create();
    // 初始化：构建各层 Sprite 与默认状态。
    bool init() override;

    // 设置上衣样式索引并刷新贴图。
    void setShirtStyle(int index);
    // 设置裤子样式索引并刷新贴图。
    void setPantsStyle(int index);
    // 设置发型样式索引并刷新贴图。
    void setHairStyle(int index);
    // 设置头发颜色并刷新贴图。
    void setHairColor(const cocos2d::Color3B& color);

    // 设置朝向（影响行走/工具动作贴图选择）。
    void setDirection(Direction dir) override;
    // 设置是否移动（决定站立/行走动画）。
    void setMoving(bool moving) override;
    // 推进并刷新行走与工具动画（由外部按帧调用）。
    void updateAnimation(float dt) override;
    // 播放工具动作动画（根据工具种类与等级选择帧序列）。
    void playToolAnimation(Game::ToolKind kind, int level) override;

    // 获取当前上衣样式索引。
    int getShirtStyle() const { return _shirtIndex; }
    // 获取当前裤子样式索引。
    int getPantsStyle() const { return _pantsIndex; }
    // 获取当前发型样式索引。
    int getHairStyle() const { return _hairIndex; }
    // 获取当前头发颜色。
    cocos2d::Color3B getHairColor() const { return _hairColor; }

    // 获取支持的最大发型样式数量。
    static int getMaxHairStyles();
    // 获取支持的最大上衣样式数量。
    static int getMaxShirtStyles();
    // 获取支持的最大裤子样式数量。
    static int getMaxPantsStyles();

    // 获取用于上层渲染（遮挡关系）的节点。
    cocos2d::Node* upperNode() const { return _upperNode; }
    // 同步上层节点位置（用于与本体节点保持一致）。
    void syncUpperLayerPosition();
    // 同步上层节点 Z 序（用于与遮挡逻辑保持一致）。
    void syncUpperLayerZ();

private:
    cocos2d::Sprite* _bodySprite = nullptr;
    cocos2d::Sprite* _shirtSprite = nullptr;
    cocos2d::Sprite* _pantsSprite = nullptr;
    cocos2d::Sprite* _armSprite = nullptr;
    cocos2d::Node* _upperNode = nullptr;
    cocos2d::Sprite* _bodySpriteUpper = nullptr;
    cocos2d::Sprite* _shirtSpriteUpper = nullptr;
    cocos2d::Sprite* _pantsSpriteUpper = nullptr;
    cocos2d::Sprite* _hairSprite = nullptr;
    cocos2d::Sprite* _armSpriteUpper = nullptr;

    int _shirtIndex = 0;
    int _pantsIndex = 0;
    int _hairIndex = 0;
    cocos2d::Color3B _hairColor = cocos2d::Color3B::WHITE;

    Direction _currentDir = Direction::DOWN;
    bool _isMoving = false;
    float _animTimer = 0.0f;
    int _animFrame = 0;

    cocos2d::Sprite* _toolSprite = nullptr;
    bool _isUsingTool = false;
    Game::ToolKind _currentTool = Game::ToolKind::Axe;
    float _toolAnimTimer = 0.0f;
    int _toolAnimFrame = 0;

    // 按当前外观参数与动画帧刷新所有 Sprite 的贴图与可见性。
    void updateSprites();
    // 计算身体层在指定朝向与帧下的贴图矩形。
    cocos2d::Rect getBodyRect(Direction dir, int frame);
    // 计算上衣层在指定样式/朝向/帧下的贴图矩形。
    cocos2d::Rect getShirtRect(int index, Direction dir, int frame);
    // 计算裤子层在指定样式/朝向/帧下的贴图矩形。
    cocos2d::Rect getPantsRect(int index, Direction dir, int frame);
    // 计算头发层在指定样式/朝向/帧下的贴图矩形。
    cocos2d::Rect getHairRect(int index, Direction dir, int frame);
    // 根据当前工具动作帧刷新工具 Sprite 的贴图与位置。
    void updateToolSprite();
    // 获取指定朝向下工具动作的总帧数。
    int toolFramesForDirection(Direction dir) const;
    // 获取指定朝向下指定帧对应的工具贴图列索引。
    int toolColumnFor(Direction dir, int frame) const;
};
}
