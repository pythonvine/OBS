# Karma Overlay Engine Architecture

## Overview

Karma is the overlay rendering engine for Bosh Broadcast Pro, integrated directly into the OBS Studio fork. It uses the existing obs-browser plugin infrastructure to render dynamic, animated overlays without creating a duplicate browser engine.

---

## Design Philosophy

### 1. Reuse Over Reinvention

**DO**: Use obs-browser (CEF) that's already part of OBS  
**DO NOT**: Create a new browser engine

### 2. Integration Over Separation

**DO**: Integrate Karma as a Bosh subsystem  
**DO NOT**: Make Karma a separate application

### 3. Templates Over Hardcoding

**DO**: Use template system for flexibility  
**DO NOT**: Hardcode alert designs

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Karma Manager                                │
│  - Lifecycle management                                         │
│  - Source creation/destruction                                  │
│  - State synchronization                                        │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                  Template Engine                                │
│  - Variable substitution                                        │
│  - Conditional rendering                                        │
│  - Animation triggers                                           │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                   Theme Engine                                  │
│  - Built-in themes                                              │
│  - Theme inheritance                                            │
│  - Custom themes                                                │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                 Animation Engine                                │
│  - CSS animations                                               │
│  - JavaScript timelines                                         │
│  - Sound effects                                                │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│              Browser Source Runtime                             │
│  - obs-browser wrapper                                          │
│  - postMessage communication                                    │
│  - State updates                                                │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                  OBS Scene                                      │
│  - Browser source as scene item                                 │
│  - Transparency support                                         │
│  - Layer ordering                                               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. KarmaManager

**Location**: `plugins/bosh/overlays/karma-manager.cpp`

```cpp
class KarmaManager : public QObject {
    Q_OBJECT
    
public:
    static KarmaManager* instance();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Overlay lifecycle
    QString createOverlay(const OverlayConfig& config);
    void destroyOverlay(const QString& overlayId);
    Overlay* getOverlay(const QString& overlayId);
    
    // Triggering
    void triggerAlert(const Alert& alert);
    void triggerEvent(const QString& eventType, const QVariantMap& data);
    
    // Scene management
    void addToScene(const QString& overlayId, obs_scene_t* scene);
    void removeFromScene(const QString& overlayId, obs_scene_t* scene);
    
    // Preview
    void previewOverlay(const QString& overlayId);
    void stopPreview();
    
signals:
    void overlayCreated(const QString& overlayId);
    void overlayDestroyed(const QString& overlayId);
    void overlayError(const QString& overlayId, const QString& error);
    
private:
    QString generateOverlayUrl(const QString& templateId, const QString& themeId);
    void sendMessageToBrowser(const QString& overlayId, const QJsonObject& message);
    
    QHash<QString, Overlay*> overlays_;
    QString previewOverlayId_;
};
```

### 2. TemplateEngine

**Location**: `plugins/bosh/overlays/karma-template.cpp`

```cpp
class TemplateEngine : public QObject {
    Q_OBJECT
    
public:
    bool loadTemplate(const QString& templatePath);
    Template getTemplate(const QString& templateId);
    QList<Template> listTemplates();
    
    // Render template with variables
    QString render(const QString& templateId, const QVariantMap& variables);
    
    // Validation
    bool validateTemplate(const Template& template);
    
private:
    QString substituteVariables(const QString& content, const QVariantMap& variables);
    void loadTemplateAssets(Template& tmpl);
    
    QHash<QString, Template> templates_;
    QString templateDirectory_;
};
```

#### Template Schema

```json
{
  "id": "subscriber-alert-v1",
  "name": "Subscriber Alert",
  "category": "engagement",
  "version": "1.0",
  "resolution": {
    "width": 1920,
    "height": 1080
  },
  "variables": [
    {
      "name": "username",
      "type": "string",
      "required": true,
      "default": ""
    },
    {
      "name": "displayName",
      "type": "string",
      "required": true,
      "default": ""
    },
    {
      "name": "tier",
      "type": "string",
      "required": false,
      "default": "1000"
    },
    {
      "name": "message",
      "type": "string",
      "required": false,
      "default": ""
    },
    {
      "name": "platform",
      "type": "string",
      "required": true,
      "default": "twitch"
    }
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
    "hold": 2000,
    "cooldown": 3000
  },
  "priority": 5,
  "layers": [
    {
      "type": "background",
      "zIndex": 0
    },
    {
      "type": "icon",
      "zIndex": 1
    },
    {
      "type": "text-username",
      "zIndex": 2
    },
    {
      "type": "text-message",
      "zIndex": 2
    }
  ]
}
```

### 3. ThemeEngine

**Location**: `plugins/bosh/overlays/karma-theme.cpp`

```cpp
class ThemeEngine : public QObject {
    Q_OBJECT
    
public:
    bool initialize();
    
    Theme getTheme(const QString& themeId);
    QList<Theme> listThemes();
    
    // Apply theme to template
    QString applyTheme(const QString& templateHtml, const Theme& theme);
    
    // Custom themes
    bool saveCustomTheme(const Theme& theme);
    bool deleteCustomTheme(const QString& themeId);
    
private:
    void loadBuiltInThemes();
    void loadCustomThemes();
    
    QHash<QString, Theme> builtInThemes_;
    QHash<QString, Theme> customThemes_;
};
```

