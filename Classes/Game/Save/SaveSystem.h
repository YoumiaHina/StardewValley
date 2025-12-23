#pragma once

#include <string>

namespace Game {

// 保存当前全局游戏状态到指定路径的文本文件中。
// - fullPath 为空字符串时，使用 defaultSavePath() 指定的默认存档路径。
// - 返回值为 true 表示保存成功，false 表示文件创建或写入失败。
bool saveToFile(const std::string& fullPath);

// 从指定路径的文本文件中加载游戏存档到全局状态 WorldState。
// - fullPath 为空字符串时，同样使用 defaultSavePath()；
// - 返回值为 true 表示读取并解析成功，false 表示文件打不开或格式/版本不匹配。
bool loadFromFile(const std::string& fullPath);

// 设置自定义的存档根目录（例如 “save” 或绝对路径），用于覆盖默认的写入目录。
// 实际使用时会通过 FileUtils 把相对路径转换成绝对路径。
void setSaveRootDirectory(const std::string& rootDir);

// 记录“当前正在使用的存档文件路径”，方便游戏内展示“最近存档路径”等信息。
void setCurrentSavePath(const std::string& fullPath);

// 返回最近一次 saveToFile/loadFromFile 使用的存档完整路径。
std::string currentSavePath();

// 根据玩家输入的存档名生成一个合法的文件路径：
// - 会过滤非法文件名字符；
// - 自动补全 .txt 后缀；
// - 前面拼上 saveDirectory() 得到最终绝对路径。
std::string makeSavePathWithName(const std::string& name);

// 返回存档目录的绝对路径：
// - 若 setSaveRootDirectory 设置了自定义目录，则优先使用；
// - 否则使用 cocos2d::FileUtils 的 writablePath() 下面的 “save” 子目录。
std::string saveDirectory();

// 默认存档路径，一般对应第 1 号存档槽位。
std::string defaultSavePath();

// 根据槽位编号生成对应的存档路径（save1.txt ~ save50.txt）。
std::string savePathForSlot(int slot);

} // namespace Game
