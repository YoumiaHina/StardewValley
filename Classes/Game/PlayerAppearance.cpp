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

    // Initialize sprites
    // Order: Body -> Pants -> Shirt -> Hair -> Arms (Arms handling is complex, let's stick to basic first)
    // Actually SV render order: Body, Pants, Shirt, Arms, Hair.
    
    _bodySprite = cocos2d::Sprite::create("Farmer/farmer_base.png");
    _pantsSprite = cocos2d::Sprite::create("Farmer/pants.png");
    _shirtSprite = cocos2d::Sprite::create("Farmer/shirts.png");
    _hairSprite = cocos2d::Sprite::create("Farmer/hairstyles.png");
    _armSprite = cocos2d::Sprite::create("Farmer/farmer_base.png");
    
    // Check if sprites loaded
    if (!_bodySprite || !_pantsSprite || !_shirtSprite || !_hairSprite || !_armSprite) {
        cocos2d::log("Error loading character textures");
        return false;
    }

    this->addChild(_bodySprite, 0);
    this->addChild(_pantsSprite, 1);
    this->addChild(_shirtSprite, 2);
    this->addChild(_armSprite, 3); // Arms above shirt
    this->addChild(_hairSprite, 4);

    // Center sprites
    _bodySprite->setPosition(0, 0);
    _pantsSprite->setPosition(0, 0);
    _hairSprite->setPosition(0, -1); 
    _armSprite->setPosition(0, 0);
    
    // Shirt is 8x8, Body is 16x32. Need to center shirt.
    _shirtSprite->setPosition(0, -3); // Offset for torso
    _shirtSprite->setScale(1.0f); 
    _bodySprite->getTexture()->setAliasTexParameters();
    _pantsSprite->getTexture()->setAliasTexParameters();
    _shirtSprite->getTexture()->setAliasTexParameters();
    _hairSprite->getTexture()->setAliasTexParameters();
    _armSprite->getTexture()->setAliasTexParameters();

    // Initialize defaults
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
            _animFrame = 0; // Reset to standing
            updateSprites();
        }
    }
}

void PlayerAppearance::updateAnimation(float dt) {
    if (_isMoving) {
        _animTimer += dt;
        if (_animTimer > 0.15f) { // Animation speed
            _animTimer = 0;
            _animFrame = (_animFrame + 1) % 4; // 4 frames: Stand, R, Stand, L usually
            updateSprites();
        }
    } else if (_animFrame != 0) {
        // Ensure we reset if we stopped but update loop didn't catch it
        _animFrame = 0;
        updateSprites();
    }
}

// Helper to calculate grid position
// Assumes 0-based index
// w, h: sprite size
// texW: texture width
static cocos2d::Rect calculateRect(int index, int w, int h, int texW) {
    int cols = texW / w;
    if (cols == 0) cols = 1;
    int c = index % cols;
    int r = index / cols;
    return cocos2d::Rect(c * w, r * h, w, h);
}

