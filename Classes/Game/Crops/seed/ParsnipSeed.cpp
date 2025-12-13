// 萝卜种子：物品栏静态信息（名称/价格/映射作物类型）
#include "Game/Crops/seed/SeedBase.h"

namespace Game {

class ParsnipSeed : public SeedBase {
public:
    ItemType item() const override { return ItemType::ParsnipSeed; }
    CropType cropType() const override { return CropType::Parsnip; }
    const char* name() const override { return "Parsnip Seed"; }
    int price() const override { return 25; }
};

}
