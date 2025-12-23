/**
 * SplashScene：启动过渡场景，负责展示启动画面并跳转到主菜单。
 * - 职责边界：只管理启动展示与定时跳转，不承载资源系统与业务逻辑。
 * - 主要协作对象：通过场景切换入口协作。
 */
#pragma once

#include "cocos2d.h"

class SplashScene : public cocos2d::Scene {
public:
    /**
     * 创建启动过渡场景实例。
     */
    static cocos2d::Scene* createScene();

    /**
     * 初始化启动过渡场景。
     */
    virtual bool init() override;

    CREATE_FUNC(SplashScene);

private:
    /**
     * 定时回调：跳转到主菜单场景。
     */
    void goToMainMenu(float dt);
};