void PlayerAppearance::updateSprites() {
    // Body
    // 16x32, Rows: Down, Right, Up, Left
    // Cols: 0=Stand, 1=RightFoot, 2=Stand, 3=LeftFoot (Standard SV)
    
    int dirRow = 0;
    switch (_currentDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break;
    }
    
    int animCol = 0;
    int walkFrames[] = {0, 1, 0, 2}; // Common walk cycle pattern
    animCol = walkFrames[_animFrame];
    
    // Bobbing: Shift down on step frames (1 and 2)
    float bobY = (animCol == 1 || animCol == 2) ? -1.0f : 0.0f;

    // Flip sprites if facing left (Direction::LEFT is 3)
    // BUT: Texture for LEFT (row 3) exists for Body, Pants, Arms.
    // Shirt: Has LEFT block.
    // Hair: Uses SIDE block (shared for Right/Left).
    // So:
    // Body, Pants, Arms: Use Row 3. No flip needed usually if texture is correct.
    // Shirt: Use Block 2 (Left). No flip needed.
    // Hair: Uses Block 1 (Side). Needs flip if Left.
    
    bool flipHair = (_currentDir == Direction::LEFT);
    
    // User Requirement: "When moving left, reverse the action of moving right".
    // This implies the current LEFT animation might be wrong or the user wants to reuse RIGHT sprites flipped.
    // SV standard: Row 3 IS Left. 
    // If user says "Left action has problem, use Right reversed":
    // We should map LEFT direction to RIGHT row/block, and set setFlippedX(true).
    
    bool useRightForLeft = (_currentDir == Direction::LEFT);
    Direction effectiveDir = _currentDir;
    if (useRightForLeft) {
        effectiveDir = Direction::RIGHT;
    }

    // Re-calculate rows based on effectiveDir
    switch (effectiveDir) {
        case Direction::DOWN: dirRow = 0; break;
        case Direction::RIGHT: dirRow = 1; break;
        case Direction::UP: dirRow = 2; break;
        case Direction::LEFT: dirRow = 3; break; // Should not happen if useRightForLeft is true
    }

    cocos2d::Rect bodyRect = getBodyRect(effectiveDir, _animFrame);
    _bodySprite->setTextureRect(bodyRect);
    _bodySprite->setFlippedX(useRightForLeft);
    
    // Arm (Cols 7-12 -> Indices 6-11)
    // armCol = animCol + 6
    int w = 16;
    int h = 32;
    int armCol = animCol + 6;
    float armX = armCol * w;
    float armY = dirRow * h;
    _armSprite->setTextureRect(cocos2d::Rect(armX, armY, w, h));
    _armSprite->setFlippedX(useRightForLeft);
    
    // Pants
    cocos2d::Rect pantsRect = getPantsRect(_pantsIndex, effectiveDir, _animFrame);
    _pantsSprite->setTextureRect(pantsRect);
    _pantsSprite->setFlippedX(useRightForLeft);
    
    // Shirt
    // Shirt has specific Left block (Index 2).
    // If we use Right (Index 1) flipped, it might look better if original Left was bad.
    // User said: "When moving left, just reverse right action".
    // So we use Shirt's RIGHT block and flip it.
    cocos2d::Rect shirtRect = getShirtRect(_shirtIndex, effectiveDir, _animFrame);
    _shirtSprite->setTextureRect(shirtRect);
    _shirtSprite->setFlippedX(useRightForLeft);
    
    // Hair
    // Hair uses Side block (Index 1) for both Right and Left.
    // So we just need to flip it if Left.
    // getHairRect handles index mapping, but we need to ensure we ask for RIGHT (Side) if we are Left.
    cocos2d::Rect hairRect = getHairRect(_hairIndex, effectiveDir, _animFrame);
    _hairSprite->setTextureRect(hairRect);
    _hairSprite->setFlippedX(useRightForLeft);
    
    // Apply Bobbing to attachments
    _shirtSprite->setPosition(0, -3 + bobY);
    _hairSprite->setPosition(0, -1 + bobY);
    
    // Arms: In farmer_base, arms are aligned with body. 
    _armSprite->setPosition(0, 0); 
    
    // Pants
    _pantsSprite->setPosition(0, 0);
}

cocos2d::Rect PlayerAppearance::getBodyRect(Direction dir, int frame) {
    int w = 16;
    int h = 32;
    int row = (int)dir; // 0, 1, 2, 3
    
    // User said: "Each row's first 6 are body". 
    // Let's assume the texture width is large enough.
    // Walk cycle: 0 (Stand), 1 (Right), 0 (Stand), 2 (Left) ?
    // Let's use 1, 2, 3, 4... 
    // Standard SV: 0=Stand, 1=Walk1, 2=Walk2... 
    // Let's try {0, 1, 0, 2} indices within the row.
    int frames[] = {0, 1, 0, 2}; 
    int col = frames[frame];
    
    // Calculate y based on row
    float y = row * h;
    float x = col * w;
    
    return cocos2d::Rect(x, y, w, h);
}

