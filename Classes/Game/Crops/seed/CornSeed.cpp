// 玉米种子：物品栏静态信息（名称/价格/映射作物类型）
#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class CornSeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::CornSeed; }
    CropType cropType() const override { return CropType::Corn; }
    const char* name() const override { return "Corn Seed"; }
    int price() const override { return 25; }
};

}
