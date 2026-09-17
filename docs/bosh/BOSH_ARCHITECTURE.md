# Bosh Broadcast Pro Architecture

## System Overview

Bosh Broadcast Pro is a deep integration of broadcast automation, AI assistance, and audience engagement features directly into OBS Studio.

```
┌─────────────────────────────────────────────────────────────────┐
│                    OBS Studio Core                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │
│  │ Video       │  │ Audio       │  │ Scene/Source            │ │
│  │ Pipeline    │  │ Pipeline    │  │ Management              │ │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                  Bosh Plugin Layer                              │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Event    │  │ AI       │  │ Karma    │  │ Multi-Agent    │  │
│  │ Bus      │  │ Engine   │  │ Overlays │  │ System         │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Chat     │  │ Alerts   │  │ Platform │  │ Automation     │  │
│  │ System   │  │ Engine   │  │ Adapters │  │ Engine         │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Clips    │  │ Captions │  │ Analytics│  │ Diagnostics    │  │
│  │ Factory  │  │ Engine   │  │ Engine   │  │ Engine         │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                   UI Integration Layer                          │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Bosh     │  │ Chat     │  │ Alert    │  │ AI Copilot     │  │
│  │ Dashboard│  │ Dock     │  │ Dock     │  │ Dock           │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Overlay  │  │Analytics │  │Automation│  │ Diagnostics    │  │
│  │ Dock     │  │ Dock     │  │ Dock     │  │ Dock           │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Core Principles

### 1. Non-Destructive Integration

- Bosh enhances OBS without breaking existing functionality
- OBS must remain stable even if Bosh components fail
- No modifications to libobs unless absolutely necessary

### 2. Modular Architecture

- Each Bosh subsystem is independently testable
- Components communicate via well-defined interfaces
- Event bus decouples producers from consumers

### 3. Thread Safety

- Heavy operations run on background workers
- OBS API calls respect threading requirements
- UI updates queued to main thread

### 4. Failure Isolation

```
AI Failure → Bosh continues
Ollama Failure → AI features disabled, rest works
Overlay Failure → OBS continues streaming
Platform Failure → Other platforms unaffected
Bosh Failure → OBS core remains stable
```

### 5. Security First

- No secrets in source code
- Permission-based access control
- Input validation on all external data
- Audit logging for sensitive actions

---

## Subsystem Architecture

### 1. Bosh Core

**Location**: `plugins/bosh/core/`

Responsibilities:
- Plugin initialization/shutdown
- Global state management
- Configuration loading/saving
- Inter-subsystem coordination

```cpp
class BoshCore {
public:
    static BoshCore* instance();
    
    bool initialize();
    void shutdown();
    
    Config* config() const;
    EventBus* eventBus() const;
    KarmaManager* karmaManager() const;
    
private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<EventBus> eventBus_;
    std::unique_ptr<KarmaManager> karmaManager_;
};
```

---

### 2. Event Bus

**Location**: `plugins/bosh/events/`

The central nervous system of Bosh. All events flow through here.

#### Event Schema

```json
{
  "id": "uuid-v4",
  "type": "subscriber.new",
  "source": "twitch-adapter",
  "timestamp": "2024-01-15T10:30:00Z",
  "priority": 5,
  "payload": {
    "username": "viewer123",
    "displayName": "Viewer123",
    "platform": "twitch",
    "tier": "1000"
  },
  "correlation_id": "parent-event-id"
}
```

#### Event Types

| Category | Events |
|----------|--------|
| **Platform** | follower.new, subscriber.new, donation.received, raid.started |
| **Chat** | chat.message, chat.deleted, chat.timeout, chat.ban |
| **OBS** | scene.changed, stream.started, stream.stopped, source.muted |
| **AI** | ai.command, ai.response, ai.suggestion |
| **System** | timer.elapsed, alert.triggered, clip.created |
| **Audio** | audio.peaked, audio.silence, audio.disconnected |

#### Implementation

```cpp
class EventBus : public QObject {
    Q_OBJECT
    
public:
    static EventBus* instance();
    
    void publish(Event event);
    void subscribe(EventType type, QObject* handler);
    void unsubscribe(QObject* handler);
    
signals:
    void eventReceived(const Event& event);
    
private:
    void validateEvent(Event& event);
    void deduplicate(Event& event);
    void routeToHandlers(const Event& event);
    
