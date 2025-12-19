// 玉米种子：物品栏静态信息（名称/价格/映射作物类型）
#include "Game/Crops/seed/SeedBase.h"

namespace Game {

// 玉米种子物品：仅提供背包内静态元数据。
class CornSeed : public SeedBase {
public:
    // 返回该物品对应的枚举类型。
    ItemType item() const override { return ItemType::CornSeed; }
    // 返回播种后对应的作物类型。
    CropType cropType() const override { return CropType::Corn; }
    // 返回用于 UI 展示的名称。
    const char* name() const override { return "Corn Seed"; }
    // 返回购买价格（供商店/经济系统使用）。
    int price() const override { return 25; }
};

}
