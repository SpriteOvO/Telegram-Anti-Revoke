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
2. Open Telegram and open the top-left menu. From there, note the application version and architecture **\*ARCH\*** (`x86` or `x64`) which is labelled under the version, e.g `Version x.x.x x64` (If you are under `x84`, there is no **\*ARCH\*** displayed).
3. Close the running Telegram process.
4. Follow the instructions accordly to your Telegram version ([Before v2.8.5](#before-v285) - [Since v2.8.5](#since-v285))
5. Run Telegram.

### Before v2.8.5
1. Move the file `TAR-Resources\*ARCH*.dll` to the Telegram directory as `version.dll`.
2. Run `Telegram.exe`.

### Since v2.8.5
1. Turn off the option `Launch Telegram when system starts` in the Telegram settings.
2. Move the file `TAR-Launcher-*ARCH*.exe` and the folder `TAR-Resources` to the Telegram directory.
3. You need to run `TAR-Launcher-*ARCH*.exe` to start Telegram each time.

### Since v2.8.5 - Start with system
1. Copy the file `TAR-Launcher-*ARCH*.exe` to your clipboard.
2. Navigate to `shell:startup` in Explorer (or run it in `Win`+`R`).
3. Right-click (open the context menu) and `Paste shortcut` in that directory.
4. Optionally to make it start minimized see the [-autostart guide](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues/65#issuecomment-890500956).

## :hammer_and_wrench: Build
See the [Build Instructions](/Docs/Build.md).

## :bug: Report
Please use the [Issue Tracker](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) to report issues or suggest new features.  
To avoid duplication of issues, please check existing issues before submitting.

## :gem: ThirdParty
* [sigmatch](https://github.com/SpriteOvO/sigmatch) ([Apache-2.0 License](https://github.com/SpriteOvO/sigmatch/blob/main/LICENSE)) - ✨ Modern C++ 20 signature match / search library (another project of mine)
* [json](https://github.com/nlohmann/json) ([MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
* [MinHook](https://github.com/TsudaKageyu/minhook) ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))
* [spdlog](https://github.com/gabime/spdlog) ([MIT License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))

## :beer: Acknowledgments
* Thanks to *采蘑菇的小蘑菇* for providing help with compiling Telegram.
* Thanks to *dummycatz* for pointing out the cause of cross-module `malloc` / `free` crash.

## :warning: Warning
This open source project and plugin is unofficial and is for educational purposes only.
