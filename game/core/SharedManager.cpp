//
// Created by lalit on 3/14/2026.
//

#include "SharedManager.h"

#include <iostream>

bool IsWindowFullscreenCrossPlatform()
{
#ifdef PLATFORM_WEB
    return false;
#else
    return IsWindowFullscreen();
#endif
}

bool IsCursorOnScreenCrossPlatform()
{
#ifdef PLATFORM_WEB
    return true;
#else
    return IsCursorOnScreen();
#endif
}

void SharedManager::DisplaySettings(Vector2 Position, float Offset1, float Offset2)
{
    if (!ControlBindingsMenu)
    {
        Panel(Rectangle{Position.x + 200, Position.y + 25, GetRenderWidth() - 400.0f, GetRenderHeight() - 200.0f}, "SETTINGS", -(Offset1 + Offset2)/4);
        Slider({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, "VOLUME", &this->Volume, &LastVolumeBar, &VolumeBarInteraction, 0, 100.0f);
        Slider({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4 + 60}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, "FRAMERATE", &this->FrameRate, &LastFrameRateBar, &FrameRateBarInteraction, 30, 240);
        Checkmark({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4 + 120}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, this->UIAssets.ButtonSmallRedImg, "CURSOR WINDOW LOCK", &CursorWindowLock);
        Checkmark({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4 + 180}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, this->UIAssets.ButtonSmallRedImg, "DEVELOPER MODE", &DevMode);
        Checkmark({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4 + 240}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, this->UIAssets.ButtonSmallRedImg, "SHAKE CAMERA", &ShakeCamera);
        Checkmark({Position.x + (float)GetRenderWidth() /2,Position.y + 150 + (Offset1 + Offset2)/4 + 300}, GetMousePosition(), this->UIAssets.SliderDrag, this->UIAssets.ButtonSmallImg, this->UIAssets.ButtonSmallRedImg, "FULLSCREEN", &Fullscreen);
        if (Button({Position.x + (float)GetRenderWidth() /2 - UIAssets.ButtonImg.width/2, Position.y + 150 + (Offset1 + Offset2)/4 + 360, (float)UIAssets.ButtonImg.width, (float)UIAssets.ButtonImg.height},
            GetMousePosition(), UIAssets.ButtonImg, UIAssets.ButtonClick, "CONTROLS"
            ))
            ControlBindingsMenu = true;
    } else
        ControlBindings(Position,Offset1,Offset2);
}

void SharedManager::ResetSettings()
{
    ControlBindingsMenu = false;
    VolumeBarInteraction = false;
    FrameRateBarInteraction = false;
    ControlSettingsScroll = 0.0f;

    if (IsRenderTextureValid(ControlsRenderTexture))
        UnloadRenderTexture(ControlsRenderTexture);

    ControlsRenderTexture = { 0 };
}

void SharedManager::ControlBindings(Vector2 Position, float Offset1, float Offset2)
{
    BeginTextureMode(ControlsRenderTexture);

    ClearBackground(ColorAlpha(BLACK, 0.5f));

    EndTextureMode();

    Rectangle PanelRect = Rectangle{Position.x + 300, Position.y + 25, GetRenderWidth() - 400.0f, GetRenderHeight() - 200.0f};
    Panel(PanelRect, "CONTROLS", -(Offset1 + Offset2)/4);

    DrawTexturePro(ControlsRenderTexture.texture, {0, 0, (float)ControlsRenderTexture.texture.width, (float)-ControlsRenderTexture.texture.height}, {
        PanelRect.x + PanelRect.width/2 - ControlsRenderTexture.texture.width/2,
        PanelRect.y + PanelRect.height/2 - ControlsRenderTexture.texture.height/2 + (Offset1 + Offset2)/4 + 25.0f,
        (float)ControlsRenderTexture.texture.width,
        (float)ControlsRenderTexture.texture.height
    }, {0, 0}, 0, WHITE);

    if (Button({Position.x + (float)GetRenderWidth() /2 - UIAssets.ButtonImg.width/2, Position.y + PanelRect.height + 100.0f + (Offset1 + Offset2)/4, (float)UIAssets.ButtonImg.width, (float)UIAssets.ButtonImg.height},
            GetMousePosition(), UIAssets.ButtonImg, UIAssets.ButtonClick, "CANCEL"
            ))
        ControlBindingsMenu = false;
}

void SharedManager::Update()
{

    Vector2 RenderTexSize = {max(GetRenderWidth() - 850.0f, 100.0f), max(GetRenderHeight() - 350.0f, 100.0f)};

    if (!IsRenderTextureValid(ControlsRenderTexture) || ControlsRenderTexture.texture.width != RenderTexSize.x || ControlsRenderTexture.texture.height != RenderTexSize.y)
    {
        if (IsRenderTextureValid(ControlsRenderTexture))
            UnloadRenderTexture(ControlsRenderTexture);
        ControlsRenderTexture = LoadRenderTexture(RenderTexSize.x, RenderTexSize.y);
    }

    if (FrameRate != LastFrameRate)
    {
        #ifndef PLATFORM_WEB
            SetTargetFPS((int)FrameRate);
        #endif
    }
    LastFrameRate = FrameRate;

    if (Controls.IsControlPressed("fullscreen"))
        Fullscreen = !Fullscreen;

    if (Fullscreen && !IsWindowFullscreenCrossPlatform())
    {
#ifndef PLATFORM_WEB
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
#endif
    }

    if (!Fullscreen && IsWindowFullscreenCrossPlatform())
    {
#ifndef PLATFORM_WEB
        SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - GetRenderWidth()/2, GetMonitorHeight(GetCurrentMonitor())/2 - GetRenderHeight()/2);
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) / 1.2f, GetMonitorHeight(GetCurrentMonitor()) / 1.2f);
#endif
    }

    if (IsWindowFullscreen() != Fullscreen)
    {
        #ifndef PLATFORM_WEB
        ToggleFullscreen();
        #endif
    }

    if (CursorWindowLock && !IsCursorOnScreenCrossPlatform())
    {

#ifndef PLATFORM_WEB
        SetMousePosition(min(max(GetMouseX(), 25), GetRenderWidth() - 25), min(max(GetMouseY(), 25), GetRenderHeight() - 25));
#endif
    }

    SetMasterVolume(Volume / 100.0f);

    Controls.Update();
}

void SharedManager::Quit()
{
    UIAssets.Quit();
    Controls.Quit();
}
