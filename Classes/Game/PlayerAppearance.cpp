#include "PlayerAppearance.h"

namespace Game {

PlayerAppearance* PlayerAppearance::create() {
    PlayerAppearance* node = new (std::nothrow) PlayerAppearance();
    if (node && node->init()) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

bool PlayerAppearance::init() {
    if (!Node::init()) return false;

    // 初始化各部件精灵；渲染顺序为：身体、裤子、上衣、手臂、头发
    
    _bodySprite = cocos2d::Sprite::create("Farmer/farmer_base.png");
    _pantsSprite = cocos2d::Sprite::create("Farmer/pants.png");
    _shirtSprite = cocos2d::Sprite::create("Farmer/shirts.png");
    _hairSprite = cocos2d::Sprite::create("Farmer/hairstyles.png");
    _armSprite = cocos2d::Sprite::create("Farmer/farmer_base.png");
    
    // 资源加载校验
    if (!_bodySprite || !_pantsSprite || !_shirtSprite || !_hairSprite || !_armSprite) {
        cocos2d::log("Error loading character textures");
        return false;
    }

    this->addChild(_bodySprite, 0);
    this->addChild(_pantsSprite, 1);
    this->addChild(_shirtSprite, 2);
    this->addChild(_armSprite, 3); // 手臂层级在上衣之上
    this->addChild(_hairSprite, 4);

    // 对齐到局部坐标原点
    _bodySprite->setPosition(0, 0);
    _pantsSprite->setPosition(0, 0);
    _hairSprite->setPosition(0, -1); 
    _armSprite->setPosition(0, 0);
    
    // 上衣素材按 8x8，小幅 Y 偏移以贴合躯干
    _shirtSprite->setPosition(0, -3); // 上衣躯干偏移
    _shirtSprite->setScale(1.0f); 
    _bodySprite->getTexture()->setAliasTexParameters();
    _pantsSprite->getTexture()->setAliasTexParameters();
    _shirtSprite->getTexture()->setAliasTexParameters();
    _hairSprite->getTexture()->setAliasTexParameters();
    _armSprite->getTexture()->setAliasTexParameters();

    // 应用初始外观
    updateSprites();

    return true;
}

void PlayerAppearance::setShirtStyle(int index) {
    _shirtIndex = index;
    updateSprites();
}

void PlayerAppearance::setPantsStyle(int index) {
    _pantsIndex = index;
    updateSprites();
}

void PlayerAppearance::setHairStyle(int index) {
    _hairIndex = index;
    updateSprites();
}

void PlayerAppearance::setHairColor(const cocos2d::Color3B& color) {
    _hairColor = color;
    if (_hairSprite) {
        _hairSprite->setColor(_hairColor);
    }
}

void PlayerAppearance::setDirection(Direction dir) {
    if (_currentDir != dir) {
        _currentDir = dir;
        updateSprites();
    }
}

void PlayerAppearance::setMoving(bool moving) {
    if (_isMoving != moving) {
        _isMoving = moving;
        if (!_isMoving) {
            _animFrame = 0; // 重置为站立帧
            updateSprites();
        }
    }
}

void PlayerAppearance::updateAnimation(float dt) {
    if (_isMoving) {
        _animTimer += dt;
        if (_animTimer > 0.15f) { // 动画节拍（秒）
            _animTimer = 0;
            _animFrame = (_animFrame + 1) % 4; // 四帧循环：站立、右脚、站立、左脚
            updateSprites();
        }
    } else if (_animFrame != 0) {
        // 停止移动后确保回归站立帧（防漏）
        _animFrame = 0;
        updateSprites();
    }
}

// 计算索引在贴图网格中的矩形区域（0 基）
// 参数：index 索引；w,h 子图尺寸；texW 贴图宽度
static cocos2d::Rect calculateRect(int index, int w, int h, int texW) {
    int cols = texW / w;
    if (cols == 0) cols = 1;
    int c = index % cols;
    int r = index / cols;
    return cocos2d::Rect(c * w, r * h, w, h);
}

void PlayerAppearance::updateSprites() {
    // 身体贴图约定：尺寸 16x32；行=朝向（下/右/上/左），列=站立/迈右脚/站立/迈左脚
    
    int dirRow = 0;
    switch (_currentDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break;
    }
    
    int animCol = 0;
    int walkFrames[] = {0, 1, 0, 2}; // 通用行走帧模式
    animCol = walkFrames[_animFrame];
    
    // 步态下沉：迈步帧（1/2）向下偏移
    float bobY = (animCol == 1 || animCol == 2) ? -1.0f : 0.0f;

    // 左向时统一使用右向帧并水平翻转，确保各素材对齐与表现一致
    bool useRightForLeft = (_currentDir == Direction::LEFT);
    Direction effectiveDir = _currentDir;
    if (useRightForLeft) {
        effectiveDir = Direction::RIGHT;
    }

    // 根据有效朝向重算行号
    switch (effectiveDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break;
    }

    cocos2d::Rect bodyRect = getBodyRect(effectiveDir, _animFrame);
    _bodySprite->setTextureRect(bodyRect);
    _bodySprite->setFlippedX(useRightForLeft);
    
    // 手臂帧位于基础贴图第 7–12 列（索引 6–11），随身体帧同步
    int w = 16;
    int h = 32;
    int armCol = animCol + 6;
    float armX = armCol * w;
    float armY = dirRow * h;
    _armSprite->setTextureRect(cocos2d::Rect(armX, armY, w, h));
    _armSprite->setFlippedX(useRightForLeft);
    
    // 裤子
    cocos2d::Rect pantsRect = getPantsRect(_pantsIndex, effectiveDir, _animFrame);
    _pantsSprite->setTextureRect(pantsRect);
    _pantsSprite->setFlippedX(useRightForLeft);
    
    // 上衣：左向使用右向块翻转以统一表现
    cocos2d::Rect shirtRect = getShirtRect(_shirtIndex, effectiveDir, _animFrame);
    _shirtSprite->setTextureRect(shirtRect);
    _shirtSprite->setFlippedX(useRightForLeft);
    
    // 头发：右/左共用侧面块，左向时水平翻转
    cocos2d::Rect hairRect = getHairRect(_hairIndex, effectiveDir, _animFrame);
    _hairSprite->setTextureRect(hairRect);
    _hairSprite->setFlippedX(useRightForLeft);
    
    // 将步态下沉应用到附着件
    _shirtSprite->setPosition(0, -3 + bobY);
    _hairSprite->setPosition(0, -1 + bobY);
    
    // 手臂与身体齐平
    _armSprite->setPosition(0, 0); 
    
    // 裤子位置
    _pantsSprite->setPosition(0, 0);
}

cocos2d::Rect PlayerAppearance::getBodyRect(Direction dir, int frame) {
    int w = 16;
    int h = 32;
    int row = (int)dir; // 行索引 0–3
    
    // 行走帧序列：{0,1,0,2}，与身体贴图列对应
    int frames[] = {0, 1, 0, 2}; 
    int col = frames[frame];
    
    // 按行计算 Y 偏移
    float y = row * h;
    float x = col * w;
    
    return cocos2d::Rect(x, y, w, h);
}

cocos2d::Rect PlayerAppearance::getPantsRect(int index, Direction dir, int frame) {
    // 裤子贴图布局：每个样式占 4 行（下/右/上/左），每行 16x32；帧序列与身体一致
    int w = 16;
    int h = 32;
    int row = (int)dir; 
    
    // 样式起始行偏移（每样式 4 行）
    int styleRowOffset = index * 4;
    
    int frames[] = {0, 1, 0, 2};
    int col = frames[frame];
    
    // 依据贴图高度计算可用总行数
    auto texSize = _pantsSprite->getTexture()->getContentSize();
    int maxRows = texSize.height / h;
    
    // 索引越界时回绕至首样式
    if (styleRowOffset >= maxRows) styleRowOffset = 0;
    
    return cocos2d::Rect(col * w, (styleRowOffset + row) * h, w, h);
}

cocos2d::Rect PlayerAppearance::getShirtRect(int index, Direction dir, int frame) {
    // 上衣贴图：以 8x32 为一个块，块内按 8x8 小图依次为 正面/右侧/左侧/背面（下/右/左/上）
    
    int w = 8;
    int h = 8;
    
    // 将朝向映射到块内索引（0–3）
    int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break; // 正面
        case Direction::RIGHT: blockIndex = 1; break; // 右侧
        case Direction::LEFT: blockIndex = 2; break; // 左侧
        case Direction::UP: blockIndex = 3; break; // 背面
    }
    
    // 定位样式所在 8x32 块
    auto texSize = _shirtSprite->getTexture()->getContentSize();
    int cols = texSize.width / 8;
    int chunkCol = index % cols;
    int chunkRow = index / cols;
    
    float chunkX = chunkCol * 8;
    float chunkY = chunkRow * 32;
    
    float y = chunkY + (blockIndex * 8);
    
    return cocos2d::Rect(chunkX, y, w, h);
}

cocos2d::Rect PlayerAppearance::getHairRect(int index, Direction dir, int frame) {
    // 头发贴图：以 16x96 为一个块，块内按 16x32 小图依次为 正面/侧面/背面（下/右左/上）
    
    int w = 16;
    int h = 32;
    
    // 贴图宽度决定每行可容纳的样式块数量
    auto texSize = _hairSprite->getTexture()->getContentSize();
    int cols = texSize.width / 16; // 每行样式块数
    
    int chunkCol = index % cols;
    int chunkRow = index / cols;
    
    float chunkX = chunkCol * 16;
    float chunkY = chunkRow * 96;
    
    int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break; // 正面
        case Direction::RIGHT: blockIndex = 1; break; // 侧面
        case Direction::LEFT: blockIndex = 1; break; // 侧面（与右侧共用，左向时翻转）
        case Direction::UP: blockIndex = 2; break; // 背面
    }
    
    float y = chunkY + (blockIndex * 32);
    
    return cocos2d::Rect(chunkX, y, w, h);
}

int PlayerAppearance::getMaxHairStyles() { return 50; } // 近似值
int PlayerAppearance::getMaxShirtStyles() { return 50; }
int PlayerAppearance::getMaxPantsStyles() { return 20; }

cocos2d::Size PlayerAppearance::getTextureSize(const std::string& filename) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage(filename);
    if (tex) return tex->getContentSize();
    return cocos2d::Size::ZERO;
}

}
