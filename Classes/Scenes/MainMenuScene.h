/**
 * MainMenuScene: Main menu scene.
 */
#pragma once

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(MainMenuScene);

private:
    void onStart(cocos2d::Ref* sender);
    void onExit(cocos2d::Ref* sender);
};