# OBS Studio Architecture Audit

## Executive Summary

This document provides a comprehensive audit of the OBS Studio source tree to enable deep integration of Bosh Broadcast Pro features directly into the OBS codebase.

**Repository**: https://github.com/obsproject/obs-studio  
**Audit Date**: 2024  
**Purpose**: Foundation for Bosh Broadcast Pro fork integration

---

## 1. Repository Structure Overview

```
obs-studio/
├── CMakeLists.txt              # Root build configuration
├── cmake/                      # Build system modules
├── deps/                       # Third-party dependencies
├── docs/                       # Documentation
├── frontend/                   # Qt-based UI application
├── libobs/                     # Core OBS library
├── libobs-d3d11/               # Direct3D 11 integration (Windows)
├── libobs-metal/               # Metal integration (macOS)
├── libobs-opengl/              # OpenGL integration
├── libobs-winrt/               # WinRT integration (Windows)
├── plugins/                    # OBS plugins (official)
├── shared/                     # Shared code libraries
├── test/                       # Test infrastructure
└── additional_install_files/   # Installation resources
```

---

## 2. Frontend Layer Analysis

### Location: `frontend/`

The frontend is the Qt-based user interface application that users interact with.

### Key Files:

| File | Purpose |
|------|---------|
| `OBSApp.cpp/hpp` | Main Qt application class, initialization, event handling |
| `obs-main.cpp` | Application entry point |
| `OBSStudioAPI.cpp/hpp` | Public API for frontend-backend communication |
| `OBSApp_Themes.cpp` | Theme management |

### Subdirectories:

| Directory | Purpose |
|-----------|---------|
| `api/` | Frontend API implementation |
| `components/` | Reusable UI components |
| `data/` | UI data resources |
| `dialogs/` | Modal dialogs |
| `docks/` | Dockable panels (BrowserDock, YouTubeAppDock, etc.) |
| `forms/` | Qt Designer UI forms (.ui files) |
| `importers/` | Profile/scene collection importers |
| `models/` | Qt data models |
| `oauth/` | OAuth authentication handlers |
| `plugin-manager/` | Plugin management UI |
| `settings/` | Settings dialogs |
| `updater/` | Application updater |
| `utility/` | Utility classes (CrashHandler, OBSEventFilter, etc.) |
| `widgets/` | Custom Qt widgets (OBSBasic, etc.) |
| `wizards/` | Setup wizards |

### Dock System (Critical for Bosh Integration):

Current docks in `frontend/docks/`:
- `BrowserDock.cpp/hpp` - Browser-based dock content
- `OBSDock.cpp/hpp` - Base dock class
- `YouTubeAppDock.cpp/hpp` - YouTube integration dock
- `YouTubeChatDock.cpp/hpp` - YouTube chat dock

**Integration Point**: Bosh docks will follow this same pattern.

### CMake Integration:

```cmake
# frontend/CMakeLists.txt structure
add_executable(obs MACOSX_BUNDLE WIN32)
target_link_libraries(obs PRIVATE OBS::libobs OBS::frontend-api ...)
```

---

## 3. Core Library Analysis (libobs)

### Location: `libobs/`

The heart of OBS - handles video/audio pipelines, sources, scenes, outputs, encoders.

### Key Headers:

| File | Purpose |
|------|---------|
| `obs.h` | Main public API header (107KB) |
| `obs.hpp` | C++ wrapper headers |
| `obs-internal.h` | Internal structures (not for external use) |
| `obs-source.h` | Source API (scenes, cameras, captures, etc.) |
| `obs-scene.h` | Scene management |
| `obs-output.h` | Streaming/recording outputs |
| `obs-encoder.h` | Video/audio encoders |
| `obs-service.h` | Streaming services |
| `obs-hotkey.h` | Hotkey system |
| `obs-data.h` | Data serialization (JSON-like) |
| `obs-properties.h` | Property system for source configuration |
| `obs-module.h` | Plugin module system |
| `obs-audio-controls.h` | Audio metering/control |

### Core Systems:

#### 3.1 Video Pipeline
- `obs-video.c` - Video rendering pipeline
- `obs-video-gpu-encode.c` - GPU encoding helpers
- `obs-display.c` - Display preview handling

#### 3.2 Audio Pipeline
- `obs-audio.c` - Audio mixing and routing
- `obs-audio-controls.c` - Audio source controls

#### 3.3 Source System
- `obs-source.c` (176KB) - Source lifecycle, rendering, interaction
- `obs-source-transition.c` - Scene transitions
- `obs-source-deinterlace.c` - Deinterlacing

#### 3.4 Scene System
- `obs-scene.c` (112KB) - Scene graph, scene items, ordering

#### 3.5 Output System
- `obs-output.c` (95KB) - Streaming/recording output management
- `obs-output-delay.c` - Delay functionality

#### 3.6 Encoder System
- `obs-encoder.c` - Encoder abstraction

#### 3.7 Event System
Callbacks available via `obs_data.c` and signals in various modules.

