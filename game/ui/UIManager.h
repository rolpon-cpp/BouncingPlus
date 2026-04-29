//
// Created by lalit on 9/28/2025.
//

#ifndef BOUNCINGPLUS_UI_H
#define BOUNCINGPLUS_UI_H
#include <raylib.h>

class Game;

class UIManager {
public:
    float StartingBlackScreenTrans = 1.0f;
    float EndBlackScreenTrans = 0.0f;

    RenderTexture2D WeaponUITexture;
    RenderTexture2D DeathScreen;
    RenderTexture2D PauseScreen;
    RenderTexture2D GameWinScreen;

    float CursorMiddleTrans = 0.0f;
    float CursorRotation = 0.0f;

    int WeaponSlotIndex = -1;
    float WeaponSlotOffset = 0;
    float WeaponSlotSize = 0;
    float HealthBarSize = 250;
    float UITransparency = 1.0f;
    float DeathTextAnimRot = 0.0f;
    float HealthBarAnimRot = 0.0f;
    float LastHealth = 0;
    float FntSize = 92;
    int Margin = 15;
    float Alpha = 0.75f;

    Vector2 StressShakePos = {0,0};
    double LastChangedStressShakePos = 0;

    float RankSideSize = 100.0f;
    float RankSizeHeight = 250.0f;

    Game *game;
    UIManager(Game &game);
    UIManager();
    void RefreshRenderTextures();
    void PauseMenu();
    void DeathMenu();
    void DisplayTopHUD();
    void DisplayCursor();
    void DisplayHealthMeter();
    void DisplayCooldownMeter(float LowestX, float LowestY, float HighestWidth, float HighestHeight);
    void DisplayInventory(bool Found, float Prev, float HighestWidth, float* HighestHeight);
    void DisplayKillMeter();
    void DisplayPowerupMeter();
    void DisplayRank();
    void GameWin();
    void GameUI();
    void Clear();
    void Quit();
};


#endif //BOUNCINGPLUS_UI_H