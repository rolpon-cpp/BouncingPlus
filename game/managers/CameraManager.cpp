//
// Created by lalit on 10/23/2025.
//

#include "CameraManager.h"

#include <iostream>
#include <raymath.h>

#include "../Game.h"

void CameraManager::Clear() {
    if (CamTextureInitialized)
        UnloadRenderTexture(CameraRenderTexture);
    CameraPosition = Vector2{0.0f, 0.0f};
    CameraTarget = Vector2{0.0f, 0.0f};
    CameraPositionUnaffected = {0, 0};
    CameraSpeed = 20.0f;
    CameraZoom = 1.0f;
    CameraShakes = 0;
    CameraShakeIntensity = 0;
    CameraShakeOffset = {0, 0};
    CameraShakeTimer = GetTime();
    CameraRenderTexture = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    BackgroundDepth = 3;
    BackgroundGridSize = 36;
    BackgroundColor = {100, 100, 100, 255};
    CamTextureInitialized = true;
    BGTexture = GetRandomValue(1, 3);
    ShowLines = GetRandomValue(1, 1000) == 783;
    ZoomResetTimer= 0;
    ShaderPixelPower = 2;
    uWidth = -1;
    uHeight = -1;
    uPixelSize = -1;
    uImpactFrame = -1;
    RaylibCamera = {{0,0}, {0, 0}, 0, 1.0f};
}

CameraManager::CameraManager(Game &game) {
    this->game = &game;
    CamTextureInitialized = false;
    IntendedScreenWidth = 1480;
    IntendedScreenHeight = 920;
    Clear();
}

CameraManager::CameraManager() {
}

CameraManager::~CameraManager() {
}

void CameraManager::QuickZoom(float Zoom, double Time, bool Instant) {
    this->CameraZoom = Zoom;
    if (Instant)
        RaylibCamera.zoom = Zoom * GetNaturalZoom();
    ZoomResetTimer = Time;
}

void CameraManager::Display(int ImpactFrame) {
    BeginShaderMode(game->GameResources.Shaders["main_game"]);
    int w = CameraRenderTexture.texture.width;
    int h = CameraRenderTexture.texture.height;
    if (uWidth == -1 || uHeight == -1 || uPixelSize == -1 || uImpactFrame) {
        uWidth = GetShaderLocation(this->game->GameResources.Shaders["main_game"], "renderWidth");
        uHeight = GetShaderLocation(this->game->GameResources.Shaders["main_game"], "renderHeight");
        uPixelSize = GetShaderLocation(this->game->GameResources.Shaders["main_game"], "pixelSize");
        uImpactFrame = GetShaderLocation(this->game->GameResources.Shaders["main_game"], "impactFrame");
    }
    SetShaderValue(game->GameResources.Shaders["main_game"], uWidth, &w, SHADER_UNIFORM_INT);
    SetShaderValue(game->GameResources.Shaders["main_game"], uHeight, &h, SHADER_UNIFORM_INT);
    SetShaderValue(game->GameResources.Shaders["main_game"], uPixelSize, &ShaderPixelPower, SHADER_UNIFORM_FLOAT);
    SetShaderValue(game->GameResources.Shaders["main_game"], uImpactFrame, &ImpactFrame, SHADER_UNIFORM_INT);
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    DrawTexturePro(CameraRenderTexture.texture, {0, 0, (float)GetRenderWidth(), (float)-GetRenderHeight()}, {0, 0, (float)GetRenderWidth(), (float)GetRenderHeight()}, {0,0},0, WHITE);
    EndBlendMode();
    EndShaderMode();
}

void CameraManager::ProcessCameraShake() {
    if (GetTime() - CameraShakeTimer >= 0.02f && game->GameShared->ShakeCamera) {
        if (CameraShakes > 0) {
            CameraShakeOffset = {(float)GetRandomValue((int)(-50 * CameraShakeIntensity), (int)(50 * CameraShakeIntensity)), (float)GetRandomValue((int)(-50 * CameraShakeIntensity), (int)(50 * CameraShakeIntensity))};
            CameraShakes--;
        } else {
            CameraShakeOffset = {0, 0};
        }
        CameraShakeTimer = GetTime();
    }
}

void CameraManager::UpdateScreenImageSize() {
    if (CameraRenderTexture.texture.width != GetRenderWidth() || CameraRenderTexture.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(CameraRenderTexture);
        CameraRenderTexture = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }
}

