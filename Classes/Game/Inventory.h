/**
 * Inventory: simple toolbar inventory for tools.
 */
#pragma once

#include <vector>
#include <cstddef>
#include "Game/Tool.h"

namespace Game {

class Inventory {
public:
    explicit Inventory(std::size_t slots);

    std::size_t size() const { return _slots.size(); }

    void setSlot(std::size_t index, const Tool& tool);
    const Tool* getSlot(std::size_t index) const;

    int selectedIndex() const { return _selected; }
    void selectIndex(int index);
    void next();
    void prev();

    const Tool* selectedTool() const { return getSlot(static_cast<std::size_t>(_selected)); }

private:
    std::vector<Tool> _slots;
    int _selected = 0;
};

} // namespace Game