# OBS Integration Map for Bosh Broadcast Pro

## Purpose

This document maps exactly where and how Bosh Broadcast Pro features will integrate into the OBS Studio source tree.

---

## 1. Source Tree Modifications

### 1.1 New Directories to Create

```
obs-studio/
├── plugins/
│   └── bosh/                          # NEW - Main Bosh plugin
│       ├── CMakeLists.txt
│       ├── bosh-plugin.cpp            # Module entry point
│       ├── bosh-plugin.h
│       ├── core/                      # Core Bosh functionality
│       │   ├── bosh-core.cpp
│       │   ├── bosh-core.h
│       │   └── ...
│       ├── ai/                        # AI engine
│       │   ├── ai-engine.cpp
│       │   ├── ai-engine.h
│       │   ├── ollama-adapter.cpp
│       │   ├── ollama-adapter.h
│       │   └── ...
│       ├── agents/                    # Multi-agent system
│       │   ├── agent-bus.cpp
│       │   ├── agent-bus.h
│       │   ├── master-agent.cpp
│       │   └── ...
│       ├── events/                    # Event bus
│       │   ├── event-bus.cpp
│       │   ├── event-bus.h
│       │   └── ...
│       ├── alerts/                    # Alert engine
│       │   ├── alert-manager.cpp
│       │   └── ...
│       ├── chat/                      # Unified chat
│       │   ├── chat-dock.cpp
│       │   ├── chat-manager.cpp
│       │   └── ...
│       ├── overlays/                  # Karma overlay engine
│       │   ├── karma-manager.cpp
│       │   ├── karma-template.cpp
│       │   ├── karma-theme.cpp
│       │   └── ...
│       ├── automation/                # Scene automation
│       │   ├── rule-engine.cpp
│       │   └── ...
│       ├── analytics/                 # Analytics engine
│       │   └── ...
│       ├── clips/                     # Clip factory
│       │   └── ...
│       ├── captions/                  # Real-time captions
│       │   └── ...
│       ├── translation/               # Translation engine
│       │   └── ...
│       ├── scheduler/                 # Broadcast scheduler
│       │   └── ...
│       ├── platforms/                 # Platform adapters
│       │   ├── platform-adapter.h
│       │   ├── twitch-adapter.cpp
│       │   ├── youtube-adapter.cpp
│       │   └── ...
│       ├── security/                  # Security & permissions
│       │   └── ...
│       ├── diagnostics/               # Diagnostics engine
│       │   └── ...
│       ├── ui/                        # Bosh docks and UI
│       │   ├── bosh-dashboard.cpp
│       │   ├── bosh-dashboard.h
│       │   ├── bosh-chat-dock.cpp
│       │   ├── bosh-alert-dock.cpp
│       │   ├── bosh-ai-dock.cpp
│       │   ├── bosh-overlay-dock.cpp
│       │   ├── bosh-analytics-dock.cpp
│       │   ├── bosh-automation-dock.cpp
│       │   ├── bosh-scheduler-dock.cpp
│       │   ├── bosh-diagnostics-dock.cpp
│       │   └── ...
│       ├── data/                      # Locale and resources
│       │   └── en-US/
│       │       └── bosh.ini
│       └── testing/                   # Test utilities
│           └── ...
│
├── frontend/
│   ├── docks/
│   │   └── (optional Bosh docks if integrated here)
│   └── widgets/
│       └── (optional Bosh widgets if integrated here)
│
└── docs/
    └── bosh/                          # NEW - Bosh documentation
        ├── README.md
        ├── ARCHITECTURE.md
        ├── OBS_INTEGRATION.md
        ├── KARMA.md
        ├── AI.md
        ├── AGENTS.md
        ├── CHAT.md
        ├── ALERTS.md
        ├── CLIPS.md
        ├── CAPTIONS.md
        ├── ANALYTICS.md
        ├── AUTOMATION.md
        ├── SECURITY.md
        ├── TESTING.md
        ├── BUILDING.md
        └── UPSTREAM_SYNC.md
```

### 1.2 Files to Modify

#### Minimal Modifications Preferred

| File | Modification | Reason |
|------|-------------|--------|
| `plugins/CMakeLists.txt` | Add `add_obs_plugin(bosh ...)` | Register Bosh plugin in build |
| `frontend/widgets/OBSBasic.cpp` | Add Bosh menu (optional) | Main menu integration |
| `frontend/OBSApp.cpp` | (Optional) Initialize Bosh on startup | Auto-load Bosh subsystem |

