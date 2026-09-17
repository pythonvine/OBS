#include "avatar-manager.hpp"
#include "../core/bosh-context.hpp"
#include <obs.h>
#include <util/platform.h>
#include <filesystem>
#include <algorithm>

namespace Bosh {

AvatarManager& AvatarManager::Instance() {
    static AvatarManager instance;
    return instance;
}

void AvatarManager::Initialize() {
    if (m_initialized) {
        return;
    }
    
    DiscoverAvatars();
    
    // Load default avatar (banana)
    if (!m_avatars.empty()) {
        LoadAvatar("banana");
    }
    
    m_initialized = true;
    
    blog(LOG_INFO, "[Bosh] Avatar Manager initialized with %zu avatars", m_avatars.size());
}

void AvatarManager::Shutdown() {
    if (!m_initialized) {
        return;
    }
    
    UnloadAvatar();
    m_avatars.clear();
    m_initialized = false;
    
    blog(LOG_INFO, "[Bosh] Avatar Manager shutdown complete");
}

bool AvatarManager::LoadAvatar(const std::string& avatarId) {
    auto it = std::find_if(m_avatars.begin(), m_avatars.end(),
        [&avatarId](const AvatarConfig& config) {
            return config.id == avatarId;
        });
    
    if (it == m_avatars.end()) {
        blog(LOG_WARNING, "[Bosh] Avatar '%s' not found", avatarId.c_str());
        return false;
    }
    
    UnloadAvatar();
    m_currentAvatar = &(*it);
    m_currentAvatar->enabled = true;
    
    blog(LOG_INFO, "[Bosh] Loaded avatar: %s (%s)", 
         m_currentAvatar->name.c_str(), m_currentAvatar->id.c_str());
    
    return true;
}

bool AvatarManager::UnloadAvatar() {
    if (m_currentAvatar) {
        m_currentAvatar = nullptr;
        blog(LOG_INFO, "[Bosh] Unloaded current avatar");
        return true;
    }
    return false;
}

bool AvatarManager::IsAvatarLoaded() const {
    return m_currentAvatar != nullptr;
}

void AvatarManager::SetAvatarEnabled(bool enabled) {
    m_enabled = enabled;
    if (m_currentAvatar) {
        m_currentAvatar->enabled = enabled;
    }
}

bool AvatarManager::IsAvatarEnabled() const {
    return m_enabled && (m_currentAvatar ? m_currentAvatar->enabled : false);
}

void AvatarManager::SetCameraOverlayVisible(bool visible) {
    if (m_currentAvatar) {
        m_currentAvatar->showCameraOverlay = visible;
    }
}

void AvatarManager::SetExpression(const std::string& expression) {
    if (m_currentAvatar && !expression.empty()) {
        m_currentAvatar->currentExpression = expression;
        blog(LOG_DEBUG, "[Bosh] Avatar expression set to: %s", expression.c_str());
    }
}

const AvatarConfig& AvatarManager::GetCurrentConfig() const {
    static AvatarConfig emptyConfig;
    return m_currentAvatar ? *m_currentAvatar : emptyConfig;
}

std::vector<AvatarConfig> AvatarManager::GetAvailableAvatars() const {
    return m_avatars;
}

std::string AvatarManager::GetAvatarURL() const {
    if (!m_currentAvatar || !m_currentAvatar->enabled) {
        return "";
    }
    
    std::string dataPath = GetDataPath();
    std::string fullPath = dataPath + "/avatars/" + m_currentAvatar->htmlPath;
    
    // Convert to file URL for browser source
    return "file:///" + fullPath;
}

int AvatarManager::GetAvatarWidth() const {
    return m_currentAvatar ? m_currentAvatar->width : 300;
}

int AvatarManager::GetAvatarHeight() const {
    return m_currentAvatar ? m_currentAvatar->height : 400;
}

void AvatarManager::DiscoverAvatars() {
    std::string dataPath = GetDataPath();
    std::string avatarsDir = dataPath + "/avatars";
    
    try {
        if (!std::filesystem::exists(avatarsDir)) {
            std::filesystem::create_directories(avatarsDir);
            blog(LOG_INFO, "[Bosh] Created avatars directory: %s", avatarsDir.c_str());
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(avatarsDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".html") {
                AvatarConfig config;
                config.id = entry.path().stem().string();
                config.name = config.id;
                config.htmlPath = entry.path().filename().string();
                
                // Capitalize first letter for display name
                if (!config.name.empty()) {
                    config.name[0] = std::toupper(config.name[0]);
                    config.name += " Avatar";
                }
                
                // Add default expressions
                config.expressions = {"neutral", "happy", "excited", "surprised"};
                config.currentExpression = "neutral";
                
                m_avatars.push_back(config);
                blog(LOG_DEBUG, "[Bosh] Discovered avatar: %s", config.id.c_str());
            }
        }
    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[Bosh] Error discovering avatars: %s", e.what());
    }
}

std::string AvatarManager::GetDataPath() const {
    char* path = obs_module_file("data");
    if (!path) {
        // Fallback to module data path
        path = os_get_config_path_ptr("bosh/data");
    }
    
    std::string result = path ? path : "";
    bfree(path);
    return result;
}

} // namespace Bosh
