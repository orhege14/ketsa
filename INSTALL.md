# Installing Ketsa

## Quick Install

### Windows
```
git clone https://github.com/ketsa-lang/ketsa
cd ketsa
build.bat
```

### Linux / macOS
```bash
git clone https://github.com/ketsa-lang/ketsa
cd ketsa
chmod +x build.sh
./build.sh
```

### Install System-Wide
```
ketsa install
```

## Prerequisites

| Requirement | Windows | Linux | macOS |
|------------|---------|-------|-------|
| C++20 Compiler | MSVC 2022+ | GCC 11+ / Clang 14+ | Apple Clang 14+ |
| CMake 3.20+ | `winget install Kitware.CMake` | `apt install cmake` | `brew install cmake` |
| Ninja (optional) | `winget install Ninja` | `apt install ninja-build` | `brew install ninja` |
| Git | `winget install Git` | `apt install git` | `brew install git` |

## Verify Installation

```bash
ketsa --version
ketsa doctor    # System diagnostics
```

## Build From Source

### Debug Build
```bash
build.bat           # Windows
./build.sh          # Linux/macOS
```

### Release Build
```bash
build.bat --release
./build.sh --release
```

## Run Your First Program

```bash
ketsa new hello
cd hello
ketsa run src/main.ketsa
```
