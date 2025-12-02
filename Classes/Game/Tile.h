/**
 * Tile: basic tile types for the map grid.
 */
#pragma once

namespace Game {

enum class TileType {
    Soil,      // default ground
    Tilled,    // hoed soil
    Watered,   // watered tilled soil
    Rock,      // obstacle removable by pickaxe
    Tree       // obstacle removable by axe
};

} // namespace Game