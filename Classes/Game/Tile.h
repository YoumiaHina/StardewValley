#pragma once

namespace Game {

// TileType：农场/地图格子类型枚举。
// - Soil    ：可耕作的普通土地（未翻地、未浇水）；
// - Tilled  ：已经被锄头翻过的耕地，可用于播种；
// - Watered ：已浇水的耕地，当日可推进作物生长；
// - NotSoil：不可耕作区域（道路、建筑、障碍等），仅用于碰撞/遮罩。
enum class TileType {
    Soil,
    Tilled,
    Watered,
    NotSoil
};

} // namespace Game
