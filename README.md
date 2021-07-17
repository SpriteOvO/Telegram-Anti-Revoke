# Telegram-Anti-Revoke

[![](https://github.com/SpriteOvO/Telegram-Anti-Revoke/actions/workflows/windows-msvc.yml/badge.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/actions/workflows/windows-msvc.yml)
[![](https://img.shields.io/github/v/release/SpriteOvO/Telegram-Anti-Revoke)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![](https://img.shields.io/github/downloads/SpriteOvO/Telegram-Anti-Revoke/total.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)

:earth_americas: English | :earth_asia: [简体中文](/README-CN.md)

Telegram local messages anti-revoke plugin.

## :mag: Preview
![](/Resource/Preview.gif)

## :sparkles: Features
* Prevent local messages from being revoked by the server.
* Revoked messages will be marked as "deleted".
* The plugin has excellent compatibility.
* Multi-language support.  

## :tomato: Compatibility
Usually the plugin is not affected by small updates of Telegram.  
But some updates may cause the plugin to error or crash, please [report the issue](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) and wait for the plugin to be updated.

If you are using **Telegram beta**, please check the latest preview version.  
(Compatibility updates for **Telegram beta** may not be released in time)

## :hamburger: Usage
1. Go to the [Release Page](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases) to download the latest version of the plugin.  
2. Open Telegram and open the top-left menu. From there, note the application version and architecture \***ARCH**\* (x86 or x64) which is labelled under its version, e.g "**Version** x.x.x **x64**" (If you are under x84 it shouldn't display the architecture on the version).
3. Close the running Telegram process.
4. Follow the instructions accordly to your Telegram version ([Before 2.8.5](#before-285) - [After 2.8.4](#after-284))
5. Run Telegram.

### Before 2.8.5
1. Move the file "TAR-Resources\\***ARCH**\*.dll" to the Telegram directory as `version.dll`.

### After 2.8.4
1. Turn off the option "Launch Telegram when system starts" in the Telegram settings.
2. Move the file "TAR-Launcher-\***ARCH**\*.exe" and the folder "TAR-Resources" to the Telegram directory.
3. Execute the "TAR-Launcher-\***ARCH**\*.exe" or/and set it to [start with system](#after-284---start-with-system)

### After 2.8.4 - Start with system
1. Copy the file "TAR-Launcher-\***ARCH**\*.exe" to your clipboard.
2. Navigate to `shell:startup` in Explorer `Win`+`R`).
3. Right-click (open your context menu) and "Paste shortcut" in that directory.

## :hammer_and_wrench: Build
See the [Build Instructions](/Docs/Build.md).

## :bug: Report
Please use the [Issue Tracker](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) to report issues or suggest new features.  
To avoid duplication of issues, please check existing issues before submitting.

## :gem: ThirdParty
* json ([MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
* MinHook ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))
* spdlog ([MIT License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))

## :beer: Acknowledgments
* Thanks to *采蘑菇的小蘑菇* for providing help with compiling Telegram.
* Thanks to *dummycatz* for pointing out the cause of cross-module `malloc` / `free` crash.

## :warning: Warning
This open source project and plugin is unofficial and is for educational purposes only.
