#ifndef __CUSTOMIZATION_SCENE_H__
#define __CUSTOMIZATION_SCENE_H__

#include "cocos2d.h"
#include "Game/View/PlayerView.h"

/**
 * CustomizationScene：角色外观定制场景，负责展示角色预览与外观选项切换。
 * - 职责边界：只做 UI 交互与外观参数调整，不承载存档/进入游戏的业务规则。
 * - 主要协作对象：通过场景切换入口与角色视图等接口协作。
 */
class CustomizationScene : public cocos2d::Scene {
public:
    // 创建外观定制场景实例。
    static cocos2d::Scene* createScene();

    // 初始化外观定制场景。
    virtual bool init() override;
    
    // 处理“开始游戏”按钮回调。
    void onStartGame(cocos2d::Ref* sender);
    
    CREATE_FUNC(CustomizationScene);

private:
    Game::PlayerView* _character;
    
    int _currentShirt = 0;
    int _currentPants = 0;
    int _currentHair = 0;
    
    cocos2d::Label* _shirtLabel = nullptr;
    cocos2d::Label* _pantsLabel = nullptr;
    cocos2d::Label* _hairLabel = nullptr;
    
    // 切换上衣样式。
    void changeShirt(int delta);

    // 切换裤子样式。
    void changePants(int delta);

    // 切换发型样式。
    void changeHair(int delta);

    // 修改头发颜色。
    void changeHairColor(const cocos2d::Color3B& color);
    
    // 刷新当前 UI 文本显示。
    void updateLabels();
};

#endif // __CUSTOMIZATION_SCENE_H__
