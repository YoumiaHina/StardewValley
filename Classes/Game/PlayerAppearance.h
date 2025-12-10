#ifndef __CHARACTER_NODE_H__
#define __CHARACTER_NODE_H__

#include "cocos2d.h"

namespace Game {

// 玩家外观节点：组合身体、裤子、上衣、头发与手臂，并驱动行走动画
class PlayerAppearance : public cocos2d::Node {
public:
    enum class Direction {
        DOWN = 0,
        RIGHT = 1,
        UP = 2,
        LEFT = 3
    };

    static PlayerAppearance* create();
    virtual bool init() override;

    // 外观定制接口
    void setShirtStyle(int index);
    void setPantsStyle(int index);
    void setHairStyle(int index);
    void setHairColor(const cocos2d::Color3B& color);
    
    // 动画控制
    void setDirection(Direction dir);
    void setMoving(bool moving);
    void updateAnimation(float dt);

    // 状态读取（用于存档）
    int getShirtStyle() const { return _shirtIndex; }
    int getPantsStyle() const { return _pantsIndex; }
    int getHairStyle() const { return _hairIndex; }
    cocos2d::Color3B getHairColor() const { return _hairColor; }

    // 资源辅助
    static int getMaxHairStyles();
    static int getMaxShirtStyles();
    static int getMaxPantsStyles();

private:
    cocos2d::Sprite* _bodySprite;
    cocos2d::Sprite* _shirtSprite;
    cocos2d::Sprite* _pantsSprite;
    cocos2d::Sprite* _hairSprite;
    cocos2d::Sprite* _armSprite;

    int _shirtIndex = 0;
    int _pantsIndex = 0;
    int _hairIndex = 0;
    cocos2d::Color3B _hairColor = cocos2d::Color3B::WHITE;

    Direction _currentDir = Direction::DOWN;
    bool _isMoving = false;
    float _animTimer = 0.0f;
    int _animFrame = 0; // 0–3 帧索引

    void updateSprites();
    cocos2d::Rect getBodyRect(Direction dir, int frame);
    cocos2d::Rect getShirtRect(int index, Direction dir, int frame);
    cocos2d::Rect getPantsRect(int index, Direction dir, int frame);
    cocos2d::Rect getHairRect(int index, Direction dir, int frame);
    
    // 安全获取贴图尺寸
    cocos2d::Size getTextureSize(const std::string& filename);
};

}

#endif // 头文件保护宏
