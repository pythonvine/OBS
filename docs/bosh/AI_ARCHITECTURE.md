# Bosh AI Engine Architecture

## Overview

The Bosh AI Engine provides intelligent broadcast assistance through local and cloud-based AI models. It is designed to run asynchronously without blocking OBS's critical rendering paths.

---

## Design Principles

### 1. Non-Blocking Operation

All AI inference runs on background threads. The UI and OBS rendering are never blocked waiting for AI responses.

### 2. Model Agnostic

Support multiple AI providers:
- Ollama (local models)
- OpenAI API (cloud)
- Anthropic API (cloud)
- Future providers

### 3. Graceful Degradation

If AI fails, Bosh continues operating with reduced functionality.

### 4. Privacy First

Local processing preferred. Cloud calls only when explicitly configured.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    AI Engine Core                               │
│  - Request routing                                              │
│  - Context management                                           │
│  - Response caching                                             │
│  - Rate limiting                                                │
└─────────────────────────────────────────────────────────────────┘
                              ↕
┌──────────────────┬──────────────────┬──────────────────────────┐
│   OllamaAdapter  │  OpenAIAdapter   │  FutureAdapters          │
│   (Local)        │  (Cloud)         │                          │
└──────────────────┴──────────────────┴──────────────────────────┘
                              ↕
┌─────────────────────────────────────────────────────────────────┐
│                  Capability Layer                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Command  │  │ Chat     │  │ Clip     │  │ Moderation     │  │
│  │ Parser   │  │ Analyzer │  │ Detector │  │ Engine         │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ Audience │  │ Caption  │  │ Translate│  │ Suggestion     │  │
│  │ Insights │  │ Enhancer │  │ Engine   │  │ Engine         │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. AIEngine

**Location**: `plugins/bosh/ai/ai-engine.cpp`

```cpp
class AIEngine : public QObject {
    Q_OBJECT
    
public:
    static AIEngine* instance();
    
    bool initialize(const AIConfig& config);
    void shutdown();
    
    // Provider management
    void setProvider(ProviderType type);
    ProviderType currentProvider() const;
    bool isAvailable() const;
    
    // Inference
    void generate(const QString& prompt, const GenerateOptions& options);
    void generateStream(const QString& prompt, const GenerateOptions& options);
    
    // Context
    void setContext(const QString& contextId, const QVariantMap& data);
    void clearContext(const QString& contextId);
    
signals:
    void responseReady(const QString& response);
    void responseChunk(const QString& chunk);
    void error(const QString& message);
    void availabilityChanged(bool available);
    
private:
    std::unique_ptr<ModelAdapter> adapter_;
    QHash<QString, QVariantMap> contexts_;
    QQueue<PendingRequest> pendingRequests_;
    QTimer* rateLimitTimer_;
};
```

### 2. OllamaAdapter

**Location**: `plugins/bosh/ai/ollama-adapter.cpp`

```cpp
class OllamaAdapter : public ModelAdapter {
    Q_OBJECT
    
public:
    explicit OllamaAdapter(const QString& endpoint = "http://localhost:11434");
    
    // Health & Discovery
    bool healthCheck() override;
    QStringList availableModels() override;
    ModelInfo currentModel() const override;
    
    // Inference
    void generate(const QString& prompt, const GenerateOptions& options) override;
    void generateStream(const QString& prompt, const GenerateOptions& options) override;
    
    // Configuration
    void setModel(const QString& modelName);
    void setEndpoint(const QString& endpoint);
    void setContextLimit(int tokens);
    
signals:
    void modelsDiscovered(const QStringList& models);
    void healthStatusChanged(bool healthy);
    
private slots:
    void onNetworkReply(QNetworkReply* reply);
    
private:
    QJsonObject buildRequestBody(const QString& prompt, const GenerateOptions& options);
    
    QNetworkAccessManager* network_;
    QString endpoint_;
    QString currentModel_;
    int contextLimit_ = 4096;
    bool healthy_ = false;
};
```

#### Ollama API Integration

```cpp
QJsonObject OllamaAdapter::buildRequestBody(
    const QString& prompt,
    const GenerateOptions& options
) {
    QJsonObject request;
    request["model"] = currentModel_;
    request["prompt"] = prompt;
    request["stream"] = options.streaming;
    
    QJsonObject opt;
    opt["temperature"] = options.temperature;
    opt["top_p"] = options.topP;
    opt["num_predict"] = options.maxTokens;
    request["options"] = opt;
    
    // Add system context
    if (!options.systemPrompt.isEmpty()) {
        request["system"] = options.systemPrompt;
    }
    
    return request;
}
```