    QHash<EventType, QList<QObject*>> handlers_;
    QSet<QString> recentEventIds_;
    QQueue<Event> priorityQueue_;
};
```

---

### 3. AI Engine

**Location**: `plugins/bosh/ai/`

Provides AI capabilities for broadcast assistance.

#### Components

```
AI Engine
├── OllamaAdapter (local inference)
├── CommandParser (intent recognition)
├── ContextManager (broadcast state)
├── ResponseGenerator (formatted output)
└── SafetyFilter (content moderation)
```

#### Capabilities

| Capability | Description |
|------------|-------------|
| **Broadcast Commands** | "Switch to gameplay scene", "Mute my mic" |
| **Chat Analysis** | Sentiment, toxicity, spam detection |
| **Clip Detection** | Identify highlight moments |
| **Audience Insights** | Engagement trends, active viewers |
| **Moderation Support** | Auto-timeout suggestions |
| **Caption Enhancement** | Improve auto-generated captions |

#### Ollama Integration

```cpp
class OllamaAdapter : public QObject {
    Q_OBJECT
    
public:
    explicit OllamaAdapter(const QString& endpoint = "http://localhost:11434");
    
    bool healthCheck();
    QStringList availableModels();
    
    void generate(const QString& prompt, const GenerateOptions& options);
    void generateStream(const QString& prompt, const GenerateOptions& options);
    
signals:
    void responseReady(const QString& response);
    void responseChunk(const QString& chunk);
    void error(const QString& message);
    
private:
    QNetworkAccessManager* network_;
    QString endpoint_;
    QString currentModel_;
};
```

---

### 4. Multi-Agent System

**Location**: `plugins/bosh/agents/`

Specialist AI agents coordinated by a Master Agent.

#### Agent Roster

| Agent | Responsibility |
|-------|----------------|
| **Master Agent** | Task planning, delegation, QA orchestration |
| **Architect Agent** | Code review, architecture compliance |
| **OBS Agent** | OBS state monitoring, scene management |
| **Karma Agent** | Overlay templates, theme management |
| **Chat Agent** | Chat analysis, moderation |
| **Alert Agent** | Alert queue, trigger logic |
| **AI Agent** | LLM coordination, prompt engineering |
| **Moderation Agent** | Toxicity detection, rule enforcement |
| **Clip Agent** | Moment detection, clip creation |
| **Caption Agent** | Speech-to-text, caption timing |
| **Translation Agent** | Multi-language translation |
| **Analytics Agent** | Metrics collection, insights |
| **Automation Agent** | Rule execution, triggers |
| **Scheduler Agent** | Broadcast scheduling |
| **Platform Agent** | Platform API connections |
| **Audio Agent** | Audio monitoring, alerts |
| **Video Agent** | Video quality monitoring |
| **Security Agent** | Permission checks, audit logs |
| **Performance Agent** | Resource monitoring |
| **QA Agent** | Testing, validation |
| **Release Agent** | Build verification |

#### Agent Communication

```cpp
struct AgentMessage {
    QString id;
    MessageType type;  // TASK, STATUS, RESULT, REVIEW, QA, etc.
    QString fromAgent;
    QString toAgent;
    QVariant payload;
    qint64 timestamp;
    QString correlationId;
};

class AgentBus : public QObject {
    Q_OBJECT
    
public:
    void sendMessage(const AgentMessage& message);
    void registerAgent(const QString& agentId, Agent* agent);
    
signals:
    void messageReceived(const AgentMessage& message);
};
```

---

### 5. Karma Overlay Engine

**Location**: `plugins/bosh/overlays/`

Manages animated overlays rendered via obs-browser.

#### Architecture

```
Karma Manager
├── Template Engine
│   ├── Variable substitution
│   ├── Conditional rendering
│   └── Animation triggers
├── Theme Engine
│   ├── Built-in themes (Neon, Cyber, Gaming, etc.)
│   ├── Theme inheritance
│   └── Custom themes
├── Animation Engine
│   ├── CSS animations
│   ├── JavaScript timelines
│   └── Sound effects
└── Runtime
    ├── Browser source wrapper
    ├── Message passing (postMessage)
    └── State synchronization
```

#### Template Structure

```json
{
  "id": "subscriber-alert-v1",
  "name": "Subscriber Alert",
  "category": "engagement",
  "version": "1.0",
  "resolution": {"width": 1920, "height": 1080},
  "variables": [
    {"name": "username", "type": "string", "required": true},
    {"name": "displayName", "type": "string", "required": true},
    {"name": "tier", "type": "string", "default": "1000"},
    {"name": "message", "type": "string", "optional": true}
  ],
  "assets": {
    "html": "templates/subscriber/alert.html",
    "css": "templates/subscriber/alert.css",
    "js": "templates/subscriber/alert.js",
    "sound": "sounds/subscriber-alert.mp3"
  },
  "animation": {
    "duration": 5000,
    "enter": "slide-in-left",
    "exit": "fade-out",
    "cooldown": 3000
  },
  "priority": 5
}
```

#### Built-in Themes

- Karma Neon
- Karma Cyber
- Karma Gaming
- Karma Creator
- Karma Podcast
- Karma News
- Karma Minimal
- Karma Futuristic

---

### 6. Alert Engine

**Location**: `plugins/bosh/alerts/`

Manages alert queuing, prioritization, and triggering.

#### Pipeline

```
Event Received
     ↓
