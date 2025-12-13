#pragma once

#include <string>

namespace Managers {

enum class SceneZone { Farm, Room, Abyss };

class AudioManager {
public:
    static AudioManager& getInstance();

    void playBackgroundFor(SceneZone zone);
    void stop();

private:
    int _currentAudioId = -1;
    std::string _currentPath;
    float _volume = 0.6f;

    std::string pathForZone(SceneZone zone) const;
};

} // namespace Managers

