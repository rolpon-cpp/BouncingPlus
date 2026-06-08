//
// Created by lalit on 3/13/2026.
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
    bool Fullscreen = false;
    bool CursorWindowLock = false;
    bool ShakeCamera = true;
    bool DevMode = true;

    // UI Variables
    bool ControlBindingsMenu = false;
    bool VolumeBarInteraction = false;
    bool FrameRateBarInteraction = false;
    float ControlSettingsScroll = 0.0f;
    RenderTexture ControlsRenderTexture = { 0 };

    // Processing
    float LastVolumeBar = 100.0f;
    float LastFrameRateBar = 60.0f;
    float LastFrameRate = -1;

    SharedManager();
    ~SharedManager();

    // Functions
    void ControlBindings(Vector2 Position, float Offset1, float Offset2);
    void DisplaySettings(Vector2 Position, float Offset1, float Offset2);
    void ResetSettings();
    void ReloadLevels();
    void Update();
    void Quit();
};

#endif //BOUNCINGPLUS_SETTINGS_H