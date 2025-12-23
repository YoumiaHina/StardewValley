/**
 * MainMenuScene：主菜单场景，负责展示主界面并触发进入游戏/读取存档/退出。
 * - 职责边界：只处理主菜单 UI 事件与场景切换触发，不承载具体存档与游戏逻辑。
 * - 主要协作对象：通过场景切换入口与外部控制模块协作。
 */
#pragma once

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene {
public:
    // 创建主菜单场景实例。
    static cocos2d::Scene* createScene();

    // 初始化主菜单场景。
    virtual bool init() override;

    CREATE_FUNC(MainMenuScene);

private:
    // 处理“开始游戏”按钮回调。
    void onStart(cocos2d::Ref* sender);

    // 处理“读取存档”按钮回调。
    void onLoad(cocos2d::Ref* sender);

    // 处理“退出”按钮回调。
    void onExit(cocos2d::Ref* sender);
};
