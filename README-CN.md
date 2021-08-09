# Telegram-Anti-Revoke

[![](https://github.com/SpriteOvO/Telegram-Anti-Revoke/actions/workflows/windows-msvc.yml/badge.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/actions/workflows/windows-msvc.yml)
[![](https://img.shields.io/github/v/release/SpriteOvO/Telegram-Anti-Revoke)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![](https://img.shields.io/github/downloads/SpriteOvO/Telegram-Anti-Revoke/total.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)

:earth_americas: [English](/README.md) | :earth_asia: 简体中文

Telegram 本地消息防撤回插件。

## :mag: 预览
![](/Resource/Preview.gif)

## :sparkles: 特性
* 阻止本地消息被服务器撤回。
* 被撤回的消息将被标记上 “已删除” 。
* 插件具有极好的兼容性。
* 多语言支持。  

## :tomato: 兼容性
通常该插件不受 Telegram 小更新的影响。  
但某些更新可能会使插件出错或崩溃，请 [报告该问题](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) 并等待插件更新。

如果您正在使用 **Telegram beta**，请检查最新预览版本。  
（对于 **Telegram beta** 的兼容性更新可能不会及时发布）

## :hamburger: 用法
1. 前往 [发布页面](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases) 下载最新版本插件。  
2. 打开 Telegram 顶层菜单，注意底部的 ARCH 信息，`Version x.x.x` 为 x86 ARCH，`Version x.x.x x64` 为 x64 ARCH。  
3. 关闭正在运行的 Telegram 进程。  
4. 根据您正在使用的 Telegram 版本遵循下面的步骤。

### v2.8.5 之前
1. 复制 *version.dll* 文件到 Telegram 根目录。

### v2.8.4 之后
1. 在 Telegram 设置中关闭开机自启。
2. 移动 “TAR-Launcher-\***ARCH**\*.exe” 文件和 “TAR-Resources” 文件夹到 Telegram 目录。
3. 运行 “TAR-Launcher-\***ARCH**\*.exe” 或者设置该文件为开机自启。

### v2.8.4 之后 - 设置开机自启
1. 复制 “TAR-Launcher-\***ARCH**\*.exe” 文件。
2. 在 `Win`+`R` 中输入 `shell:startup` 回车转到启动目录。
3. 在该目录中右键菜单点击 “粘贴快捷方式”。
4. 使其在开机启动时最小化，请参见 [#65 中的评论](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues/65#issuecomment-890500956)。

## :hammer_and_wrench: 构建
转到 [构建说明](/Docs/Build.md)。

## :bug: 报告
请使用 [问题跟踪器](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) 报告问题或提出新功能。  
为避免问题重复，请在提交前检查已有的问题。

## :gem: 第三方
* [sigmatch](https://github.com/SpriteOvO/sigmatch) ([Apache-2.0 License](https://github.com/SpriteOvO/sigmatch/blob/main/LICENSE)) - ✨ 现代 C++ 20 签名匹配/搜索库（我的另一个项目）
* [json](https://github.com/nlohmann/json) ([MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
* [MinHook](https://github.com/TsudaKageyu/minhook) ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))
* [spdlog](https://github.com/gabime/spdlog) ([MIT License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))

## :beer: 鸣谢
* 感谢 *采蘑菇的小蘑菇* 提供编译 Telegram 的帮助。
* 感谢 *dummycatz* 指出跨模块 `malloc` / `free` 崩溃的原因。

## :warning: 警告
此开源项目及插件是非官方的，且仅用于教育目的。
