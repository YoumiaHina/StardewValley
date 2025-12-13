#pragma once

#include <vector>
#include "Game/Crop.h"
#include "Game/WorldState.h"
#include "Controllers/IMapController.h"

namespace Controllers {

class CropSystem {
public:
    CropSystem();
    const std::vector<Game::Crop>& crops() const;
    std::vector<Game::Crop>& crops();
    int findCropIndex(int c, int r) const;
    void plantCrop(Game::CropType type, int c, int r);
    void markWateredAt(int c, int r);
    bool canHarvestAt(int c, int r) const;
    bool yieldsOnHarvestAt(int c, int r) const;
    void advanceCropsDaily(IMapController* map);
    void harvestCropAt(int c, int r);
    void instantMatureAllCrops();
    void advanceCropOnceAt(int c, int r);

private:
    std::vector<Game::Crop> _crops;
    void syncLoad();
    void syncSave();
    struct ICropBehavior {
        virtual bool canAccelerate(const Game::Crop& cp) const = 0;
        virtual bool canHarvest(const Game::Crop& cp) const = 0;
        virtual void onHarvest(Game::Crop& cp, bool& remove) const = 0;
        virtual bool onDailyRegrow(Game::Crop& cp) const = 0;
        virtual bool accelerate(Game::Crop& cp) const = 0;
        virtual bool yieldsOnHarvest(const Game::Crop& cp) const = 0;
        virtual ~ICropBehavior() = default;
    };
    struct DefaultBehavior;
    struct BlueberryBehavior;
    struct CornBehavior;
    struct StrawberryBehavior;
    struct EggplantBehavior;
    const ICropBehavior& behaviorFor(Game::CropType t) const;
};

}