**Philosophy**: Keep modifications to existing OBS files minimal. Prefer plugin-based approach.

---

## 2. Build System Integration

### 2.1 plugins/CMakeLists.txt Modification

Add after existing plugins (alphabetical order):

```cmake
# Add Bosh Broadcast Pro plugin
add_obs_plugin(
    bosh
    PLATFORMS WINDOWS MACOS LINUX
    WITH_MESSAGE
)
```

### 2.2 plugins/bosh/CMakeLists.txt Structure

```cmake
cmake_minimum_required(VERSION 3.28...3.30)

find_package(Qt6 REQUIRED Widgets Network)

# Find Ollama SDK if available (optional)
find_package(Ollama QUIET)

add_library(bosh MODULE)
add_library(OBS::bosh ALIAS bosh)

# Core sources
target_sources(
    bosh
    PRIVATE
        bosh-plugin.cpp
        bosh-plugin.h
        core/bosh-core.cpp
        core/bosh-core.h
        events/event-bus.cpp
        events/event-bus.h
        ai/ai-engine.cpp
        ai/ai-engine.h
        ai/ollama-adapter.cpp
        ai/ollama-adapter.h
        agents/agent-bus.cpp
        agents/agent-bus.h
        agents/master-agent.cpp
        agents/master-agent.h
        alerts/alert-manager.cpp
        alerts/alert-manager.h
        chat/chat-manager.cpp
        chat/chat-manager.h
        overlays/karma-manager.cpp
        overlays/karma-manager.h
        overlays/karma-template.cpp
        overlays/karma-template.h
        overlays/karma-theme.cpp
        overlays/karma-theme.h
        automation/rule-engine.cpp
        automation/rule-engine.h
        # ... additional modules
)

# UI forms
target_sources(
    bosh
    PRIVATE
        ui/forms/bosh-dashboard.ui
        ui/forms/bosh-chat-dock.ui
        ui/forms/bosh-alert-dock.ui
        ui/forms/bosh-ai-dock.ui
        ui/forms/bosh-overlay-dock.ui
        ui/forms/bosh-analytics-dock.ui
        ui/forms/bosh-automation-dock.ui
        ui/forms/bosh-scheduler-dock.ui
        ui/forms/bosh-diagnostics-dock.ui
)

# Dock implementations
target_sources(
    bosh
    PRIVATE
        ui/bosh-dashboard.cpp
        ui/bosh-dashboard.h
        ui/bosh-chat-dock.cpp
        ui/bosh-chat-dock.h
        ui/bosh-alert-dock.cpp
        ui/bosh-alert-dock.h
        ui/bosh-ai-dock.cpp
        ui/bosh-ai-dock.h
        ui/bosh-overlay-dock.cpp
        ui/bosh-overlay-dock.h
        ui/bosh-analytics-dock.cpp
        ui/bosh-analytics-dock.h
        ui/bosh-automation-dock.cpp
        ui/bosh-automation-dock.h
        ui/bosh-scheduler-dock.cpp
        ui/bosh-scheduler-dock.h
        ui/bosh-diagnostics-dock.cpp
        ui/bosh-diagnostics-dock.h
)

target_link_libraries(
    bosh
    PRIVATE
        OBS::frontend-api
        OBS::libobs
        OBS::qt-wrappers
        Qt::Widgets
        Qt::Network
        $<$<BOOL:${Ollama_FOUND}>:Ollama::Ollama>
)

# Platform-specific resources
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
               AUTOUIC_SEARCH_PATHS "ui/forms"
               XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)
```

---

## 3. Integration Points by Feature

### 3.1 Bosh Plugin Core

**Location**: `plugins/bosh/bosh-plugin.cpp`

