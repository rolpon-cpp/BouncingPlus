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
    this->WeaponUITexture = LoadRenderTexture(GetRenderWidth(), 250);
    this->DeathScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    this->PauseScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    this->GameWinScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
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

    this->WeaponSlotIndex = -1;
    this->WeaponSlotOffset = 0;
    this->WeaponSlotSize = 0;
    this->HealthBarSize = 250;
    this->UITransparency = 1.0f;
    this->DeathTextAnimRot = 0.0f;
    this->HealthBarAnimRot = 0.0f;
    this->LastHealth = 0;
    this->FntSize = 92;
    this->Margin = 15;
    this->Alpha = 0.75f;

    this->StressShakePos = {0, 0};
    this->LastChangedStressShakePos = 0;
    this->RankSideSize = 100.0f;
    this->RankSizeHeight = 250.0f;

    WasPausedLast = false;
    IsChangingSettings = false;
}

void GameplayUI::GameUI()
{
    RefreshRenderTextures();

    BeginTextureMode(WeaponUITexture);
    ClearBackground(BLANK);

    float Prev = 125;
    bool Found = false;

    float lowest_x = 50.0f;
    float lowest_y = 125.0f;
    float highest_width = -1;
    float highest_height = -1;

    float tPrev = 0;
    for (int i = 0; i < 3; i++)
    {
        float offset = 0;
        float size = 0;
        if (game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex == i)
        {
            WeaponSlotIndex = i;
        }
        if (WeaponSlotIndex == i)
        {
            offset = WeaponSlotOffset;
            size = WeaponSlotSize;
        }
        std::string name = game->MainPlayer->MainWeaponsSystem.Weapons[i];
        if (name.empty())
            name = "Empty";
        if (i == 2)
            highest_height = tPrev + 20 + size;
        if (MeasureText(name.c_str(), 20 + size) + offset > highest_width)
            highest_width = MeasureText(name.c_str(), 20 + size) + offset;
        tPrev += 35 + size;
    }

    DrawRectangle(lowest_x - Margin, lowest_y - Margin, highest_width + (Margin * 2), highest_height + (Margin * 2),
                  ColorAlpha(BLACK, Alpha));

    // cooldown meter
    DisplayCooldownMeter(lowest_x, lowest_y, highest_width, highest_height);

    // inventory
    DisplayInventory(Found, Prev, highest_width, &highest_height);

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

    EndTextureMode();

    DeathTextAnimRot = sin(GetTime() * 2) * 6;

    BeginTextureMode(DeathScreen);
    ClearBackground(ColorAlpha(RED, 0.2f));
    EndTextureMode();

    BeginTextureMode(GameWinScreen);
    ClearBackground(ColorAlpha(GREEN, 0.2f));
    EndTextureMode();

    if (game->MainPlayer->ShouldDelete || game->MainPlayer->Health <= 0)
        DeathMenu();

    if (game->DebugDraw)
        DrawText(to_string(UITransparency).c_str(), 50, 250, 10, WHITE);

    if (game->GameMode->WonLevel)
        GameWin();
    else
        DrawTextureRec(DeathScreen.texture,
                       Rectangle{0, 0, (float)DeathScreen.texture.width, -(float)DeathScreen.texture.height},
                       Vector2{0, (float)GetRenderHeight() - DeathScreen.texture.height},
                       ColorAlpha(WHITE, 1.0f - UITransparency));

    DrawTextureRec(WeaponUITexture.texture,
                   Rectangle{0, 0, (float)WeaponUITexture.texture.width, -(float)WeaponUITexture.texture.height},
                   Vector2{0.0f, (float)GetRenderHeight() - WeaponUITexture.texture.height},
                   ColorAlpha(WHITE, UITransparency));

    if (game->MainPlayer->Health > 0 && !game->GameMode->WonLevel && UITransparency < 1.0f)
    {
        UITransparency += 1.9f * GetFrameTime();
        if (UITransparency > 1.0f)
            UITransparency = 1.0f;
    }
    else if (UITransparency > 0 && (game->MainPlayer->Health <= 0 || game->GameMode->WonLevel))
    {
        UITransparency -= 2.2f * GetFrameTime();
        if (UITransparency < 0.0f)
            UITransparency = 0.0f;
    }

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

void GameplayUI::RefreshRenderTextures()
{
    if (WeaponUITexture.texture.width != GetRenderWidth())
    {
        UnloadRenderTexture(WeaponUITexture);
        WeaponUITexture = LoadRenderTexture(GetRenderWidth(), WeaponUITexture.texture.height);
    }

    if (DeathScreen.texture.width != GetRenderWidth() || DeathScreen.texture.height != GetRenderHeight())
    {
        UnloadRenderTexture(DeathScreen);
        DeathScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    if (GameWinScreen.texture.width != GetRenderWidth() || GameWinScreen.texture.height != GetRenderHeight())
    {
        UnloadRenderTexture(GameWinScreen);
        GameWinScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    if (PauseScreen.texture.width != GetRenderWidth() || PauseScreen.texture.height != GetRenderHeight())
    {
        UnloadRenderTexture(PauseScreen);
        PauseScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }
}

void GameplayUI::Quit()
{
    UnloadRenderTexture(WeaponUITexture);
    UnloadRenderTexture(PauseScreen);
    UnloadRenderTexture(GameWinScreen);
    UnloadRenderTexture(DeathScreen);
}