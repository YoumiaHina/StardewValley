#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/Interact/BeachInteractor.h"
namespace Controllers { class ChestInteractor; }

class BeachScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(BeachScene);

protected:
    bool init() override;
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;
    void onMouseDown(cocos2d::EventMouse* e) override;

private:
    Controllers::BeachMapController* _beachMap = nullptr;
    Controllers::BeachInteractor _interactor;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
};