### 3. CommandParser

**Location**: `plugins/bosh/ai/command-parser.cpp`

Parses natural language commands into structured actions.

```cpp
class CommandParser : public QObject {
    Q_OBJECT
    
public:
    ParsedCommand parse(const QString& input, const BroadcastContext& context);
    ConfidenceLevel getConfidence() const;
    
    // Training
    void addExample(const QString& input, const ParsedCommand& command);
    
private:
    Intent classifyIntent(const QString& input);
    QVariantMap extractParameters(const QString& input, Intent intent);
    bool validateParameters(const ParsedCommand& command);
    
    QHash<Intent, QList<TrainingExample>> trainingData_;
    BroadcastContext currentContext_;
};
```

#### Supported Commands

| Command Type | Examples |
|--------------|----------|
| **Scene Control** | "Switch to gameplay", "Go to BRB scene" |
| **Audio Control** | "Mute my mic", "Turn on desktop audio" |
| **Stream Control** | "Start streaming", "Stop recording" |
| **Chat Query** | "What's chat saying?", "Any important messages?" |
| **Clip Creation** | "Create a clip", "Save that moment" |
| **Alert Control** | "Show subscriber alert", "Trigger raid animation" |
| **Information** | "How many viewers?", "Stream health?" |
| **Caption Control** | "Enable captions", "Change caption language" |

### 4. ChatAnalyzer

**Location**: `plugins/bosh/ai/chat-analyzer.cpp`

```cpp
class ChatAnalyzer : public QObject {
    Q_OBJECT
    
public:
    // Analysis
    SentimentResult analyzeSentiment(const ChatMessage& message);
    ToxicityResult analyzeToxicity(const ChatMessage& message);
    SpamResult detectSpam(const QList<ChatMessage>& recentMessages);
    
    // Insights
    TopicSummary extractTopics(const QList<ChatMessage>& messages);
    EngagementMetrics calculateEngagement(const QList<ChatMessage>& messages);
    ActiveUsers findActiveUsers(const QList<ChatMessage>& messages, int limit);
    
    // Classification
    MessageCategory categorize(const ChatMessage& message);
    
signals:
    void toxicityDetected(const ChatMessage& message, float score);
    void spamDetected(const QList<ChatMessage>& messages);
    void sentimentShift(SentimentType from, SentimentType to);
    
private:
    QCache<QString, SentimentResult> sentimentCache_;
    QCache<QString, ToxicityResult> toxicityCache_;
};
```

### 5. ClipDetector

**Location**: `plugins/bosh/ai/clip-detector.cpp`

Identifies highlight moments using multi-signal analysis.

```cpp
class ClipDetector : public QObject {
    Q_OBJECT
    
public:
    void processFrame(const VideoFrame& frame, const BroadcastState& state);
    void processChatSpike(const ChatSpikeEvent& event);
    void processAudioPeak(const AudioPeakEvent& event);
    
    // Manual markers
    void addMarker(const QString& reason);
    
    // Clip ranking
    QList<ClipCandidate> getRankedCandidates();
    
signals:
    void clipMomentDetected(const ClipCandidate& candidate);
    void highlightScoreUpdated(const QString& momentId, float score);
    
private:
    float calculateMomentScore(const MomentSignals& signals);
    void pruneOldCandidates();
    
    QHash<QString, ClipCandidate> activeMoments_;
    QList<ClipCandidate> completedCandidates_;
    BroadcastState currentState_;
};
```

#### Moment Scoring

```cpp
float ClipDetector::calculateMomentScore(const MomentSignals& signals) {
    float score = 0.0f;
    
    // Chat activity (0-30 points)
    score += qMin(30.0f, signals.chatSpikeIntensity * 30.0f);
    
    // Audio excitement (0-25 points)
    score += qMin(25.0f, signals.audioExcitement * 25.0f);
    
    // Viewer reactions (0-25 points)
    score += qMin(25.0f, signals.viewerReactions * 25.0f);
    
    // Scene importance (0-10 points)
    score += signals.sceneImportance * 10.0f;
    
    // Keyword detection (0-10 points)
    score += signals.keywordDetected ? 10.0f : 0.0f;
    
    return score;
}
```

---

## AI Capabilities

### 1. Broadcast Commands

Natural language control of OBS and Bosh features.

