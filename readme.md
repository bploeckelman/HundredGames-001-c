# Hundred Games - 001 - Prong

## Description

Brian makes 100 Games, starting in 2024. This is the first one, a Pong variant.

## Requirements

- `pthreads` for [`Physac`](https://github.com/victorfisac/Physac)
    - on Windows, install through the `vcpkg` tool
    - in Clion, ensure the `Add vcpkg integration to existing CMake profiles` option is checked in the `vcpkg` tool
    - this adds `-DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake` to IDE CMake commands

### Resources

- [Raylib game template](https://github.com/raysan5/raylib-game-template)
