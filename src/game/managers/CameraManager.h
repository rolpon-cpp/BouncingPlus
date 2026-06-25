//
// Created by Rolpon on 10/23/2025.
//

#ifndef BOUNCINGPLUS_CAMERA_H
#define BOUNCINGPLUS_CAMERA_H

#include <raylib.h>

class Game;

class CameraManager {
    void ProcessCameraShake();
    void UpdateScreenImageSize();
    void BackgroundLines();
    void UpdateCamera();
    float GetNaturalZoom();

    bool CamTextureInitialized;

    float CameraSpeed;

    int CameraShakes;
    float CameraShakeIntensity;
    double CameraShakeTimer;
    Vector2 CameraShakeOffset;

    float BackgroundDepth;
    float BackgroundGridSize;
    Color BackgroundColor;

    int uWidth;
    int uHeight;
    int uPixelSize;
    int uImpactFrame;
    int uImpactFrameRadius;

    bool ShowLines;

    int BGTexture;

    double ZoomResetTimer;
    public:
        int IntendedScreenWidth;
        int IntendedScreenHeight;
        RenderTexture CameraRenderTexture;
        Game* game;
        Vector2 CameraPosition;
        Vector2 CameraPositionUnaffected;
        Vector2 CameraTarget;
        Camera2D RaylibCamera;
        float CameraZoom;
        float ShaderPixelPower;
        CameraManager(Game *game);
        CameraManager();
        ~CameraManager();
        void BeginRenderTexture(RenderTexture &Tex, bool UseMainCamera = false);
        void EndRenderTexture();
        void StopCamera();
        void BeginCamera();
        void ShakeCamera(float Intensity);
        void QuickZoom(float Zoom, double Time, bool Instant = false);
        void Display(int ImpactFrame = 0, float ImpactFrameRadius = -1.0f);
        void Begin();
        void End();
        void Clear();
        void Quit();
};


#endif //BOUNCINGPLUS_CAMERA_H