**Example Flow:**
```
User: "Switch to the gameplay scene please"
     ↓
CommandParser → Intent: SCENE_SWITCH, Parameter: "gameplay"
     ↓
Permission check (OBS_CONTROL)
     ↓
Find matching scene
     ↓
Execute via obs_frontend_set_current_scene()
     ↓
Confirm: "Switched to Gameplay scene"
```

### 2. Chat Analysis

Real-time chat sentiment and toxicity detection.

**Use Cases:**
- Highlight positive moments
- Detect harassment early
- Identify frequently asked questions
- Surface important messages to streamer

### 3. Clip Detection

Automatic highlight identification.

**Signals Combined:**
- Chat rate spikes
- Audio level changes (laughter, shouting)
- Viewer reaction bursts
- Scene transitions
- Keyword mentions
- Manual markers

### 4. Audience Insights

Understand viewer engagement patterns.

**Metrics:**
- Peak engagement times
- Most active chatters
- Popular topics
- Sentiment trends
- Retention indicators

### 5. Moderation Support

AI-assisted chat moderation.

**Modes:**
- **Observe**: Log only
- **Suggest**: Recommend timeouts/bans
- **Auto**: Auto-timeout obvious violations
- **Strict**: Aggressive auto-moderation

**Detection:**
- Toxicity scoring
- Spam patterns
- Suspicious links
- Raid detection
- Bot behavior

### 6. Caption Enhancement

Improve auto-generated captions.

**Enhancements:**
- Punctuation insertion
- Speaker identification
- Technical term correction
- Profanity filtering (optional)

### 7. Translation

Multi-language support.

**Pipeline:**
```
English transcript → Translation API → Spanish caption
                   → Translation API → French caption
                   → Translation API → Japanese caption
```

---

## Context Management

### Broadcast Context

```cpp
struct BroadcastContext {
    QString streamTitle;
    QString gameCategory;
    qint64 streamDuration;
    int currentViewers;
    int peakViewers;
    QString currentScene;
    bool isLive;
    bool isRecording;
    QDateTime streamStartTime;
    QList<QString> recentChatMessages;
    AudioLevels audioLevels;
    StreamHealth health;
};
```

### Conversation Context

Maintain conversation history for coherent multi-turn dialogue.

```cpp
class ConversationManager {
public:
    void addMessage(const QString& role, const QString& content);
    void clearHistory();
    QList<Message> getHistory(int maxMessages = 10);
    
    // Context window management
    void setMaxTokens(int tokens);
    void compressIfNeeded();
    
private:
    QList<Message> history_;
    int maxTokens_ = 4096;
};
```

---

## Safety & Ethics

### Content Filtering

```cpp
class SafetyFilter {
public:
    SafetyResult filter(const QString& input);
    SafetyResult filterResponse(const QString& response);
    
    // Configurable filters
    void setProfanityFilter(bool enabled);
    void setPersonalInfoFilter(bool enabled);
    void setTopicRestrictions(const QStringList& restricted);
    
private:
    bool containsPersonalInfo(const QString& text);
    bool violatesPolicy(const QString& text);
};
```

### Permission Requirements

| Action | Required Permission |
|--------|---------------------|
| Read chat | READ_ONLY |
| Send chat message | CHAT_CONTROL |
| Switch scene | OBS_CONTROL |
| Start/stop stream | PUBLISH |
| Timeout/ban user | CHAT_CONTROL + ADMIN |
| Access analytics | READ_ONLY |
| Modify settings | ADMIN |

---

## Performance Optimization

### 1. Caching

```cpp
// Response cache
QCache<QString, QString> responseCache_;

// Embedding cache for similarity comparisons
QCache<QString, QVector<float>> embeddingCache_;
```

### 2. Rate Limiting

```cpp
class RateLimiter {
public:
    bool allowRequest();
    int getRemainingRequests();
    qint64 getResetTime();
    
private:
    int maxRequestsPerMinute_ = 60;
    QQueue<QDateTime> requestTimestamps_;
};
```

### 3. Batch Processing

For non-real-time tasks like analytics:

```cpp
void AnalyticsAgent::processBatch(const QList<ChatMessage>& messages)
{
    // Process 100 messages at once instead of individually
    auto result = aiEngine_->generate(
        buildBatchPrompt(messages),
        options
    );
}
```

### 4. Model Selection

Choose appropriate model for task:

