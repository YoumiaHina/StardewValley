#ifndef __CHARACTER_NODE_H__
#define __CHARACTER_NODE_H__

#include "cocos2d.h"

namespace Game {

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

    // Customization setters
    void setShirtStyle(int index);
    void setPantsStyle(int index);
    void setHairStyle(int index);
    void setHairColor(const cocos2d::Color3B& color);
    
    // Animation control
    void setDirection(Direction dir);
    void setMoving(bool moving);
    void updateAnimation(float dt);

    // Getters for current state (for saving)
    int getShirtStyle() const { return _shirtIndex; }
    int getPantsStyle() const { return _pantsIndex; }
    int getHairStyle() const { return _hairIndex; }
    cocos2d::Color3B getHairColor() const { return _hairColor; }

    // Resource helpers
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
    int _animFrame = 0; // 0-3

    void updateSprites();
    cocos2d::Rect getBodyRect(Direction dir, int frame);
    cocos2d::Rect getShirtRect(int index, Direction dir, int frame);
    cocos2d::Rect getPantsRect(int index, Direction dir, int frame);
    cocos2d::Rect getHairRect(int index, Direction dir, int frame);
    
    // Helper to get texture size safely
    cocos2d::Size getTextureSize(const std::string& filename);
};

}

#endif // __CHARACTER_NODE_H__