### Subdirectories:

| Directory | Purpose |
|-----------|---------|
| `audio-monitoring/` | Audio monitoring backends (PulseAudio, etc.) |
| `callback/` | Signal/callback system |
| `graphics/` | Graphics abstraction (libobs graphics API) |
| `media-io/` | Media input/output handling |
| `util/` | Utility functions (hashing, threading, containers) |
| `pkgconfig/` | pkg-config files |
| `cmake/` | libobs CMake configuration |
| `data/` | Default locale strings |

---

## 4. Plugin System Analysis

### Location: `plugins/`

OBS uses a modular plugin architecture. Each plugin is a shared library loaded at runtime.

### Official Plugins:

| Plugin | Purpose |
|--------|---------|
| `aja/` | AJA hardware capture cards |
| `coreaudio-encoder/` | macOS CoreAudio encoder |
| `decklink/` | Blackmagic DeckLink support |
| `frontend-tools/` | Built-in tools (scene switcher, timers, scripts) |
| `image-source/` | Image/video file sources |
| `linux-*` | Linux-specific capture (ALSA, V4L2, PipeWire, etc.) |
| `mac-*` | macOS-specific capture and features |
| `nv-filters/` | NVIDIA broadcast filters |
| `obs-browser/` | CEF-based browser source (CRITICAL for Karma) |
| `obs-ffmpeg/` | FFmpeg-based sources/outputs |
| `obs-filters/` | Built-in video/audio filters |
| `obs-outputs/` | Additional output protocols |
| `obs-text/` | Text rendering sources |
| `obs-transitions/` | Transition effects |
| `obs-websocket/` | WebSocket remote control API |
| `obs-x264/` | x264 encoder |
| `rtmp-services/` | RTMP service definitions |
| `vlc-video/` | VLC-based video playback |
| `win-*` | Windows-specific capture (DirectShow, WASAPI, etc.) |

### Plugin CMake Pattern:

```cmake
add_obs_plugin(plugin-name
    PLATFORMS WINDOWS MACOS LINUX
    WITH_MESSAGE
)
```

Each plugin has:
- `CMakeLists.txt` - Build configuration
- `*.c` / `*.cpp` - Source files
- `data/` - Locale strings, icons
- `forms/` - Qt UI forms (if applicable)

### Critical Plugin: obs-browser

Location: `plugins/obs-browser/` (submodule - needs initialization)

This is the CEF (Chromium Embedded Framework) integration that powers:
- Browser Sources
- Browser Docks

**Strategy**: Reuse obs-browser for Karma overlay rendering instead of creating a separate browser engine.

---

## 5. Shared Libraries

### Location: `shared/`

Reusable code shared between frontend and plugins.

### Subdirectories:

| Directory | Purpose |
|-----------|---------|
| `qt/` | Qt utility libraries |
| `obs-scripting/` | Lua/Python scripting support |
| `properties-view/` | Property UI widget |
| `obfuscate/` | String obfuscation |

---

## 6. Build System Analysis

### Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.28...3.30)
project(obs-studio VERSION ${OBS_VERSION_CANONICAL})