Validation (schema, duplicates)
     ↓
Priority Assignment
     ↓
Rule Evaluation
     ↓
AI Enhancement (optional)
     ↓
Alert Queue (bounded)
     ↓
Cooldown Check
     ↓
Trigger Karma Overlay
     ↓
Play Sound / TTS
     ↓
Log & Analytics
```

#### Alert Queue

```cpp
class AlertQueue {
public:
    void enqueue(const Alert& alert);
    Alert dequeue();
    bool isEmpty() const;
    int size() const;
    
    void setMaxSize(int size);
    void setCooldownMs(int ms);
    
private:
    QQueue<Alert> highPriority_;
    QQueue<Alert> normalPriority_;
    QQueue<Alert> lowPriority_;
    
    int maxSize_ = 20;
    int cooldownMs_ = 5000;
    QTimer* cooldownTimer_;
};
```

---

### 7. Chat System

**Location**: `plugins/bosh/chat/`

Unified chat from multiple platforms.

#### Architecture

```
Platform Adapters
├── Twitch Adapter
├── YouTube Adapter
├── Facebook Adapter
├── Kick Adapter
└── ...
        ↓
Chat Normalizer
        ↓
Unified Chat Model
        ↓
Chat Dock UI
        ↓
Moderation Engine
        ↓
AI Analysis
```

#### Unified Chat Message

```cpp
struct ChatMessage {
    QString id;
    QString platform;
    QString username;
    QString displayName;
    QString message;
    QList<Badge> badges;
    QColor color;
    qint64 timestamp;
    bool isSubscriber;
    bool isModerator;
    bool isBroadcaster;
    MessageFlags flags;
};
```

#### Moderation Modes

| Mode | Behavior |
|------|----------|
| **Observe** | Log only, no action |
| **Suggest** | Suggest actions to moderator |
| **Auto** | Auto-timeout obvious violations |
| **Strict Auto** | Aggressive auto-moderation |

---

### 8. Clip Factory

**Location**: `plugins/bosh/clips/`

Automatic clip detection and creation.

#### Detection Signals

| Signal | Source |
|--------|--------|
| Chat spike | Chat rate increase |
| Audio peak | Audio metering |
| Viewer reaction | Platform reactions |
| Scene change | OBS events |
| Keyword | AI keyword detection |
| Laughter | Audio pattern |
| Applause | Audio pattern |
| Manual marker | User button press |

#### Pipeline

```
Video Stream (background copy)
     ↓
Moment Detection (multiple signals)
     ↓
Ranking Algorithm (score moments)
     ↓
Clip Creation (FFmpeg)
     ↓
Subtitle Generation
     ↓
Vertical Reframe (for TikTok/Shorts)
     ↓
QA Check
     ↓
Export
```

**Critical**: Never block OBS video pipeline. Use background workers.

---

### 9. Caption Engine

**Location**: `plugins/bosh/captions/`

Real-time speech-to-text captions.

#### Pipeline

```
Audio Input (program/mic)
     ↓
Speech Recognition (local or cloud)
     ↓
Transcript with timestamps
     ↓
Caption Formatter
     ↓
Karma Overlay (caption source)
     ↓
OBS Scene
```

#### Supported Languages

Configurable based on speech recognition backend.

---

### 10. Translation Engine

**Location**: `plugins/bosh/translation/`

Multi-language caption translation.

#### Pipeline

```
Transcript (source language)
     ↓
Translation Engine
     ↓
Translated Transcript
     ↓
Caption Renderer
     ↓