```cpp
#include <obs-module.h>
#include "core/bosh-core.h"
#include "events/event-bus.h"
#include "ui/bosh-dashboard.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("bosh", "en-US")

static BoshCore *bosh_core = nullptr;
static EventBus *event_bus = nullptr;

bool obs_module_load(void)
{
    blog(LOG_INFO, "[Bosh] Loading Bosh Broadcast Pro module");
    
    // Initialize core
    bosh_core = new BoshCore();
    
    // Initialize event bus
    event_bus = new EventBus();
    
    // Register docks (called from UI thread)
    obs_queue_task(OBS_TASK_UI, register_bosh_docks, nullptr, false);
    
    // Register hotkeys
    register_bosh_hotkeys();
    
    blog(LOG_INFO, "[Bosh] Bosh Broadcast Pro loaded successfully");
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[Bosh] Unloading Bosh Broadcast Pro module");
    
    delete event_bus;
    delete bosh_core;
    
    blog(LOG_INFO, "[Bosh] Bosh Broadcast Pro unloaded");
}
```

### 3.2 Event Bus Integration

**Location**: `plugins/bosh/events/event-bus.cpp`

Connects to OBS signals and emits Bosh events:

```cpp
void EventBus::connectToOBS()
{
    // Connect to scene change events
    auto main_view = obs_frontend_get_main_window();
    obs_frontend_add_event_callback([](obs_frontend_event event, void *priv) {
        EventBus *bus = static_cast<EventBus*>(priv);
        
        switch(event) {
            case OBS_FRONTEND_EVENT_SCENE_CHANGED:
                bus->emit(SceneChangedEvent{...});
                break;
            case OBS_FRONTEND_EVENT_STREAMING_STARTING:
                bus->emit(StreamStartingEvent{...});
                break;
            case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
                bus->emit(StreamStoppedEvent{...});
                break;
            // ... more events
        }
    }, this);
    
    // Connect to source signals
    // Connect to output signals
}
```

### 3.3 Karma Overlay Integration

**Strategy**: Use existing `obs-browser` plugin infrastructure.

**Location**: `plugins/bosh/overlays/karma-manager.cpp`

```cpp
class KarmaManager {
public:
    void createOverlaySource(const QString &templateId, const QString &themeId)
    {
        // Create browser source with Karma URL
        obs_data_t *settings = obs_data_create();
        obs_data_set_string(settings, "url", karmaUrl.toUtf8().constData());
        obs_data_set_int(settings, "width", 1920);
        obs_data_set_int(settings, "height", 1080);
        obs_data_set_bool(settings, "shutdown_source", true);
        
        obs_source_t *source = obs_source_create("browser_source", 
                                                  "Karma Overlay", 
                                                  settings, nullptr);
        
        // Add to appropriate scene
        obs_scene_t *scene = obs_frontend_get_current_scene();
        obs_sceneitem_t *item = obs_scene_add(scene, source);
        
        obs_source_release(source);
        obs_scene_release(scene);
        obs_data_release(settings);
    }
    
    void triggerAlert(const Alert &alert)
    {
        // Send message to browser source via postMessage
        // Browser source executes JavaScript to show animated alert
    }
};
```

### 3.4 Bosh Docks

**Pattern**: Follow `frontend/docks/OBSDock.cpp` pattern.

**Location**: `plugins/bosh/ui/bosh-dashboard.cpp`

```cpp
#include "docks/OBSDock.hpp"

class BoshDashboardDock : public OBSDock {
    Q_OBJECT
    
public:
    BoshDashboardDock(QWidget *parent = nullptr)
        : OBSDock(parent)
    {
        setWindowTitle("Bosh Broadcast Center");
        setObjectName("BoshDashboardDock");
        
        // Create dashboard UI
        setupUi();
    }
    
private:
    void setupUi()
    {
        // Create sections: LIVE, CHAT, ALERTS, SCENES, etc.
        // Use Qt layouts
    }
};

// Registration function called from plugin load
void registerBoshDocks()
{
    auto dock = new BoshDashboardDock();
    obs_frontend_add_dock(dock);
}
```

### 3.5 AI Engine Integration

**Location**: `plugins/bosh/ai/ai-engine.cpp`

Runs on background thread:

```cpp
class AIEngine : public QObject {
    Q_OBJECT
    
public slots:
    void processCommand(const QString &command)
    {
        // Run AI inference on worker thread
        QtConcurrent::run([=]() {
            auto response = ollamaAdapter_->generate(command, context_);
            emit responseReady(response);
        });
    }
    
    void analyzeChat(const ChatMessage &message)
    {
        // Classify message toxicity, spam, etc.
        // Never block UI thread
    }
};
```

### 3.6 Platform Adapters

**Location**: `plugins/bosh/platforms/`

