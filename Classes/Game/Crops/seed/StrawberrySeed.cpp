// 草莓种子：物品栏静态信息（名称/价格/映射作物类型）
#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class StrawberrySeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::StrawberrySeed; }
    CropType cropType() const override { return CropType::Strawberry; }
    const char* name() const override { return "Strawberry Seed"; }
    int price() const override { return 25; }
};

}