#### Built-in Themes

| Theme | Description |
|-------|-------------|
| **Karma Neon** | Vibrant neon colors, cyberpunk aesthetic |
| **Karma Cyber** | Futuristic tech look, grid patterns |
| **Karma Gaming** | Bold gaming aesthetic, energetic |
| **Karma Creator** | Clean, professional creator look |
| **Karma Podcast** | Minimal, conversation-focused |
| **Karma News** | Professional broadcast style |
| **Karma Minimal** | Simple, unobtrusive design |
| **Karma Futuristic** | Advanced sci-fi aesthetic |

#### Theme Structure

```json
{
  "id": "karma-neon",
  "name": "Karma Neon",
  "version": "1.0",
  "colors": {
    "primary": "#FF00FF",
    "secondary": "#00FFFF",
    "accent": "#FFFF00",
    "background": "#0A0A0A",
    "text": "#FFFFFF",
    "textSecondary": "#CCCCCC"
  },
  "fonts": {
    "heading": "Orbitron",
    "body": "Roboto"
  },
  "effects": {
    "glow": true,
    "gradient": true,
    "particles": false
  },
  "borderRadius": 8,
  "shadowIntensity": 0.5
}
```

### 4. AnimationEngine

**Location**: `plugins/bosh/overlays/karma-animation.cpp`

Handles animation orchestration via CSS and JavaScript.

```cpp
class AnimationEngine : public QObject {
    Q_OBJECT
    
public:
    // Pre-built animations
    static QString slideInLeft();
    static QString slideInRight();
    static QString fadeIn();
    static QString zoomIn();
    static QString bounce();
    
    // Custom animation
    QString createAnimation(const AnimationConfig& config);
    
    // Sound effects
    void playSound(const QString& soundPath);
    void setVolume(float volume);
    
private:
    QHash<QString, QString> animationCache_;
    float soundVolume_ = 0.7f;
};
```

---

## Overlay Types

### Supported Event Overlays

| Type | Variables |
|------|-----------|
| **Subscriber** | username, displayName, tier, message, months |
| **Follower** | username, displayName, platform |
| **Donation** | username, amount, currency, message, platform |
| **Membership** | username, tier, duration, benefits |
| **Raid** | username, viewerCount, platform |
| **Like** | username, platform |
| **Reaction** | username, reactionType, platform |
| **Gift** | gifterUsername, receiverUsername, count, tier |
| **Chat Highlight** | username, message, highlightReason |
| **Goal** | goalName, currentValue, targetValue, type |
| **Poll** | pollQuestion, options, results |
| **Giveaway** | giveawayName, winner, prize |
| **Countdown** | eventName, startTime, currentTime |
| **Lower Third** | personName, title, subtitle |
| **Sponsor** | sponsorName, logo, message, tier |
| **Breaking Alert** | headline, description, priority |
| **System Alert** | message, type, icon |
| **Caption** | text, language, speaker |
| **Translation** | originalText, translatedText, language |
| **AI Message** | message, aiModel, confidence |

---

## Browser Source Integration

### Creating Browser Source

```cpp
obs_source_t* KarmaManager::createBrowserSource(
    const QString& overlayId,
    const QString& url,
    int width,
    int height
) {
    obs_data_t* settings = obs_data_create();
    obs_data_set_string(settings, "url", url.toUtf8().constData());
    obs_data_set_int(settings, "width", width);
    obs_data_set_int(settings, "height", height);
    obs_data_set_bool(settings, "shutdown_source", true);
    obs_data_set_bool(settings, "reroute_audio", false);
    
    // Create unique name
    QString sourceName = QString("Karma_%1").arg(overlayId);
    
    obs_source_t* source = obs_source_create(
        "browser_source",
        sourceName.toUtf8().constData(),
        settings,
        nullptr
    );
    
    obs_data_release(settings);
    return source;
}
```

### Communication Protocol

JavaScript interface in browser source:

```javascript
// Receive messages from C++
window.addEventListener('message', (event) => {
    const { type, payload } = event.data;
    
    switch(type) {
        case 'TRIGGER_ALERT':
            showAlert(payload);
            break;
        case 'UPDATE_VARIABLES':
            updateVariables(payload);
            break;
        case 'PLAY_SOUND':
            playSound(payload.url);
            break;
        case 'HIDE':
            hideOverlay();
            break;
    }
});

// Send events back to C++
function notifyComplete(alertId) {
    window.obsstudio?.postMessage({
        type: 'ALERT_COMPLETE',
        alertId: alertId
    });
}
```

C++ message sending:

```cpp
void KarmaManager::sendMessageToBrowser(
    const QString& overlayId,
    const QJsonObject& message
) {
    auto overlay = overlays_.value(overlayId);
    if (!overlay || !overlay->browserSource) {
        return;
    }
    
    // Use obs_browser_post_message if available
    // Or use signal/slot mechanism
    QByteArray jsonData = QJsonDocument(message).toJson();
    obs_source_send_notification(
        overlay->browserSource,
        "karma_message",
        jsonData.constData(),
        jsonData.size()
    );
}
```