```cpp
class PlatformAdapter {
public:
    virtual ~PlatformAdapter() = default;
    virtual QString platformName() const = 0;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual QList<ChatMessage> fetchChat() = 0;
    virtual StreamHealth getStreamHealth() = 0;
};

class TwitchAdapter : public PlatformAdapter {
    // Twitch API implementation
};

class YouTubeAdapter : public PlatformAdapter {
    // YouTube API implementation
};
```

---

## 4. Data Flow Architecture

### 4.1 Event Flow

```
External Event (e.g., Twitch Subscription)
         ↓
Platform Adapter (receives webhook/API event)
         ↓
Validation & Normalization
         ↓
Bosh Event Bus
         ↓
    ┌─────────────┬─────────────┬──────────────┐
    ↓             ↓             ↓              ↓
Alert Agent   Chat Agent   Analytics Agent  AI Agent
    ↓             ↓             ↓              ↓
Karma Template  Chat Dock   Metrics Store   Decision
    ↓
OBS Browser Source (renders overlay)
    ↓
OBS Preview/Program Output
```

### 4.2 Command Flow (AI Copilot)

```
User Voice/Text Command
         ↓
AI Engine (intent recognition)
         ↓
Permission Check
         ↓
OBS API Call (via libobs)
         ↓
State Verification
         ↓
Execute Action
         ↓
Confirmation to User
```

---

## 5. Thread Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    UI Thread (Qt)                        │
│  - Docks                                                 │
│  - Dashboard                                             │
│  - Settings dialogs                                      │
│  - OBS preview widget                                    │
└─────────────────────────────────────────────────────────┘
                          ↕ (queued connections)
┌─────────────────────────────────────────────────────────┐
│                  OBS Video Thread                        │
│  - Scene rendering                                       │
│  - Source updates                                        │
└─────────────────────────────────────────────────────────┘
                          ↕
┌─────────────────────────────────────────────────────────┐
│                  OBS Audio Thread                        │
│  - Audio mixing                                          │
│  - Metering                                              │
└─────────────────────────────────────────────────────────┘
                          ↕
┌─────────────────────────────────────────────────────────┐
│              Bosh Worker Threads                         │
│  - AI inference (Ollama)                                 │
│  - Platform API polling                                  │
│  - Analytics processing                                  │
│  - Clip detection                                        │
│  - Caption generation                                    │
│  - Translation                                           │
└─────────────────────────────────────────────────────────┘
```

**Critical**: All OBS API calls that affect rendering must be made from the correct thread using `obs_queue_task()`.

---

## 6. Configuration Storage

### 6.1 Bosh Settings Location

Bosh stores configuration alongside OBS profiles:

```
obs-studio-config/
├── basic/
│   ├── profiles/
│   │   └── <profile-name>/
│   │       ├── bosh.json          # Bosh profile settings
│   │       └── ...
│   └── scenecollections/
│       └── <collection-name>/
│           └── ...
└── global/
    └── bosh.json                   # Global Bosh settings
```

### 6.2 Settings Schema

```json
{
  "bosh": {
    "version": 1,
    "ai": {
      "enabled": true,
      "model": "llama3.1:8b",
      "endpoint": "http://localhost:11434"
    },
    "chat": {
      "platforms": [],
      "moderation_mode": "observe"
    },
    "overlays": {
      "karma_enabled": true,
      "default_theme": "karma-neon"
    },
    "alerts": {
      "cooldown_ms": 5000,
      "queue_size": 10
    },
    "platforms": {
      "twitch": { "connected": false },
      "youtube": { "connected": false }
    }
  }
}
```

---

## 7. Localization Strategy

### 7.1 Locale Files

Create `plugins/bosh/data/en-US/bosh.ini`:

```ini
[General]
ModuleName="Bosh Broadcast Pro"

[BoshDashboard]
Title="Bosh Broadcast Center"
LiveSection="LIVE"
ChatSection="CHAT"
AlertsSection="ALERTS"
ScenesSection="SCENES"
AISession="AI"

[Commands]
SwitchScene="Switch to scene: %1"
MuteMic="Mute microphone"
UnmuteMic="Unmute microphone"
StartStreaming="Start streaming"
StopStreaming="Stop streaming"

