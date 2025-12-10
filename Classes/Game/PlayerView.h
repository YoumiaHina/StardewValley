#pragma once

#include "Game/IPlayerView.h"

namespace Game {

class PlayerView : public IPlayerView {
public:
    static PlayerView* create();
    bool init() override;

    void setShirtStyle(int index);
    void setPantsStyle(int index);
    void setHairStyle(int index);
    void setHairColor(const cocos2d::Color3B& color);

    void setDirection(Direction dir) override;
    void setMoving(bool moving) override;
    void updateAnimation(float dt) override;

    int getShirtStyle() const { return _shirtIndex; }
    int getPantsStyle() const { return _pantsIndex; }
    int getHairStyle() const { return _hairIndex; }
    cocos2d::Color3B getHairColor() const { return _hairColor; }

    static int getMaxHairStyles();
    static int getMaxShirtStyles();
    static int getMaxPantsStyles();

private:
    cocos2d::Sprite* _bodySprite = nullptr;
    cocos2d::Sprite* _shirtSprite = nullptr;
    cocos2d::Sprite* _pantsSprite = nullptr;
    cocos2d::Sprite* _hairSprite = nullptr;
    cocos2d::Sprite* _armSprite = nullptr;

    int _shirtIndex = 0;
    int _pantsIndex = 0;
    int _hairIndex = 0;
    cocos2d::Color3B _hairColor = cocos2d::Color3B::WHITE;

    Direction _currentDir = Direction::DOWN;
    bool _isMoving = false;
    float _animTimer = 0.0f;
    int _animFrame = 0;

    void updateSprites();
    cocos2d::Rect getBodyRect(Direction dir, int frame);
    cocos2d::Rect getShirtRect(int index, Direction dir, int frame);
    cocos2d::Rect getPantsRect(int index, Direction dir, int frame);
    cocos2d::Rect getHairRect(int index, Direction dir, int frame);
};

}