---

## Automatic Scene Integration

### First-Run Setup

```cpp
void KarmaManager::autoConfigureScenes()
{
    // Get current scene collection
    obs_sceneenum_t* enumerator = createSceneEnumerator();
    obs_enum_scenes(enumerator);
    
    for (auto scene : enumerator->scenes()) {
        // Check if Karma overlay already exists
        if (!hasKarmaOverlay(scene)) {
            // Create default overlay
            QString overlayId = createDefaultOverlay();
            addToScene(overlayId, scene);
        }
    }
}
```

### Rollback Support

```cpp
bool KarmaManager::rollbackSceneChanges()
{
    // Find all Karma sources
    auto karmaSources = findKarmaSources();
    
    for (auto source : karmaSources) {
        // Remove from all scenes
        removeFromAllScenes(source);
        
        // Release source
        obs_source_remove(source);
    }
    
    return true;
}
```

---

## File Structure

```
plugins/bosh/overlays/
├── karma-manager.cpp
├── karma-manager.h
├── karma-template.cpp
├── karma-template.h
├── karma-theme.cpp
├── karma-theme.h
├── karma-animation.cpp
├── karma-animation.h
└── data/
    └── templates/
        ├── subscriber/
        │   ├── alert.html
        │   ├── alert.css
        │   └── alert.js
        ├── follower/
        │   ├── alert.html
        │   ├── alert.css
        │   └── alert.js
        ├── donation/
        │   ├── alert.html
        │   ├── alert.css
        │   └── alert.js
        └── ...
    └── themes/
        ├── karma-neon/
        │   └── theme.json
        ├── karma-cyber/
        │   └── theme.json
        └── ...
```

---

## Example Template HTML

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <link rel="stylesheet" href="alert.css">
</head>
<body class="theme-karma-neon">
    <div id="alert-container" class="hidden">
        <div class="alert-card">
            <img class="icon" src="{{iconUrl}}" alt="">
            <div class="content">
                <h1 class="username">{{displayName}}</h1>
                <p class="message">{{message}}</p>
                <div class="platform-badge">
                    <img src="{{platformIcon}}" alt="">
                </div>
            </div>
        </div>
    </div>
    
    <script src="alert.js"></script>
</body>
</html>
```

---

## Performance Considerations

### 1. Browser Source Limits

- Limit concurrent browser sources (recommend max 3-5)
- Use single browser source with dynamic content when possible
- Set appropriate resolution (don't use 4K for simple alerts)

### 2. Memory Management

```cpp
// Shutdown unused overlays
void KarmaManager::cleanupInactiveOverlays()
{
    for (auto it = overlays_.begin(); it != overlays_.end();) {
        if ((*it)->lastUsed < QDateTime::currentDateTime().addSecs(-300)) {
            destroyOverlay((*it)->id);
            it = overlays_.erase(it);
        } else {
            ++it;
        }
    }
}
```

### 3. Asset Caching

- Cache compiled templates
- Cache theme stylesheets
- Preload common sounds
- Use sprite sheets for icons

---

## Testing

### Manual Test Mode

Buttons in Karma Dock:
- Test Subscriber Alert
- Test Follower Alert
- Test Donation Alert
- Test Raid Alert
- Test Goal Update
- Test Lower Third
- Test Caption Display

### Automated Test

```cpp
void KarmaTest::testSubscriberAlertRendering()
{
    // Create test alert
    Alert alert;
    alert.type = AlertType::Subscriber;
    alert.payload["username"] = "test_viewer";
    alert.payload["displayName"] = "Test Viewer";
    alert.payload["tier"] = "1000";
    
    // Trigger
    karmaManager->triggerAlert(alert);
    
    // Wait for animation
    QTest::qWait(1000);
    
    // Capture frame
    QImage frame = captureOBSFrame();
    
    // Verify overlay visible
    QVERIFY(frame.containsPoint(QPoint(100, 100)));
    
    // Wait for completion
    QTest::qWait(5000);
    
    // Verify overlay hidden
    QImage finalFrame = captureOBSFrame();
    QVERIFY(!finalFrame.containsPoint(QPoint(100, 100)));
}
```

---

## Migration Path

For users upgrading from manual browser source setup:

1. Detect existing browser sources named "Karma*" or similar
2. Offer to migrate to managed Karma overlays
3. Preserve URL and dimensions
4. Import custom templates if found
5. Provide rollback option

---

## Future Enhancements

1. **WebGL Effects**: GPU-accelerated particle effects
2. **Video Overlays**: Short video clips in alerts
3. **Interactive Elements**: Clickable overlays
4. **Live Data**: Real-time stats in overlays
5. **AR Integration**: Augmented reality overlays
6. **Multi-Language Templates**: Auto-translate overlay text

---

## Dependencies

- obs-browser plugin (CEF)
- Qt Network (for asset loading)
- Qt Multimedia (optional, for sound)

---

*This document evolves as Karma implementation progresses.*
