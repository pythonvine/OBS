# Bosh Broadcast Pro - Dependency Map

## Core Dependencies

### Build System

| Dependency | Version | Purpose | Required |
|------------|---------|---------|----------|
| CMake | 3.28-3.30 | Build system | Yes |
| Qt | 6.x | UI framework | Yes |
| C++ Compiler | C++17 | Language standard | Yes |
| Python | 3.x | Build scripts | Yes |

### OBS Internal Dependencies

| Module | Location | Purpose |
|--------|----------|---------|
| libobs | `libobs/` | Core OBS functionality |
| frontend-api | `frontend/` | OBS frontend API |
| obs-browser | `plugins/obs-browser/` | Browser source for Karma |
| qt-wrappers | `shared/qt/wrappers/` | Qt utilities |

### Runtime Dependencies

| Dependency | Purpose | Optional | Notes |
|------------|---------|----------|-------|
| Ollama | Local AI inference | Yes | For AI features |
| CEF | Browser rendering | No | Via obs-browser |
| FFmpeg | Video processing | No | Via OBS core |

---

## Bosh Plugin Dependencies

### Qt Modules

```cmake
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Concurrent
)
```

### Optional External Libraries

| Library | Purpose | Fallback |
|---------|---------|----------|
| Ollama SDK | Local AI | Cloud AI APIs |
| libwebsockets | Real-time chat | HTTP polling |
| nlohmann/json | JSON handling | Qt JSON classes |

---

## Platform-Specific Dependencies

### Windows

- Visual Studio 2022
- Windows SDK 10+
- DirectShow (video capture)
- WASAPI (audio capture)

### macOS

- Xcode 15+
- macOS SDK 11+
- AVFoundation (capture)
- CoreAudio (audio)

### Linux

- GCC 11+ or Clang 14+
- X11 or Wayland
- PipeWire or PulseAudio
- V4L2 (video)

---

## Dependency Graph

```
Bosh Plugin
├── OBS Core (libobs)
│   ├── Video Pipeline
│   ├── Audio Pipeline
│   └── Source System
├── OBS Frontend
│   ├── Dock System
│   └── Event System
├── Qt Framework
│   ├── Widgets (UI)
│   ├── Network (HTTP)
│   └── Concurrent (threads)
├── obs-browser (CEF)
│   └── Karma Overlays
└── Optional: Ollama
    └── AI Features
```

---

## Version Compatibility

### OBS Versions

| Bosh Version | Min OBS | Max OBS | Notes |
|--------------|---------|---------|-------|
| 1.0 | 30.0 | 31.x | Initial release |

### Qt Versions

| Qt Version | Support | Notes |
|------------|---------|-------|
| 6.5+ | Full | Recommended |
| 6.2-6.4 | Partial | May need workarounds |
| < 6.2 | None | Not supported |

---

## Installation Dependencies

### User Must Install

| Software | Purpose | Link |
|----------|---------|------|
| OBS Studio | Base application | obsproject.com |
| Ollama (optional) | AI features | ollama.ai |

### Bundled with Bosh

| Component | Purpose |
|-----------|---------|
| Karma templates | Overlay designs |
| Karma themes | Visual themes |
| Sound effects | Alert sounds |
| Locale files | Translations |

---

## Development Dependencies

### Required Tools

- Git
- CMake
- Qt Creator (recommended)
- OBS Studio build environment

### Optional Tools

- Qt Designer (UI design)
- Performance profilers
- Memory leak detectors

---

*Update this document when adding new dependencies.*