cocos2d::Rect PlayerAppearance::getPantsRect(int index, Direction dir, int frame) {
    // User: "pants can be divided into 16*32 blocks".
    // This implies styles are separate blocks?
    // Or styles are rows?
    // Standard SV `pants.png`: grid of 16x32. 
    // Rows = colors? Cols = Frames?
    // Actually standard `pants.png` is 10 columns x 1 row (just frames).
    // User says "16*32 blocks".
    // Let's assume: Index selects a STYLE. A style has directions.
    // Maybe: Rows = Styles. Cols = Direction/Frame?
    // Or: Rows = Directions. Cols = Styles?
    
    // Simplest assumption for "Choice":
    // The image contains multiple styles.
    // Let's assume a grid where each style is a 16x32 block?
    // But then no animation.
    
    // Let's assume the pants sheet matches the body sheet layout for animation.
    // i.e., Index 0 = Style 1 (occupies 4 rows of animation).
    // Index 1 = Style 2.
    // This would be a very tall image.
    
    // Alternative: Pants are just color variations of a single base?
    // User says "Free choice of pants style".
    
    // Let's try: Pants sheet is a grid of 16x32 sprites.
    // 1 row per direction?
    // Let's stick to: Rows 0-3 = Down, Right, Up, Left.
    // Columns = Styles?
    // If I select style `index`, I use column `index`?
    // But then where is animation?
    // Maybe (Style * FrameCount) + Frame?
    
    // Let's look at file size: `pants.png` is 455KB. `farmer_base.png` is 81KB.
    // Pants is HUGE.
    // Likely: Grid of styles.
    // Standard SV `pants.png` is actually 1 row of many frames? No.
    
    // Let's assume standard SV mod format:
    // Many mods use `pants.png` as a 10x grid?
    
    // Let's use a generic grid logic:
    // Index maps to a "start" coordinate.
    // We need 4 directions x 4 frames = 16 sprites per style?
    // Or just 1 sprite per style (static legs)? No, "movement".
    
    // Let's assume:
    // Pants texture layout matches Body texture layout.
    // But there are multiple styles.
    // So maybe Style 0 is Rows 0-3. Style 1 is Rows 4-7.
    
    int w = 16;
    int h = 32;
    int row = (int)dir; 
    
    // Style offset
    int styleRowOffset = index * 4; // 4 rows per style?
    
    int frames[] = {0, 1, 0, 2};
    int col = frames[frame];
    
    // Check if texture is tall enough
    auto texSize = _pantsSprite->getTexture()->getContentSize();
    int maxRows = texSize.height / h;
    
    // If index is too large, wrap
    if (styleRowOffset >= maxRows) styleRowOffset = 0;
    
    return cocos2d::Rect(col * w, (styleRowOffset + row) * h, w, h);
}

cocos2d::Rect PlayerAppearance::getShirtRect(int index, Direction dir, int frame) {
    // User: "8x32 chunks. Each chunk ... 8x8 small blocks ... Front, Right, Left, Back"
    // Mapping:
    // 0 (0-8y): Front (Down)
    // 1 (8-16y): Right
    // 2 (16-24y): Left
    // 3 (24-32y): Back (Up)
    
    int w = 8;
    int h = 8;
    
    // Map Direction to Block Index (0-3)
    int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break; // Front
        case Direction::RIGHT: blockIndex = 1; break; // Right
        case Direction::LEFT: blockIndex = 2; break; // Left
        case Direction::UP: blockIndex = 3; break; // Back
    }
    
    // Locate the 8x32 chunk
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
    // User: "hairstyles ... 16x96 chunks"
    // Each chunk divided into 16x32 small blocks.
    // Three forms: Front, Side, Back.
    // Mapping:
    // Block 0 (0-32y): Front (Down)
    // Block 1 (32-64y): Side (Right/Left)
    // Block 2 (64-96y): Back (Up)
    
    int w = 16;
    int h = 32;
    
    // 16x96 chunks. Texture width determines how many chunks per row.
    auto texSize = _hairSprite->getTexture()->getContentSize();
    int cols = texSize.width / 16; // Number of chunks per row
    
    int chunkCol = index % cols;
    int chunkRow = index / cols;
    
    float chunkX = chunkCol * 16;
    float chunkY = chunkRow * 96;
    
    int blockIndex = 0;
    switch(dir) {
        case Direction::DOWN: blockIndex = 0; break; // Front
        case Direction::RIGHT: blockIndex = 1; break; // Side
        case Direction::LEFT: blockIndex = 1; break; // Side (Reuse Right, handled by flip if needed)
        case Direction::UP: blockIndex = 2; break; // Back
    }
    
    float y = chunkY + (blockIndex * 32);
    
    return cocos2d::Rect(chunkX, y, w, h);
}

int PlayerAppearance::getMaxHairStyles() { return 50; } // Approximate
int PlayerAppearance::getMaxShirtStyles() { return 50; }
int PlayerAppearance::getMaxPantsStyles() { return 20; }

cocos2d::Size PlayerAppearance::getTextureSize(const std::string& filename) {
    auto tex = cocos2d::Director::getInstance()->getTextureCache()->addImage(filename);
    if (tex) return tex->getContentSize();
    return cocos2d::Size::ZERO;
}

}
