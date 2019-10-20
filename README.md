# Telegram-Anti-Revoke

[![Version](https://img.shields.io/badge/beta-v0.1.0-blue.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/releases)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/SpriteOvO/Telegram-Anti-Revoke/pulls)

:earth_americas: English | [:earth_asia: 简体中文](/README-CN.md)

Telegram local message anti-revoke plugin.

## :mag: Preview
![](/Resource/Preview.gif)

## :sparkles: Features
* Prevent local messages from being revoked by the server.
* Messages that are revoked will be marked as "deleted".
* The plugin has excellent compatibility.
* Mark content supports multi-language automatic recognition.  
Automatic recognition is currently supported:   
*English, Simplified Chinese, Traditional Chinese, Japanese, Korean*
* You can still actively delete messages.

## :tomato: Compatibility
Currently only compatible with Windows Telegram 1.8.0 ~ 1.8.15 or higher.

## :hamburger: Usage
1. Close the running Telegram process.  
2. Copy *avrt.dll* file into the Telegram root directory.  
3. Run the Telegram program.

## :bug: Report
Please use the [GitHub issue tracker](https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues) for reporting problems or suggesting new features.

<!--
If a crash occurs, please provide the following information when reporting a problem:
* *ArLog.txt* file. (In the same directory as the *avrt.dll* file.)
* Which version of Telegram are you using?
* Which operating system are you using?
* Did you do anything when it crashed?
* Reproduce the steps of the crash? (optional)
* More useful information?
-->

If Telegram releases a new version update and the change is large, the plugin may go wrong or cause a crash.  
Please report the issue and wait for the plugin to update.
<!--Don't repeat reports on reported issues, thank you!-->

## :gem: ThirdParty
* JsonCpp ([MIT License](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE))
* MinHook ([BSD License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :beer: Acknowledgments
* Thanks to [采蘑菇的小蘑菇] for providing help with compiling Telegram.
* Thanks to [dummycatz] for pointing out the reason for the error in malloc/free memory across modules.

## :warning: Warning
* This open source project and plugin is unofficial and is for educational purposes only.