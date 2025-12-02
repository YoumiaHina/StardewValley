/**
 * SplashScene: Boot scene.
 */
#pragma once

#include "cocos2d.h"

class SplashScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(SplashScene);

private:
    void goToMainMenu(float dt);
};