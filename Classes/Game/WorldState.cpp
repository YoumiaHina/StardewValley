#include "Game/WorldState.h"

namespace Game {

WorldState& globalState() {
    static WorldState state;
    return state;
}

} // namespace Game