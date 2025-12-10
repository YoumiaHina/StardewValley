#include "Controllers/PlayerActionController.h"

namespace Controllers {

PlayerActionController* PlayerActionController::create() {
    PlayerActionController* node = new (std::nothrow) PlayerActionController();
    if (node && node->init()) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

bool PlayerActionController::init() {
    if (!Node::init()) return false;

    _bodySprite = cocos2d::Sprite::create("Farmer/farmer_base.png");
    _pantsSprite = cocos2d::Sprite::create("Farmer/pants.png");
    _shirtSprite = cocos2d::Sprite::create("Farmer/shirts.png");
    _hairSprite = cocos2d::Sprite::create("Farmer/hairstyles.png");
    _armSprite = cocos2d::Sprite::create("Farmer/farmer_base.png");

    if (!_bodySprite || !_pantsSprite || !_shirtSprite || !_hairSprite || !_armSprite) {
        cocos2d::log("Error loading character textures");
        return false;
    }

    this->addChild(_bodySprite, 0);
    this->addChild(_pantsSprite, 1);
    this->addChild(_shirtSprite, 2);
    this->addChild(_armSprite, 3);
    this->addChild(_hairSprite, 4);

    _bodySprite->setPosition(0, 0);
    _pantsSprite->setPosition(0, 0);
    _hairSprite->setPosition(0, -1);
    _armSprite->setPosition(0, 0);

    _shirtSprite->setPosition(0, -3);
    _shirtSprite->setScale(1.0f);
    _bodySprite->getTexture()->setAliasTexParameters();
    _pantsSprite->getTexture()->setAliasTexParameters();
    _shirtSprite->getTexture()->setAliasTexParameters();
    _hairSprite->getTexture()->setAliasTexParameters();
    _armSprite->getTexture()->setAliasTexParameters();

    updateSprites();
    return true;
}

void PlayerActionController::setShirtStyle(int index) {
    _shirtIndex = index;
    updateSprites();
}

void PlayerActionController::setPantsStyle(int index) {
    _pantsIndex = index;
    updateSprites();
}

void PlayerActionController::setHairStyle(int index) {
    _hairIndex = index;
    updateSprites();
}

void PlayerActionController::setHairColor(const cocos2d::Color3B& color) {
    _hairColor = color;
    if (_hairSprite) { _hairSprite->setColor(_hairColor); }
}

void PlayerActionController::setDirection(Direction dir) {
    if (_currentDir != dir) { _currentDir = dir; updateSprites(); }
}

void PlayerActionController::setMoving(bool moving) {
    if (_isMoving != moving) {
        _isMoving = moving;
        if (!_isMoving) { _animFrame = 0; updateSprites(); }
    }
}

void PlayerActionController::updateAnimation(float dt) {
    if (_isMoving) {
        _animTimer += dt;
        if (_animTimer > 0.15f) {
            _animTimer = 0;
            _animFrame = (_animFrame + 1) % 4;
            updateSprites();
        }
    } else if (_animFrame != 0) {
        _animFrame = 0;
        updateSprites();
    }
}

void PlayerActionController::updateSprites() {
    int dirRow = 0;
    switch (_currentDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break;
    }
    int walkFrames[] = {0, 1, 0, 2};
    int animCol = walkFrames[_animFrame];
    float bobY = (animCol == 1 || animCol == 2) ? -1.0f : 0.0f;
    bool useRightForLeft = (_currentDir == Direction::LEFT);
    Direction effectiveDir = useRightForLeft ? Direction::RIGHT : _currentDir;
    switch (effectiveDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break;
    }
    cocos2d::Rect bodyRect = getBodyRect(effectiveDir, _animFrame);
    _bodySprite->setTextureRect(bodyRect);
    _bodySprite->setFlippedX(useRightForLeft);
    int w = 16; int h = 32; int armCol = animCol + 6; float armX = armCol * w; float armY = dirRow * h;
    _armSprite->setTextureRect(cocos2d::Rect(armX, armY, w, h));
    _armSprite->setFlippedX(useRightForLeft);
    cocos2d::Rect pantsRect = getPantsRect(_pantsIndex, effectiveDir, _animFrame);
    _pantsSprite->setTextureRect(pantsRect);
    _pantsSprite->setFlippedX(useRightForLeft);
    cocos2d::Rect shirtRect = getShirtRect(_shirtIndex, effectiveDir, _animFrame);
    _shirtSprite->setTextureRect(shirtRect);
    _shirtSprite->setFlippedX(useRightForLeft);
    cocos2d::Rect hairRect = getHairRect(_hairIndex, effectiveDir, _animFrame);
    _hairSprite->setTextureRect(hairRect);
    _hairSprite->setFlippedX(useRightForLeft);
    _shirtSprite->setPosition(0, -3 + bobY);
    _hairSprite->setPosition(0, -1 + bobY);
    _armSprite->setPosition(0, 0);
    _pantsSprite->setPosition(0, 0);
}

cocos2d::Rect PlayerActionController::getBodyRect(Direction dir, int frame) {
    int w = 16; int h = 32; int row = (int)dir; int frames[] = {0, 1, 0, 2}; int col = frames[frame];
    float y = row * h; float x = col * w;
    return cocos2d::Rect(x, y, w, h);
}

cocos2d::Rect PlayerActionController::getPantsRect(int index, Direction dir, int frame) {
    int w = 16; int h = 32; int row = (int)dir; int styleRowOffset = index * 4; int frames[] = {0, 1, 0, 2}; int col = frames[frame];
    auto texSize = _pantsSprite->getTexture()->getContentSize(); int maxRows = texSize.height / h; if (styleRowOffset >= maxRows) styleRowOffset = 0;
    return cocos2d::Rect(col * w, (styleRowOffset + row) * h, w, h);
}

cocos2d::Rect PlayerActionController::getShirtRect(int index, Direction dir, int frame) {
    int w = 8; int h = 8; int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break;
        case Direction::RIGHT: blockIndex = 1; break;
        case Direction::LEFT: blockIndex = 2; break;
        case Direction::UP: blockIndex = 3; break;
    }
    auto texSize = _shirtSprite->getTexture()->getContentSize(); int cols = texSize.width / 8; int chunkCol = index % cols; int chunkRow = index / cols;
    float chunkX = chunkCol * 8; float chunkY = chunkRow * 32; float y = chunkY + (blockIndex * 8);
    return cocos2d::Rect(chunkX, y, w, h);
}

cocos2d::Rect PlayerActionController::getHairRect(int index, Direction dir, int frame) {
    int w = 16; int h = 32; auto texSize = _hairSprite->getTexture()->getContentSize(); int cols = texSize.width / 16; int chunkCol = index % cols; int chunkRow = index / cols;
    float chunkX = chunkCol * 16; float chunkY = chunkRow * 96; int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break;
        case Direction::RIGHT: blockIndex = 1; break;
        case Direction::LEFT: blockIndex = 1; break;
        case Direction::UP: blockIndex = 2; break;
    }
    float y = chunkY + (blockIndex * 32);
    return cocos2d::Rect(chunkX, y, w, h);
}

int PlayerActionController::getMaxHairStyles() { return 50; }
int PlayerActionController::getMaxShirtStyles() { return 50; }
int PlayerActionController::getMaxPantsStyles() { return 20; }

cocos2d::Size PlayerActionController::getTextureSize(const std::string& filename) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage(filename);
    if (tex) return tex->getContentSize();
    return cocos2d::Size::ZERO;
}

}

