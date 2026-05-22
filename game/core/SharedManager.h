//
// Created by lalit on 3/13/2026.
//

#ifndef BOUNCINGPLUS_SETTINGS_H
#define BOUNCINGPLUS_SETTINGS_H
#include <map>
#include <string>
#include <nlohmann/json_fwd.hpp>

#include "Controls.h"
#include "../ui/UIUtils.h"

using namespace std;
using namespace nlohmann;

struct SharedManager
{
    // Separate Objects
    map<string, json> LevelData;
    UIAssets UIAssets;
    Controls Controls;

    // Settings
    float FrameRate = -1;
    float Volume = 50.0f;
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
    float LastVolumeBar = 50.0f;
    float LastFrameRateBar = 60.0f;
    float LastFrameRate = -1;

    // Functions
    void ControlBindings(Vector2 Position, float Offset1, float Offset2);
    void DisplaySettings(Vector2 Position, float Offset1, float Offset2);
    void ResetSettings();
    void Update();
    void Quit();
};

#endif //BOUNCINGPLUS_SETTINGS_H