| Task | Recommended Model |
|------|-------------------|
| Simple classification | TinyLlama (1.1B) |
| Chat analysis | Llama 3.1 8B |
| Complex reasoning | Llama 3.1 70B |
| Translation | Dedicated translation model |
| Transcription | Whisper |

---

## Error Handling

### Failure Modes

| Failure | Recovery |
|---------|----------|
| Ollama not running | Show setup dialog, disable AI features |
| Model not found | List available models, suggest download |
| Context limit exceeded | Truncate oldest messages |
| Rate limit hit | Queue requests, retry after cooldown |
| Timeout | Retry with backoff, show error if persistent |
| Invalid response | Log, notify, use fallback response |

### Health Monitoring

```cpp
class AIHealthMonitor : public QObject {
    Q_OBJECT
    
public:
    void startMonitoring();
    
    HealthStatus getStatus() const;
    QString getLastErrorMessage() const;
    int getUptimeSeconds() const;
    
signals:
    void healthStatusChanged(HealthStatus status);
    void recoverySucceeded();
    
private:
    QTimer* healthCheckTimer_;
    int consecutiveFailures_ = 0;
    HealthStatus currentStatus_ = HealthStatus::Unknown;
};
```

---

## Configuration

### AI Settings Schema

```json
{
  "ai": {
    "enabled": true,
    "provider": "ollama",
    "endpoint": "http://localhost:11434",
    "model": "llama3.1:8b",
    "fallback_model": "tinyllama:1.1b",
    "context_limit": 4096,
    "max_tokens": 512,
    "temperature": 0.7,
    "timeout_ms": 5000,
    "retry_count": 3,
    "streaming": true,
    "cache_enabled": true,
    "rate_limit": {
      "requests_per_minute": 60,
      "tokens_per_minute": 10000
    },
    "features": {
      "commands": true,
      "chat_analysis": true,
      "clip_detection": true,
      "moderation": false,
      "captions": false,
      "translation": false
    },
    "safety": {
      "profanity_filter": true,
      "personal_info_filter": true,
      "restricted_topics": []
    }
  }
}
```

---

## Testing

### Unit Tests

```cpp
void AITest::testCommandParsing()
{
    CommandParser parser;
    
    ParsedCommand cmd = parser.parse("switch to gameplay scene");
    
    QCOMPARE(cmd.intent, Intent::SCENE_SWITCH);
    QCOMPARE(cmd.parameters["scene"].toString(), "gameplay");
    QVERIFY(cmd.confidence > 0.8);
}

void AITest::testSentimentAnalysis()
{
    ChatAnalyzer analyzer;
    
    ChatMessage positiveMsg;
    positiveMsg.message = "This is amazing! Love the stream!";
    
    SentimentResult result = analyzer.analyzeSentiment(positiveMsg);
    
    QCOMPARE(result.sentiment, SentimentType::Positive);
    QVERIFY(result.confidence > 0.7);
}
```

### Integration Tests

```cpp
void AIIntegrationTest::testEndToEndCommand()
{
    // Setup
    AIEngine::instance()->initialize(config);
    
    // Execute command
    AIEngine::instance()->generate("mute my microphone", options);
    
    // Wait for response
    QSignalSpy spy(AIEngine::instance(), SIGNAL(responseReady(QString)));
    QVERIFY(spy.wait(5000));
    
    // Verify action taken
    auto source = obs_frontend_get_current_scene();
    // ... verify mic muted
}
```

---

## Security Considerations

### 1. Input Validation

All user input validated before AI processing:
- Length limits
- Character encoding
- Injection prevention

### 2. Output Sanitization

AI responses sanitized before:
- Display in chat
- Execution as commands
- Storage in logs

### 3. Credential Protection

Never send to AI:
- API keys
- Passwords
- Personal information
- Payment details

### 4. Audit Logging

Log all AI interactions:
- Timestamp
- Input (sanitized)
- Output
- Action taken
- User ID

---

## Future Enhancements

1. **Voice Commands**: Speech-to-intent pipeline
2. **Emotion Detection**: Analyze streamer voice tone
3. **Predictive Alerts**: Anticipate technical issues
4. **Auto-Editing**: AI-powered clip editing
5. **Personalized Responses**: Learn streamer preferences
6. **Multi-Modal**: Image/video understanding
7. **Real-Time Coaching**: Live performance feedback

---

## Dependencies

- Qt Network (HTTP client)
- Qt Concurrent (background processing)
- Ollama (local inference)
- Optional: Cloud AI SDKs

---

*This document evolves as AI implementation progresses.*
