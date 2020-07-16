# Telegram-Anti-Revoke

<!--
[![Version](https://img.shields.io/badge/beta-v0.1.3-blue.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/pulls)
-->
[![Platform](https://img.shields.io/badge/platform-windows-orange.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke)
[![Version](https://img.shields.io/github/v/release/SpriteOvO/Telegram-Anti-Revoke)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![Downloads](https://img.shields.io/github/downloads/SpriteOvO/Telegram-Anti-Revoke/total.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)

:earth_americas: English | :earth_asia: [简体中文](/README-CN.md)

Telegram local messages anti-revoke plugin.

## :mag: Preview
![](/Resource/Preview.gif)

## :sparkles: Features
* Prevent local messages from being revoked by the server.
* Revoked messages will be marked as "deleted".
* The plugin has excellent compatibility.
* Mark content supports multi-language automatic recognition.  
Automatic recognition is currently supported:   
*English, Simplified Chinese, Traditional Chinese, Japanese, Korean*

## :tomato: Compatibility
Currently only compatible with Windows Telegram 1.9.0 ~ 2.1.19 or higher.  
For Telegram beta, the plugin will not automatically check for updates, please download the preview version manually.

## :hamburger: Usage
1. Go to the [Release Page](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases) to download the latest version of the plugin.  
2. Close the running Telegram process.  
3. Copy *version.dll* file into the Telegram root directory.  
4. Run the Telegram program.

## :bug: Report
Please use the [Issue Tracker](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) for reporting problems or suggesting new features.

<!--
If a crash occurs, please provide the following information when reporting a problem:
* *ArLog.txt* file. (In the same directory as the *version.dll* file.)
* Which version of Telegram are you using?
* Which operating system are you using?
* Did you do anything when it crashed?
* Reproduce the steps of the crash? (optional)
* More useful information?
-->

If Telegram releases a new version update and the change is large, the plugin may go wrong or cause a crash.  
Please report an issue and wait for plugin update.
<!--Don't repeat reports on reported issues, thank you!-->

## :gem: ThirdParty
* JsonCpp ([MIT License](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE))
* MinHook ([BSD License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :beer: Acknowledgments
* Thanks to [采蘑菇的小蘑菇] for providing help with compiling Telegram.
* Thanks to [dummycatz] for pointing out the cause of cross-module malloc/free memory crashes.

## :warning: Warning
This open source project and plugin is unofficial and is for educational purposes only.