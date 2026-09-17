#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Bosh {

struct AvatarConfig {
    std::string id;
    std::string name;
    std::string htmlPath;
    int width = 300;
    int height = 400;
    bool enabled = true;
    bool showCameraOverlay = true;
    bool interactive = true;
    std::vector<std::string> expressions;
    std::string currentExpression;
};

class AvatarManager {
public:
    static AvatarManager& Instance();
    
    void Initialize();
    void Shutdown();
    
    // Avatar management
    bool LoadAvatar(const std::string& avatarId);
    bool UnloadAvatar();
    bool IsAvatarLoaded() const;
    
    // Configuration
    void SetAvatarEnabled(bool enabled);
    bool IsAvatarEnabled() const;
    void SetCameraOverlayVisible(bool visible);
    void SetExpression(const std::string& expression);
    
    // Get current config
    const AvatarConfig& GetCurrentConfig() const;
    
    // Get available avatars
    std::vector<AvatarConfig> GetAvailableAvatars() const;
    
    // Browser source integration
    std::string GetAvatarURL() const;
    int GetAvatarWidth() const;
    int GetAvatarHeight() const;
    
private:
    AvatarManager() = default;
    ~AvatarManager() = default;
    AvatarManager(const AvatarManager&) = delete;
    AvatarManager& operator=(const AvatarManager&) = delete;
    
    void DiscoverAvatars();
    std::string GetDataPath() const;
    
    mutable std::vector<AvatarConfig> m_avatars;
    AvatarConfig* m_currentAvatar = nullptr;
    bool m_initialized = false;
    bool m_enabled = true;
};

} // namespace Bosh
