# Bosh Broadcast Pro - Build Map

## Overview

This document describes how to build Bosh Broadcast Pro as an integrated OBS Studio fork.

---

## Prerequisites

### All Platforms

- Git
- CMake 3.28-3.30
- Qt 6.5+ (Widgets, Network, Concurrent)
- C++17 compatible compiler
- Python 3.x

### Windows

- Visual Studio 2022 (Community or higher)
- Windows SDK 10+
- OBS Studio build dependencies

### macOS

- Xcode 15+
- macOS SDK 11+
- OBS Studio build dependencies

### Linux

- GCC 11+ or Clang 14+
- Development packages for:
  - Qt6 (widgets, network, concurrent)
  - X11 or Wayland
  - PipeWire or PulseAudio
  - V4L2
  - libcurl
  - libmbedtls

---

## Directory Structure

```
obs-studio/                    # Root OBS repository
├── plugins/
│   ├── bosh/                  # NEW: Bosh plugin
│   │   ├── CMakeLists.txt
│   │   ├── bosh-plugin.cpp
│   │   ├── core/
│   │   ├── ai/
│   │   ├── agents/
│   │   ├── events/
│   │   ├── alerts/
│   │   ├── chat/
│   │   ├── overlays/
│   │   ├── ui/
│   │   └── data/
│   ├── obs-browser/           # Required for Karma
│   └── ...                    # Other OBS plugins
├── frontend/
│   └── docks/                 # May add Bosh docks here
├── docs/
│   └── bosh/                  # Bosh documentation
└── ...                        # Standard OBS structure
```

---

## Build Steps

### 1. Clone Repository

```bash
git clone https://github.com/obsproject/obs-studio.git
cd obs-studio

# Initialize submodules (required for obs-browser, obs-websocket)
git submodule update --init --recursive
```

### 2. Add Bosh Plugin

Copy Bosh plugin source to `plugins/bosh/`:

```bash
# After Bosh plugin code is ready
cp -r /path/to/bosh-plugin/* obs-studio/plugins/bosh/
```

### 3. Modify plugins/CMakeLists.txt

Add Bosh plugin registration (alphabetical order):

```cmake
# Add after existing plugins
add_obs_plugin(
    bosh
    PLATFORMS WINDOWS MACOS LINUX
    WITH_MESSAGE
)
```

### 4. Configure Build

#### Windows (Visual Studio)

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2019_64" `
    -DENABLE_FRONTEND=ON `
    -DENABLE_SCRIPTING=ON
```

#### macOS

```bash
cmake -B build \
    -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@6" \
    -DENABLE_FRONTEND=ON \
    -DENABLE_SCRIPTING=ON
```

#### Linux

```bash
cmake -B build \
    -DCMAKE_PREFIX_PATH="/usr/lib/qt6" \
    -DENABLE_FRONTEND=ON \
    -DENABLE_SCRIPTING=ON \
    -DENABLE_PIPEWIRE=ON
```

### 5. Build

```bash
# Build all
cmake --build build --config Release

# Or build only Bosh plugin
cmake --build build --target bosh --config Release
```

### 6. Install/Run

#### Development Run

```bash
# Set runtime path to build output
export OBS_RUNPATH="./build/runway"
./build/frontend/obs-studio(.exe)
```

#### Installation

```bash
cmake --install build --prefix /opt/bosh-broadcast-pro
```

---

## CMake Configuration

### plugins/bosh/CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.28...3.30)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent)

add_library(bosh MODULE)
add_library(OBS::bosh ALIAS bosh)

target_sources(
    bosh
    PRIVATE
        bosh-plugin.cpp
        bosh-plugin.h
        core/bosh-core.cpp
        core/bosh-core.h
        events/event-bus.cpp
        events/event-bus.h
        # ... additional sources
)

target_link_libraries(
    bosh
    PRIVATE
        OBS::frontend-api
        OBS::libobs
        OBS::qt-wrappers
        Qt::Widgets
        Qt::Network
        Qt::Concurrent
)

