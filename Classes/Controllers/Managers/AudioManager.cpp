#include "Controllers/Managers/AudioManager.h"
#include "audio/include/AudioEngine.h"

using namespace cocos2d::experimental;

namespace Managers {

AudioManager& AudioManager::getInstance() {
    static AudioManager inst;
    return inst;
}

void AudioManager::stop() {
    if (_currentAudioId != -1) {
        AudioEngine::stop(_currentAudioId);
        _currentAudioId = -1;
        _currentPath.clear();
    }
}

void AudioManager::playBackgroundFor(SceneZone zone) {
    const std::string path = pathForZone(zone);
    if (path.empty()) return;

    if (!_currentPath.empty() && _currentPath == path) {
        if (_currentAudioId != -1) {
            auto state = AudioEngine::getState(_currentAudioId);
            if (state == AudioEngine::AudioState::PLAYING) return;
        }
    }

    if (_currentAudioId != -1) {
        AudioEngine::stop(_currentAudioId);
        _currentAudioId = -1;
    }

    AudioEngine::preload(path);
    _currentAudioId = AudioEngine::play2d(path, true, _volume);
    _currentPath = path;
}

std::string AudioManager::pathForZone(SceneZone zone) const {
    switch (zone) {
        case SceneZone::Farm: return "music/stardew.mp3";
        case SceneZone::Room: return "music/coop.mp3";
        case SceneZone::Abyss: return "music/coop.mp3";
        default: return "";
    }
}

} // namespace Managers

