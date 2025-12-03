#ifndef __CUSTOMIZATION_SCENE_H__
#define __CUSTOMIZATION_SCENE_H__

#include "cocos2d.h"
#include "Game/PlayerAppearance.h"

class CustomizationScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    
    // Callback for Start Game
    void onStartGame(cocos2d::Ref* sender);
    
    CREATE_FUNC(CustomizationScene);

private:
    Game::PlayerAppearance* _character;
    
    int _currentShirt = 0;
    int _currentPants = 0;
    int _currentHair = 0;
    
    cocos2d::Label* _shirtLabel;
    cocos2d::Label* _pantsLabel;
    cocos2d::Label* _hairLabel;
    
    void changeShirt(int delta);
    void changePants(int delta);
    void changeHair(int delta);
    void changeHairColor(const cocos2d::Color3B& color);
    
    void updateLabels();
};

#endif // __CUSTOMIZATION_SCENE_H__
