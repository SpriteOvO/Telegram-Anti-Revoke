# Telegram-Anti-Revoke

[![](https://img.shields.io/badge/platform-windows-orange.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke)
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
2. 关闭正在运行的 Telegram 进程。  
3. 复制 *version.dll* 文件到 Telegram 根目录。  
4. 运行 Telegram 程序。

## :bug: 报告
请使用 [问题跟踪器](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) 报告问题或提出新功能。  
为避免问题重复，请在提交前检查已有的问题。

## :gem: 第三方
* Json ([MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
* MinHook ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :beer: 鸣谢
* 感谢 *采蘑菇的小蘑菇* 提供编译 Telegram 的帮助。
* 感谢 *dummycatz* 指出跨模块 `malloc` / `free` 崩溃的原因。

## :warning: 警告
此开源项目及插件是非官方的，且仅用于教育目的。