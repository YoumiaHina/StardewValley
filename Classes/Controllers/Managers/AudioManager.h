#pragma once

#include <string>

namespace Managers {

// 场景区域：
// - Farm：农场地图背景音乐。
// - Room：室内/房间背景音乐。
// - Abyss：矿洞/深处等特殊场景音乐。
enum class SceneZone { Farm, Room, Abyss };

// 全局音频管理器：
// - 封装 cocos AudioEngine 的简单使用，保证同一时刻只播放一首 BGM。
// - 按 SceneZone 选择对应音乐资源，并负责预加载、循环播放与停止。
class AudioManager {
public:
    // 获取单例实例。
    static AudioManager& getInstance();

    // 为指定区域播放背景音乐；如果与当前相同则不会重复切歌。
    void playBackgroundFor(SceneZone zone);
    // 停止当前背景音乐播放并清空状态。
    void stop();

private:
    int _currentAudioId = -1;
    std::string _currentPath;
    float _volume = 0.6f;

    // 根据场景区域返回对应的音乐资源路径。
    std::string pathForZone(SceneZone zone) const;
};

} // namespace Managers

