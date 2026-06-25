//
// Created by Rolpon on 3/13/2026.
//

#ifndef BOUNCINGPLUS_SETTINGS_H
#define BOUNCINGPLUS_SETTINGS_H
#include <string>
#include <unordered_map>
#include <nlohmann/json_fwd.hpp>

#include "Controls.h"
#include "../../data/Progress.h"
#include "../ui/UIUtils.h"

using namespace nlohmann;

class SharedManager
{
public:
    // Separate Objects
    std::unordered_map<std::string, json> LevelData;
    UIAssets SharedUIAssets;
    Controls Controls;
    Progress Progress;

    // Settings
    float FrameRate = -1;
    float Volume = 0.0f;
    #ifndef PLATFORM_WEB
        float CosmeticParticleLimit = 400.0f;
    #else
        float CosmeticParticleLimit = 150.0f;
    #endif
    bool Fullscreen = false;
    bool CursorWindowLock = false;
    bool ShakeCamera = true;
    bool DevMode = true;

    // UI Variables
    bool VolumeBarInteraction = false;
    bool FrameRateBarInteraction = false;
    bool ParticleBarInteraction = false;

    // UI Processing
    float LastVolumeBar = 100.0f;
    float LastParticleLimitBar = 100.0f;
    float LastFrameRateBar = 60.0f;

    // General Processing
    float LastFrameRate = -1;
    bool QuitGame = false;

    SharedManager();
    ~SharedManager();

    // Functions
    void RequestQuit();
    void DisplaySettings(Vector2 Position, float Offset1, float Offset2);
    void ResetSettings();
    void ReloadLevels();
    void Update();
    void Quit();
};

#endif //BOUNCINGPLUS_SETTINGS_H