Karma Overlay
```

Support multiple output languages simultaneously.

---

### 11. Automation Engine

**Location**: `plugins/bosh/automation/`

Rule-based scene and overlay automation.

#### Rule Structure

```json
{
  "id": "guest-arrival-automation",
  "name": "Guest Arrival",
  "enabled": true,
  "trigger": {
    "type": "guest.joined",
    "conditions": []
  },
  "actions": [
    {
      "type": "scene.switch",
      "params": {"sceneName": "Guest Interview"}
    },
    {
      "type": "overlay.show",
      "params": {"overlayId": "guest-lower-third"}
    }
  ]
}
```

#### Built-in Rules

| Trigger | Action |
|---------|--------|
| Guest joins | Switch to guest scene |
| Guest leaves | Return to main scene |
| Donation received | Trigger alert |
| Stream begins | Starting sequence |
| Stream ends | Ending sequence |
| Audio failure | Show warning |
| Chat spike | Notify streamer |

---

### 12. Analytics Engine

**Location**: `plugins/bosh/analytics/`

Metrics collection and insights.

#### Tracked Metrics

| Category | Metrics |
|----------|---------|
| **Audience** | Viewers, peak viewers, followers, subscribers |
| **Engagement** | Chat rate, reactions, poll participation |
| **Revenue** | Donations, memberships, gifts |
| **Stream Health** | FPS, dropped frames, bitrate, CPU, GPU |
| **Content** | Scenes used, alerts triggered, clips created |
| **Platform** | Per-platform metrics |

**Implementation**: Background processing, never block real-time operations.

---

### 13. Platform Adapters

**Location**: `plugins/bosh/platforms/`

Abstract platform-specific APIs.

#### Interface

```cpp
class PlatformAdapter {
public:
    virtual ~PlatformAdapter() = default;
    
    virtual QString platformName() const = 0;
    virtual PlatformType platformType() const = 0;
    
    virtual bool connect(const AuthCredentials& creds) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    virtual QList<ChatMessage> fetchChat() = 0;
    virtual StreamHealth getStreamHealth() = 0;
    virtual StreamStats getStreamStats() = 0;
    
    virtual void sendChatMessage(const QString& message) = 0;
    virtual void timeoutUser(const QString& userId, int duration) = 0;
    virtual void banUser(const QString& userId) = 0;
    
signals:
    void chatMessageReceived(const ChatMessage& msg);
    void followerEvent(const FollowerEvent& event);
    void subscriberEvent(const SubscriberEvent& event);
    void donationEvent(const DonationEvent& event);
    void connectionStateChanged(bool connected);
    void error(const QString& message);
};
```

#### Supported Platforms

- Twitch
- YouTube
- Facebook Gaming
- Kick
- (Extensible for additional platforms)

---

### 14. Diagnostics Engine

**Location**: `plugins/bosh/diagnostics/`

System health monitoring and troubleshooting.

#### Checks

| Component | Checks |
|-----------|--------|
| **OBS** | Version, plugins loaded, errors |
| **Bosh** | Module status, event bus, docks |
| **Karma** | Browser source, templates loaded |
| **AI** | Ollama connection, model availability |
| **Platform** | Connection status, API limits |
| **Network** | Latency, bandwidth |
| **Audio** | Device status, levels |
| **GPU** | Utilization, temperature |
| **CPU** | Utilization, cores |
| **Memory** | Usage, available |
| **Storage** | Free space, write speed |

#### Diagnostic Report

Exportable JSON/HTML report with redacted secrets.

---

## Data Models

### Core Entities

```cpp
// Alert
struct Alert {
    QString id;
    AlertType type;
    Priority priority;
    QVariant payload;
    QString templateId;
    QString themeId;
    qint64 createdAt;
    bool played;
};

// Chat Message
struct ChatMessage {
    QString id;
    QString platform;
    QString username;
    QString message;
    QList<Badge> badges;
    qint64 timestamp;
    MessageFlags flags;
};

// Stream Health
struct StreamHealth {
    double fps;
    int droppedFrames;
    int totalFrames;
    qint64 bitrate;
    double cpuUsage;
    double gpuUsage;
    qint64 memoryUsage;
    NetworkStatus network;
};