void CameraManager::BackgroundLines() {
    float ParallaxCamX = CameraPosition.x / BackgroundDepth;
    float ParallaxCamY = CameraPosition.y / BackgroundDepth;

    if (ShowLines)
    {
        for (int i = -1; i < round(GetRenderHeight() / BackgroundGridSize)+1; i++) {
            int y = (int)(ParallaxCamY / BackgroundGridSize);
            DrawLineEx({CameraPosition.x, ((y+i)*BackgroundGridSize) - ParallaxCamY + CameraPosition.y}, {(float) GetRenderWidth()+CameraPosition.x, ((y+i)*BackgroundGridSize) - ParallaxCamY + CameraPosition.y}, 7, ColorBrightness(WHITE, -0.5f));
        }

        for (int i = -1; i < round(GetRenderWidth() / BackgroundGridSize)+1; i++) {
            int x = (int)(ParallaxCamX / BackgroundGridSize);
            DrawLineEx({((x+i)*BackgroundGridSize) - ParallaxCamX + CameraPosition.x, CameraPosition.y}, {((x+i)*BackgroundGridSize) - ParallaxCamX + CameraPosition.x, (float) GetRenderHeight()+CameraPosition.y}, 7, ColorBrightness(WHITE, -0.5f));
            DrawLineEx({((x+i)*BackgroundGridSize) - ParallaxCamX + CameraPosition.x, CameraPosition.y}, {((x+i)*BackgroundGridSize) - ParallaxCamX + CameraPosition.x, (float) GetRenderHeight()+CameraPosition.y}, 3, ColorAlpha(WHITE, 0.5f));
        }

        for (int i = -1; i < round(GetRenderHeight() / BackgroundGridSize)+1; i++) {
            int y = (int)(ParallaxCamY / BackgroundGridSize);
            DrawLineEx({CameraPosition.x, ((y+i)*BackgroundGridSize) - ParallaxCamY + CameraPosition.y}, {(float) GetRenderWidth()+CameraPosition.x, ((y+i)*BackgroundGridSize) - ParallaxCamY + CameraPosition.y}, 3, ColorAlpha(WHITE, 0.5f));
        }
    } else
    {
        Texture& bg = game->GameResources.Textures["bg"+to_string(BGTexture)];

        int times_x = (int) ((game->GameTiles.MapWidth * game->GameTiles.TileSize) / bg.width) + 1;
        int times_y = (int) ((game->GameTiles.MapHeight * game->GameTiles.TileSize) / bg.height) + 1;

        DrawTexturePro(bg, {0, 0, (float)bg.width*3.0f,(float)bg.height*3.0f}, {
            -(ParallaxCamX/2.0f) + CameraPosition.x,
            -(ParallaxCamY/2.0f) + CameraPosition.y,
            (float)bg.width*times_x,(float)bg.height*times_y}, {bg.width*(float)(times_x/2.0f),bg.height*(float)(times_y/2.0f)},0, WHITE);

        DrawTexturePro(bg, {0, 0, (float)bg.width*3.0f,(float)bg.height*3.0f}, {
            -ParallaxCamX + CameraPosition.x,
            -ParallaxCamY + CameraPosition.y,
            (float)bg.width*times_x,(float)bg.height*times_y}, {bg.width*(float)(times_x/2.0f),bg.height*(float)(times_y/2.0f)},0, WHITE);

    }
}

void CameraManager::StopCamera()
{
    EndMode2D();
}

void CameraManager::BeginCamera()
{
    BeginMode2D(RaylibCamera);
}

void CameraManager::BeginRenderTexture(RenderTexture& Tex, bool UseMainCamera)
{
    EndTextureMode();
    if (!UseMainCamera)
        EndMode2D();
    BeginTextureMode(Tex);
    if (UseMainCamera)
        BeginMode2D(RaylibCamera);
}

void CameraManager::EndRenderTexture()
{
    EndTextureMode();
    BeginTextureMode(CameraRenderTexture);
    BeginMode2D(RaylibCamera);
}

void CameraManager::ShakeCamera(float Intensity) {
    this->CameraShakeIntensity = Intensity;
    this->CameraShakeTimer = GetTime();
    this->CameraShakeOffset = {0, 0};
    this->CameraShakes = 14;
}

float CameraManager::GetNaturalZoom()
{
    return (float)GetRenderHeight() / (float)IntendedScreenHeight;
}

void CameraManager::UpdateCamera()
{
    CameraTarget = game->MainPlayer->GetCenter();

    Vector2 MouseOffset = Vector2Subtract(GetMousePosition(), {static_cast<float>(GetRenderWidth()) / 2.0f, static_cast<float>(GetRenderHeight()) / 2.0f});
    MouseOffset = Vector2Divide(MouseOffset, {100,100});

    float TargetX = CameraTarget.x - CameraPositionUnaffected.x - (static_cast<float>(GetRenderWidth()) / 2.0f);
    float TargetY = CameraTarget.y - CameraPositionUnaffected.y - (static_cast<float>(GetRenderHeight()) / 2.0f);

    float ImportantVal = 20.0f * (1.0f / game->GetGameDeltaTime() / 144.0f);
    if (ImportantVal != 0.0f) {
        CameraPositionUnaffected.x += TargetX / ImportantVal;
        CameraPositionUnaffected.y += TargetY / ImportantVal;
        CameraPosition = {CameraPositionUnaffected.x - CameraShakeOffset.x, CameraPositionUnaffected.y - CameraShakeOffset.y};
        CameraPosition = Vector2Add(CameraPosition, MouseOffset);
        CameraPosition = Vector2Add(CameraPosition, Vector2{(5.0f + min(game->MainPlayer->Kills, 7)) * (float)sin(game->GetGameTime()),
            (5.0f + min(game->MainPlayer->Kills, 7)) * (float)cos(game->GetGameTime())});
    }

    RaylibCamera.zoom = Lerp(RaylibCamera.zoom, CameraZoom * GetNaturalZoom(), 4.0f * game->GetGameDeltaTime() * GetNaturalZoom());
    RaylibCamera.target = CameraPosition;
    RaylibCamera.offset = {((float)GetRenderWidth()/2.0f) * (1-RaylibCamera.zoom), ((float)GetRenderHeight()/2.0f) * (1-RaylibCamera.zoom)};
}

void CameraManager::Begin() {
    if (ZoomResetTimer > 0)
        ZoomResetTimer -= game->GetGameDeltaTime();
    if (ZoomResetTimer <= 0)
        CameraZoom = 1.0f;

    UpdateScreenImageSize();
    ProcessCameraShake();
    UpdateCamera();
    BeginTextureMode(CameraRenderTexture);
    ClearBackground(BackgroundColor);
    BeginMode2D(RaylibCamera);
    BackgroundLines();
}

void CameraManager::End() {
    EndTextureMode();
    EndMode2D();
}

void CameraManager::Quit() {
    UnloadRenderTexture(CameraRenderTexture);
}