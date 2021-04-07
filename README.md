# Telegram-Anti-Revoke

[![](https://img.shields.io/badge/platform-windows-orange.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke)
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
2. Close the running Telegram process.  
3. Copy *version.dll* file into the Telegram root directory.  
4. Run the Telegram program.

## :bug: Report
Please use the [Issue Tracker](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) to report issues or suggest new features.  
To avoid duplication of issues, please check existing issues before submitting.

## :gem: ThirdParty
* json ([MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT))
* MinHook ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :beer: Acknowledgments
* Thanks to *采蘑菇的小蘑菇* for providing help with compiling Telegram.
* Thanks to *dummycatz* for pointing out the cause of cross-module `malloc` / `free` crash.

## :warning: Warning
This open source project and plugin is unofficial and is for educational purposes only.