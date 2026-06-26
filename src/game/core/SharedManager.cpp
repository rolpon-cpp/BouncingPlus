//
// Created by Rolpon on 3/14/2026.
//

#include "SharedManager.h"
#include "../../globals.h"
#include "../../level/LevelLoader.h"

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

SharedManager::SharedManager()
{

    SharedUIAssets = UIAssets();

}

SharedManager::~SharedManager()
{
}

void SharedManager::Load()
{
    ReloadLevels();
    SharedUIAssets.Load();
    Progress.LoadProgress();
    Controls.SetDefaultBindings();
    CursorWindowLock = Progress.Data.CursorWindowLock;
    FrameRate = Progress.Data.FrameRate;
    Volume = Progress.Data.Volume;
    Fullscreen = Progress.Data.Fullscreen;
    CosmeticParticleLimit = Progress.Data.CosmeticParticleLimit;
    ShakeCamera = Progress.Data.ShakeCamera;
    DevMode = Progress.Data.DevMode;
}

void SharedManager::ReloadLevels()
{
    LevelData.clear();
    LevelData = GetLevelsData();
}

void SharedManager::DisplaySettings(Vector2 Position, float Offset1, float Offset2)
{
    Panel(Rectangle{Position.x + 200, Position.y + 25, GetRenderWidth() - 400.0f, GetRenderHeight() - 200.0f},
          "SETTINGS", -(Offset1 + Offset2) / 4);
    Slider({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4}, GetMousePosition(),
           this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg, "VOLUME", &this->Volume,
           &LastVolumeBar, &VolumeBarInteraction, 0, 100.0f);
    Slider({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 60},
           GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg, "FRAMERATE",
           &this->FrameRate, &LastFrameRateBar, &FrameRateBarInteraction, 30, 300);
    Slider({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 120},
           GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg, "MAX PARTICLES",
           &this->CosmeticParticleLimit, &LastParticleLimitBar, &ParticleBarInteraction, 0, 600);
    Checkmark({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 180},
              GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg,
              this->SharedUIAssets.ButtonSmallRedImg, "CURSOR WINDOW LOCK", &CursorWindowLock);
    Checkmark({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 240},
              GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg,
              this->SharedUIAssets.ButtonSmallRedImg, "DEVELOPER MODE", &DevMode);
    Checkmark({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 300},
              GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg,
              this->SharedUIAssets.ButtonSmallRedImg, "SHAKE CAMERA", &ShakeCamera);
    Checkmark({Position.x + (float)GetRenderWidth() / 2, Position.y + 150 + (Offset1 + Offset2) / 4 + 360},
              GetMousePosition(), this->SharedUIAssets.SliderDrag, this->SharedUIAssets.ButtonSmallImg,
              this->SharedUIAssets.ButtonSmallRedImg, "FULLSCREEN", &Fullscreen);
}

void SharedManager::RequestQuit()
{
    QuitGame = true;
}

void SharedManager::ResetSettings()
{
    VolumeBarInteraction = false;
    FrameRateBarInteraction = false;
}

void SharedManager::Update()
{
#ifndef PLATFORM_WEB
    if (FrameRate != LastFrameRate)
        SetTargetFPS(FrameRate);

    if (Controls.IsControlPressed("fullscreen"))
        Fullscreen = !Fullscreen;

    if (Fullscreen && !IsWindowFullscreenCrossPlatform())
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));

    if (!Fullscreen && IsWindowFullscreenCrossPlatform())
    {
        SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - WINDOW_WIDTH / 2,
                          GetMonitorHeight(GetCurrentMonitor()) / 2 - WINDOW_HEIGHT / 2);
    }

    if (IsWindowFullscreen() != Fullscreen)
        ToggleFullscreen();

    if (CursorWindowLock && !IsCursorOnScreenCrossPlatform())
        SetMousePosition(std::min(std::max(GetMouseX(), 25), GetRenderWidth() - 25),
                         std::min(std::max(GetMouseY(), 25), GetRenderHeight() - 25));

#endif

    SetMasterVolume(Volume / 100.0f);
    LastFrameRate = FrameRate;

    Controls.Update();
}

void SharedManager::Quit()
{
    Progress.Data.CursorWindowLock = CursorWindowLock;
    Progress.Data.FrameRate = FrameRate;
    Progress.Data.CosmeticParticleLimit = CosmeticParticleLimit;
    Progress.Data.Volume = Volume;
    Progress.Data.Fullscreen = Fullscreen;
    Progress.Data.ShakeCamera = ShakeCamera;
    Progress.Data.DevMode = DevMode;

    Progress.SaveProgress();
    SharedUIAssets.Quit();
    Controls.Quit();
}