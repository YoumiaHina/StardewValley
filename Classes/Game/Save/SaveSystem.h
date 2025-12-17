#pragma once

#include <string>

namespace Game {

bool saveToFile(const std::string& fullPath);
bool loadFromFile(const std::string& fullPath);

void setSaveRootDirectory(const std::string& rootDir);
void setCurrentSavePath(const std::string& fullPath);
std::string currentSavePath();
std::string makeSavePathWithName(const std::string& name);

std::string saveDirectory();
std::string defaultSavePath();
std::string savePathForSlot(int slot);

} // namespace Game