if(OS_WINDOWS)
    configure_file(cmake/windows/obs-module.rc.in bosh.rc)
    target_sources(bosh PRIVATE bosh.rc)
endif()

set_target_properties_obs(
    bosh
    PROPERTIES FOLDER plugins/bosh
               PREFIX ""
               AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON
)
```

---

## Build Options

### CMake Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `ENABLE_BOSH` | ON | Build Bosh plugin |
| `ENABLE_BOSH_AI` | ON | Include AI features |
| `ENABLE_BOSH_OLLAMA` | ON | Ollama integration |
| `ENABLE_BOSH_CHAT` | ON | Chat system |
| `ENABLE_BOSH_KARMA` | ON | Karma overlays |

### Feature Flags

```cmake
option(ENABLE_BOSH_AI "Enable Bosh AI features" ON)
option(ENABLE_BOSH_OLLAMA "Enable Ollama integration" ON)

if(ENABLE_BOSH_AI)
    target_compile_definitions(bosh PRIVATE BOSH_AI_ENABLED)
endif()

if(ENABLE_BOSH_OLLAMA)
    target_compile_definitions(bosh PRIVATE BOSH_OLLAMA_ENABLED)
endif()
```

---

## Build Artifacts

### Output Files

| Platform | Plugin File | Location |
|----------|-------------|----------|
| Windows | `bosh.dll` | `build/plugins/bosh/` |
| macOS | `bosh.so` | `build/plugins/bosh/` |
| Linux | `bosh.so` | `build/plugins/bosh/` |

### Data Files

```
data/
└── obs-plugins/
    └── bosh/
        ├── en-US/
        │   └── bosh.ini
        ├── templates/
        │   └── ...
        └── themes/
            └── ...
```

---

## Debugging

### Enable Debug Logging

```cpp
// In bosh-plugin.cpp
#define DEBUG_LOGGING 1

#if DEBUG_LOGGING
#define BLOG(level, msg) blog(level, "[Bosh] " msg)
#else
#define BLOG(level, msg)
#endif
```

### Debug Build

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

### Memory Checking

```bash
# Linux: valgrind
valgrind --leak-check=full ./build-debug/frontend/obs-studio

# macOS: Instruments
open -a Instruments.app

# Windows: Visual Studio Debugger
```

---

## Common Build Issues

### Issue: Qt Not Found

**Solution:**
```bash
cmake -DCMAKE_PREFIX_PATH="/path/to/qt" ...
```

### Issue: Submodule Missing

**Solution:**
```bash
git submodule update --init --recursive
```

### Issue: Compiler Version Too Old

**Solution:** Upgrade compiler or use newer toolchain.

### Issue: CMake Version Mismatch

**Solution:** Install CMake 3.28-3.30.

---

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Build Bosh

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [windows-latest, macos-latest, ubuntu-latest]
    
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      
      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.3'
      
      - name: Configure
        run: cmake -B build -DENABLE_FRONTEND=ON
      
      - name: Build
        run: cmake --build build --config Release
      
      - name: Test
        run: ctest --test-dir build
```

---

## Performance Build Tips

1. **Use Ninja generator** for faster builds:
   ```bash
   cmake -G Ninja -B build
   ```

2. **Enable ccache** for incremental builds:
   ```bash
   cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -B build
   ```

3. **Build only changed targets**:
   ```bash
   cmake --build build --target bosh
   ```

4. **Use release mode for performance testing**:
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release -B build
   ```

---

## Distribution

### Package Structure

```
BoshBroadcastPro-Installer/
├── obs-studio.exe          # Modified OBS
├── plugins/
│   └── bosh.dll            # Bosh plugin
├── data/
│   └── obs-plugins/
│       └── bosh/           # Bosh data files
└── README.txt
```

### Installer Creation

- **Windows**: NSIS or Inno Setup
- **macOS**: pkgbuild + productbuild
- **Linux**: DEB/RPM packages or AppImage

---

*Update this document when build process changes.*
