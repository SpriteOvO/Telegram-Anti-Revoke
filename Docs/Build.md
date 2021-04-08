# Build Instructions

## Preparations
```
git clone --recursive https://github.com/SpriteOvO/Telegram-Anti-Revoke.git
cd Telegram-Anti-Revoke
mkdir Build
cd Build
```
The current path will be named ___BuildPath___ in the rest of this document.

Download and install [CMake](https://cmake.org/download/) (>= v3.15) if you don't have it.

Choose a target platform to continue:
- [Windows](#windows)

## Windows

Download and install __Visual Studio 2019__ if you don't have it.

Open __Powershell__, go to ___BuildPath___.

Modify the following arguments according to your needs and run it.

```
cmake -G "Visual Studio 16 2019" -A <Win32|x64> -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel> -DTAR_OS=<WIN7|WIN10> ../
cmake --build . --config <Debug|Release|RelWithDebInfo|MinSizeRel>
ls ./Binary
```
