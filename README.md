# Hopgood Operator-Precedence Parser

This repository is a modern C++ reconstruction of the operator-precedence parsing technique
described by F. R. A. Hopgood.

## Prerequisites

- A C++23 compiler
- CMake 3.25 or newer
- Ninja

On macOS with Homebrew, the required build packages are `cmake` and `ninja`. The Apple Clang
compiler supplied by Xcode is supported. CMake downloads the pinned Catch2 release into the
local build tree when tests are enabled.

## Build and test

The default development workflow uses the checked-in CMake presets:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

To run the same checks with development sanitizers:

```sh
cmake --preset sanitizers
cmake --build --preset sanitizers
ctest --preset sanitizers
```

# Editor setup
I used Helix. For your case it might be different. Might need additional setup. If anyone is using Helix like I do I can share .helix configuration, just ask :).