add_subdirectory(libobs)
add_subdirectory(libobs-d3d11)      # Windows only
add_subdirectory(libobs-winrt)      # Windows only
add_subdirectory(libobs-opengl)
add_subdirectory(libobs-metal)      # macOS only
add_subdirectory(plugins)
add_subdirectory(test/test-input)
add_subdirectory(frontend)
```

### Key Build Variables:

- `ENABLE_FRONTEND` - Build Qt UI (default ON)
- `ENABLE_SCRIPTING` - Lua/Python support (default ON)
- `ENABLE_HEVC` - HEVC encoder support (default ON)
- `ENABLE_PLUGINS` - Plugin building (default ON)

### Platform Detection:

Build system detects:
- `OS_WINDOWS`, `OS_MACOS`, `OS_LINUX`, `OS_FREEBSD`, `OS_OPENBSD`
- Architecture: `x64`, `ARM64`, etc.

---

## 7. Data and Localization

### Locale System:

Each component has a `data/` directory with `.ini` files for localization:

```
data/
├── en-US/
│   ├── libobs.ini
│   └── frontend.ini
├── de-DE/
├── fr-FR/
└── ...
```

**Bosh Integration**: Add `bosh.ini` locale files.

---

## 8. Configuration System

OBS stores configuration in:
- Profiles (video, audio, output settings)
- Scene Collections (scenes, sources)
- Global settings (UI layout, hotkeys)

Configuration format: INI-style with `obs_data_t` serialization.

Location (typical):
- Windows: `%APPDATA%\obs-studio\`
- macOS: `~/Library/Application Support/obs-studio/`
- Linux: `~/.config/obs-studio/`

---

## 9. Event and Signal System

OBS uses a signal/callback system:

```c
// Example: Listen to source activation
obs_source_connect_private(source, on_source_activated, context);
```

Signals available for:
- Source activate/deactivate
- Scene item add/remove
- Output start/stop
- Audio volume changes
- Hotkey events

**Bosh Event Bus**: Will wrap and extend this system.

---

## 10. Threading Model

OBS has multiple threads:

| Thread | Purpose |
|--------|---------|
| Graphics Thread | OpenGL/D3D rendering |
| Video Thread | Video processing pipeline |
| Audio Thread | Audio mixing |
| UI Thread | Qt event loop |
| Output Thread | Encoding/streaming |

**Critical Rule**: Never block the graphics or video threads. Heavy operations must run on background workers.

---

## 11. Browser Integration (obs-browser)

The `obs-browser` plugin provides:

1. **Browser Source**: Renders web content as an OBS source
2. **Browser Dock**: Embeds web content in a dock panel
3. **Custom UI**: Panels can inject custom HTML/CSS/JS

**Karma Strategy**: Use existing browser source infrastructure for overlay rendering.

---

## 12. WebSocket API (obs-websocket)

Location: `plugins/obs-websocket/`

Provides remote control via WebSocket. Bosh internal components may use direct C++ calls instead of WebSocket for performance, but the protocol can inform internal API design.

---

## 13. Frontend Tools Plugin

Location: `plugins/frontend-tools/`

Contains built-in utilities:
- Auto Scene Switcher
- Output Timer
- Captions (Windows MSSAPI)
- Scripting (Lua/Python)

**Pattern**: This is the model for how Bosh plugin should be structured.

---

## 14. Integration Points Summary

### For Bosh Broadcast Pro:

| Component | Integration Location | Notes |
|-----------|---------------------|-------|
| **Bosh Plugin** | `plugins/bosh/` | New top-level plugin directory |
| **Bosh Docks** | `frontend/docks/` OR `plugins/bosh/ui/` | Follow OBSDock pattern |
| **Bosh Menu** | `frontend/widgets/OBSBasic.cpp` | Add Bosh menu to main window |
| **Karma Overlays** | Via obs-browser | Use browser source, no new engine needed |
| **Event Bus** | `plugins/bosh/events/` | Wrap OBS signals + add new events |
| **AI Engine** | `plugins/bosh/ai/` | Background worker threads |
| **Chat System** | `plugins/bosh/chat/` | Native dock + platform adapters |
| **Alerts** | `plugins/bosh/alerts/` | Trigger Karma overlays |
| **Settings** | `frontend/settings/` OR plugin properties | Integrate with OBS settings |
| **Locale** | `plugins/bosh/data/` | Standard OBS localization |
| **Build** | `plugins/CMakeLists.txt` | Add bosh plugin to build |

---

## 15. Constraints and Guidelines

### DO:
- Create isolated `plugins/bosh/` directory
- Use OBS module API (`obs_module_t`)
- Follow OBS naming conventions
- Use `obs_data_t` for configuration
- Leverage existing browser integration
- Add docks following `OBSDock` pattern
- Use OBS signal system for events
- Run heavy work on background threads
- Respect OBS threading requirements

### DO NOT:
- Modify `libobs/` unless absolutely necessary
- Block the graphics thread
- Create duplicate browser engines
- Store secrets in source code
- Break existing OBS functionality
- Ignore cross-platform considerations

---

## 16. Next Steps

1. Create `docs/bosh/` documentation directory
2. Initialize `plugins/bosh/` with proper CMake structure
3. Implement minimal Bosh plugin that loads successfully
4. Add first Bosh dock
5. Create Bosh event bus
6. Integrate Karma overlay system via browser source
7. Build vertical slice: Event → Alert → Overlay → Preview

---

## Appendix A: File Size Reference

| File | Size | Significance |
|------|------|--------------|
| `obs.h` | 107 KB | Main API surface |
| `obs-source.c` | 176 KB | Source complexity |
| `obs-scene.c` | 112 KB | Scene complexity |
| `obs-output.c` | 95 KB | Output complexity |
| `OBSApp.cpp` | 54 KB | Frontend app |
| `obs-video.c` | 36 KB | Video pipeline |

---

## Appendix B: Key OBS APIs for Bosh

```c
// Module initialization
bool obs_module_load(void);
void obs_module_unload(void);

// Source creation
obs_source_t *obs_source_create(const char *id, const char *name, ...);
void obs_source_release(obs_source_t *source);

// Scene manipulation
obs_scene_t *obs_scene_create(const char *name);
obs_sceneitem_t *obs_scene_add(obs_scene_t *scene, obs_source_t *source);

// Signal connections
signal_handler_t *obs_source_get_signal_handler(obs_source_t *source);
void signal_handler_connect(signal_handler_t *sh, const char *signal, ...);

// Properties
obs_properties_t *obs_source_properties(obs_source_t *source);
void obs_source_update(obs_source_t *source, obs_data_t *settings);

// Hotkeys
obs_hotkey_id obs_register_hotkey(...);
```

---

*Document generated from OBS Studio source audit.*
