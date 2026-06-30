//
// Created by Rolpon on 9/28/2025.
//

#include "GameplayUI.h"
#include "../../Game.h"
#include "../../../entities/subentities/player/Player.h"
#include "../../core/Controls.h"
#include "../../managers/ResourceManager.h"
#include "../../managers/GameModeManager.h"

GameplayUI::GameplayUI(Game* game)
{
    this->game = game;
    Clear();
}

GameplayUI::GameplayUI()
{
}

void GameplayUI::Clear()
{
    this->StartingBlackScreenTrans = 1.0f;
    this->EndBlackScreenTrans = 0.0f;
    this->CursorMiddleTrans = 0.0f;
    this->CursorRotation = 0.0f;
    this->HealthBarSize = 250;
    this->DeathTextAnimRot = 0.0f;
    this->HealthBarAnimRot = 0.0f;
    this->LastHealth = 0;
    this->HealthBarFontSize = 92;
    this->Margin = 15;
    this->PanelAlpha = 0.5f;

    this->StressShakePos = {0, 0};
    this->LastChangedStressShakePos = 0;
    this->RankSideSize = 100.0f;
    this->RankSizeHeight = 250.0f;

    this->WasPausedLast = false;
    this->IsChangingSettings = false;
}

void GameplayUI::GameUI()
{
    // kill counter
    DisplayKillMeter();

    // powerup meter
    DisplayPowerupMeter();

    // health meter
    DisplayHealthMeter();

    if (game->DebugDraw && game->MainPlayer->MainWeaponsSystem.CurrentWeapon != nullptr)
        DrawText(("Weapon info " + to_string(game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex) + " "
                     + to_string(
                         game->MainPlayer->MainWeaponsSystem.AttackCooldowns[game->MainPlayer->MainWeaponsSystem.
                             CurrentWeaponIndex]) + " " +
                     game->MainPlayer->MainWeaponsSystem.Weapons[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex]
                     + " " +
                     to_string(
                         game->MainPlayer->MainWeaponsSystem.WeaponAmmo[game->MainPlayer->MainWeaponsSystem.
                                                                              CurrentWeaponIndex])
                 ).c_str(), 0, 0, 25, WHITE);

    DeathTextAnimRot = sin(GetTime() * 2) * 6;

    if (game->MainPlayer->ShouldDelete || game->MainPlayer->Health <= 0)
        DeathMenu();

    if (game->GameMode->WonLevel)
        GameWin();

    // text shaking
    if (game->GetGameTime() - LastChangedStressShakePos >= 0.1f - (game->MainPlayer->LogicProcessor.RankLevel * 0.09f))
    {
        StressShakePos = {
            GetRandomValue(-30, 30) / 10.0f,
            GetRandomValue(-30, 30) / 10.0f
        };
        LastChangedStressShakePos = game->GetGameTime();
    }

    // lvl timer
    DisplayLevelTimer();

    // rank
    DisplayRank();

    // inventory indicator
    DisplayInventoryIndicator();

    if (game->Paused)
        PauseMenu();
    else
    {
        DisplayCursor();
        WasPausedLast = false;
    }

    StartingBlackScreenTrans -= 0.65f * GetFrameTime();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), ColorAlpha(BLACK, StartingBlackScreenTrans));
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), ColorAlpha(BLACK, EndBlackScreenTrans));
}

void GameplayUI::Quit()
{

}