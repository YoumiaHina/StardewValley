#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Interact/TownInteractor.h"
#include "Controllers/NPC/NpcControllerBase.h"
namespace Controllers { class ChestInteractor; }
namespace Controllers { class AbigailNpcController; class PierreNpcController; }

class TownScene : public SceneBase {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(TownScene);

protected:
    bool init() override;
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    void positionPlayerInitial() override;
    void onSpacePressed() override;
    const char* doorPromptText() const override;
    void onMouseDown(cocos2d::EventMouse* e) override;

private:
    Controllers::TownMapController* _townMap = nullptr;
    Controllers::TownInteractor _interactor;
    Controllers::NpcController* _npcController = nullptr;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
};
