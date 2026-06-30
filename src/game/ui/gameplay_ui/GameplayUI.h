//
// Created by Rolpon on 9/28/2025.
//

#ifndef BOUNCINGPLUS_UI_H
#define BOUNCINGPLUS_UI_H
#include <raylib.h>

class Game;

class GameplayUI
{


    float CursorMiddleTrans = 0.0f;
    float CursorRotation = 0.0f;

    float HealthBarSize = 250;
    float HealthBarFontSize = 92;
    float LastHealth = 0;

    float DeathTextAnimRot = 0.0f;
    float HealthBarAnimRot = 0.0f;

    int Margin = 15;
    float PanelAlpha = 0.5f;

    Vector2 StressShakePos = {0, 0};
    double LastChangedStressShakePos = 0;

    float RankSideSize = 100.0f;
    float RankSizeHeight = 250.0f;

    bool WasPausedLast = false;
    bool IsChangingSettings = false;

    Game* game;
public:

    float StartingBlackScreenTrans = 1.0f;
    float EndBlackScreenTrans = 0.0f;

    GameplayUI(Game* game);
    GameplayUI();

    // in GameplayUIMenus
    void PauseMenu();
    void DeathMenu();
    void GameWin();

    // in GameplayUIHeavy
    void DisplayLevelTimer();
    void DisplayInventoryIndicator();
    void DisplayRank();
    void DisplayCursor();

    // in GameplayUIMeters
    void DisplayKillMeter();
    void DisplayPowerupMeter();
    void DisplayHealthMeter();

    void GameUI();
    void Clear();
    void Quit();
};


#endif //BOUNCINGPLUS_UI_H