// Clip Candidate
struct ClipCandidate {
    QString id;
    qint64 startTime;
    qint64 endTime;
    int score;
    QList<QString> triggers;
    QString sceneName;
    bool processed;
};
```

---

## Configuration Schema

```json
{
  "bosh": {
    "version": 1,
    "general": {
      "enabled": true,
      "auto_start": false,
      "language": "en-US"
    },
    "ai": {
      "enabled": true,
      "provider": "ollama",
      "endpoint": "http://localhost:11434",
      "model": "llama3.1:8b",
      "context_limit": 4096,
      "timeout_ms": 5000
    },
    "chat": {
      "enabled": true,
      "platforms": [],
      "moderation_mode": "observe",
      "show_badges": true,
      "show_colors": true
    },
    "overlays": {
      "karma_enabled": true,
      "default_theme": "karma-neon",
      "auto_add_to_scenes": true
    },
    "alerts": {
      "enabled": true,
      "queue_size": 20,
      "cooldown_ms": 5000,
      "sound_enabled": true,
      "tts_enabled": false
    },
    "clips": {
      "enabled": false,
      "auto_create": false,
      "output_directory": "",
      "vertical_format": false
    },
    "captions": {
      "enabled": false,
      "language": "en-US",
      "style": "default"
    },
    "translation": {
      "enabled": false,
      "target_languages": []
    },
    "automation": {
      "enabled": true,
      "rules": []
    },
    "analytics": {
      "enabled": true,
      "retention_days": 30
    },
    "platforms": {
      "twitch": {
        "connected": false,
        "credentials_encrypted": true
      },
      "youtube": {
        "connected": false,
        "credentials_encrypted": true
      }
    },
    "permissions": {
      "default_level": "READ_ONLY",
      "users": {}
    }
  }
}
```

---

## Permission System

### Permission Levels

```cpp
enum class BoshPermission {
    READ_ONLY,          // View state, no modifications
    OBS_CONTROL,        // Change scenes, sources, audio
    CHAT_CONTROL,       // Moderation actions
    OVERLAY_CONTROL,    // Modify overlays, themes
    AI_CONTROL,         // Execute AI commands
    AUTOMATION,         // Create/edit automation rules
    PLATFORM_CONTROL,   // Connect/manage platforms
    PUBLISH,            // Start/stop streaming
    ADMIN               // Full access including settings
};
```

### Permission Checks

```cpp
bool PermissionManager::checkPermission(
    BoshPermission required,
    const QString& userId
) {
    auto userLevel = getUserPermission(userId);
    return userLevel >= required;
}

// Example usage
if (!permissionManager->checkPermission(
        BoshPermission::OBS_CONTROL, 
        currentUser_)) {
    showError("Insufficient permissions");
    return;
}
```

---

## Error Handling Strategy

### Graceful Degradation

```
If AI fails → Disable AI features, continue broadcasting
If Ollama fails → Show error, allow retry, continue without AI
If overlay fails → Log error, skip overlay, continue stream
If platform API fails → Retry with backoff, notify user
If Bosh crashes → OBS continues, show recovery dialog
```

### Error Categories

| Category | Recovery |
|----------|----------|
| **Configuration** | Show settings dialog |
| **Network** | Retry with exponential backoff |
| **Permission** | Request authorization |
| **Resource** | Free resources, reduce quality |
| **Internal** | Log, notify, attempt recovery |

---

## Performance Budgets

| Operation | Max Duration | Thread |
|-----------|-------------|--------|
| UI update | 16ms (60fps) | Main |
| OBS API call | 100ms | Appropriate |
| AI inference | 5000ms | Background |
| Platform API | 3000ms | Background |
| Video processing | N/A (async) | Background |
| Database write | 100ms | Background |

### Resource Limits

```cpp
constexpr int MAX_ALERT_QUEUE_SIZE = 20;
constexpr int MAX_CHAT_HISTORY = 1000;
constexpr int MAX_ANALYTICS_RETENTION_DAYS = 90;
constexpr size_t MAX_CLIP_CACHE_GB = 10;
constexpr int MAX_AI_CONCURRENT_REQUESTS = 3;
```

---

## Testing Strategy

### Test Levels

1. **Unit Tests**: Individual components
2. **Integration Tests**: Component interactions
3. **System Tests**: Full Bosh + OBS
4. **Performance Tests**: Load, stress, endurance
5. **Compatibility Tests**: Different OBS versions

### Golden Test

```
Fake Subscriber Event
     ↓
Event Bus Routing
     ↓
Alert Agent Processing
     ↓
Karma Template Rendering
     ↓
Browser Source Update
     ↓
OBS Preview Display
     ↓
Visual Verification (automated or manual)
```

---

## Documentation Structure

```
docs/bosh/
├── README.md              # Quick start
├── ARCHITECTURE.md        # This document
├── OBS_INTEGRATION.md     # Integration details
├── KARMA.md               # Karma overlay guide
├── AI.md                  # AI engine documentation
├── AGENTS.md              # Multi-agent system
├── CHAT.md                # Chat system guide
├── ALERTS.md              # Alert configuration
├── CLIPS.md               # Clip factory guide
├── CAPTIONS.md            # Caption setup
├── ANALYTICS.md           # Analytics reference
├── AUTOMATION.md          # Automation rules
├── SECURITY.md            # Security practices
├── TESTING.md             # Testing procedures
├── BUILDING.md            # Build instructions
└── UPSTREAM_SYNC.md       # OBS update strategy
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2024 | Initial architecture |

---

*This document evolves as the implementation progresses.*