[Alerts]
NewSubscriber="New subscriber: %1"
NewFollower="New follower: %1"
NewDonation="New donation: %1 %2 from %3"
```

Support multiple languages following OBS convention.

---

## 8. Hotkey Integration

Register Bosh commands with OBS hotkey system:

```cpp
void registerBoshHotkeys()
{
    obs_hotkey_register_pair(
        "bosh.trigger_alert",
        "Trigger Test Alert",
        []() {
            EventBus::instance()->emit(TestAlertEvent{});
        }
    );
    
    obs_hotkey_register_pair(
        "bosh.toggle_captions",
        "Toggle Real-Time Captions",
        []() {
            CaptionEngine::toggle();
        }
    );
    
    // ... more hotkeys
}
```

---

## 9. Testing Strategy

### 9.1 Unit Tests

Location: `plugins/bosh/testing/`

Test:
- Event bus routing
- Alert queue management
- Template variable substitution
- Platform adapter parsing

### 9.2 Integration Tests

Test:
- Event → Alert → Overlay → Render pipeline
- OBS state changes trigger Bosh events
- Bosh commands affect OBS state

### 9.3 Manual Test Mode

Include test buttons in Diagnostics dock:
- Test Subscriber Event
- Test Follower Event
- Test Donation Event
- Test Overlay Rendering
- Test AI Response
- Test Platform Connection

---

## 10. Security Considerations

### 10.1 Secret Storage

Never store API keys in:
- Source code
- Configuration files (unencrypted)
- Logs

Use OBS's secure storage mechanism or OS keychain.

### 10.2 Permission Levels

```cpp
enum class BoshPermission {
    READ_ONLY,          // View state only
    OBS_CONTROL,        // Change scenes, sources
    CHAT_CONTROL,       // Moderation actions
    OVERLAY_CONTROL,    // Modify overlays
    AI_CONTROL,         // AI command execution
    PLATFORM_CONTROL,   // Platform API calls
    PUBLISH,            // Start/stop streaming
    ADMIN               // Full access
};
```

### 10.3 Input Validation

All external input (chat messages, platform events, AI responses) must be:
- Validated
- Sanitized
- Rate-limited
- Logged (without secrets)

---

## 11. Performance Guidelines

### 11.1 Critical Path Rules

1. **Never block** the OBS video thread
2. **Never block** the OBS audio thread
3. **Never block** the UI thread for >100ms
4. AI inference must run on **background threads**
5. Video processing must use **async workers**

### 11.2 Resource Limits

```cpp
constexpr int MAX_ALERT_QUEUE_SIZE = 20;
constexpr int MAX_CHAT_BUFFER_SIZE = 1000;
constexpr int MAX_AI_REQUEST_TIMEOUT_MS = 5000;
constexpr int MAX_PLATFORM_RETRY_COUNT = 3;
```

### 11.3 Memory Management

- Use RAII patterns
- Release OBS objects properly (`obs_source_release`, etc.)
- Avoid memory leaks in long-running streams
- Monitor memory usage in Diagnostics

---

## 12. Upstream Sync Strategy

### 12.1 Maintain Separation

Keep Bosh changes isolated:
- `plugins/bosh/` - All Bosh code
- Minimal changes to existing OBS files
- Clear commit messages: `[BOSH]` prefix

### 12.2 Rebase Strategy

When upstream OBS updates:

```bash
# Fetch upstream
git fetch upstream master

# Rebase Bosh changes
git rebase upstream/master

# Resolve conflicts (prefer upstream for core files)
# Test thoroughly
```

### 12.3 Patch Documentation

Maintain `docs/bosh/PATCH_STRATEGY.md` documenting:
- Which files were modified
- Why modification was necessary
- Alternative approaches considered
- Compatibility notes

---

## 13. First Milestone Checklist

- [ ] Create `plugins/bosh/` directory structure
- [ ] Implement minimal `bosh-plugin.cpp` that loads
- [ ] Add plugin to `plugins/CMakeLists.txt`
- [ ] Build OBS with Bosh plugin
- [ ] Verify plugin appears in OBS plugin list
- [ ] Create first dock (Bosh Dashboard)
- [ ] Implement Event Bus
- [ ] Create Karma Manager (browser source wrapper)
- [ ] Implement test alert event
- [ ] Verify alert renders in OBS preview
- [ ] Pass OBS stability test

---

*This map evolves as implementation progresses.*
