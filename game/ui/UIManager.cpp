//
// Created by lalit on 9/28/2025.
//

#include "UIManager.h"
#include <algorithm>
#include <iostream>
#include <raymath.h>

#include "../Game.h"

UIManager::UIManager(Game &game) {
    this->game = &game;
    this->WeaponUITexture = LoadRenderTexture(GetRenderWidth(), 250);
    this->DeathScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    this->PauseScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    this->GameWinScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    Clear();
}

UIManager::UIManager() {
}

void UIManager::Clear()
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

    this->StressShakePos = {0,0};
    this->LastChangedStressShakePos = 0;
    this->RankSideSize = 100.0f;
    this->RankSizeHeight = 250.0f;
}

void UIManager::GameUI()
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
    for (int i = 0; i < 3; i ++ )
    {
        float offset = 0;
        float size = 0;
        if (game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex == i) {
            WeaponSlotIndex = i;
        }
        if (WeaponSlotIndex == i)
        {
            offset=WeaponSlotOffset;
            size=WeaponSlotSize;
        }
        std::string name = game->MainPlayer->MainWeaponsSystem.Weapons[i];
        if (name.empty())
            name = "Empty";
        if (i == 2)
            highest_height = tPrev + 20 + size;
        if (MeasureText(name.c_str(), 20+size)+offset > highest_width)
            highest_width = MeasureText(name.c_str(), 20+size)+offset;
        tPrev += 35 + size;
    }

    DrawRectangle(lowest_x-Margin,lowest_y-Margin,highest_width+(Margin*2),highest_height+(Margin*2),ColorAlpha(BLACK, Alpha));

    // cooldown meter
    DisplayCooldownMeter(lowest_x, lowest_y, highest_width, highest_height);

    DisplayInventory(Found, Prev, highest_width, &highest_height);

    // kill counter
    DisplayKillMeter();

    // powerup meter
    DisplayPowerupMeter();

    // health meter
    DisplayHealthMeter();

    if (game->DebugDraw && game->MainPlayer->MainWeaponsSystem.CurrentWeapon != nullptr)
        DrawText(("Weapon info " + to_string(game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex) + " "
            + to_string(game->MainPlayer->MainWeaponsSystem.AttackCooldowns[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex]) + " " +
            game->MainPlayer->MainWeaponsSystem.Weapons[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex] + " " +
            to_string(game->MainPlayer->MainWeaponsSystem.WeaponAmmo[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex])
            ).c_str(), 0, 0, 25, WHITE);

    EndTextureMode();

    DeathTextAnimRot = sin(GetTime()*2) * 6;

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

    if (game->GameMode.WonLevel)
        GameWin();
    else
        DrawTextureRec(DeathScreen.texture, Rectangle{0, 0, (float)DeathScreen.texture.width, -(float)DeathScreen.texture.height}, Vector2{0, (float)GetRenderHeight() - DeathScreen.texture.height}, ColorAlpha(WHITE, 1.0f - UITransparency));

    DrawTextureRec(WeaponUITexture.texture, Rectangle{0, 0, (float)WeaponUITexture.texture.width, -(float)WeaponUITexture.texture.height}, Vector2{0.0f, (float)GetRenderHeight() - WeaponUITexture.texture.height}, ColorAlpha(WHITE, UITransparency));

    if (game->MainPlayer->Health > 0 && !game->GameMode.WonLevel && UITransparency < 1.0f) {
        UITransparency += 1.9f * GetFrameTime();
        if (UITransparency > 1.0f)
            UITransparency = 1.0f;
    } else if (UITransparency > 0 && (game->MainPlayer->Health <= 0 || game->GameMode.WonLevel)) {
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

    // top hud
    DisplayTopHUD();

    // rank
    DisplayRank();

    if (game->Paused)
        PauseMenu();
    else
        DisplayCursor();
    
    StartingBlackScreenTrans -= 0.65f * GetFrameTime();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), ColorAlpha(BLACK, StartingBlackScreenTrans));
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), ColorAlpha(BLACK, EndBlackScreenTrans));
}

void UIManager::DisplayKillMeter()
{
    DrawRectangle(GetRenderWidth()-(125+Margin), WeaponUITexture.texture.height - (100+Margin), 125, 100, ColorAlpha(BLACK, Alpha));
    float s2 = 70;
    float s = MeasureText(to_string(game->MainPlayer->Kills).c_str(), s2);
    DrawText(to_string(game->MainPlayer->Kills).c_str(), (int)(GetRenderWidth()-(125+Margin) + 125/2 - s/2),
    WeaponUITexture.texture.height - (100+Margin) + 10, s2, WHITE);

    float s3 = 20;
    float s4 = MeasureText("Kills", s3);
    DrawText("Kills", (int)(GetRenderWidth()-(125+Margin) + 125/2 - s4/2),
    WeaponUITexture.texture.height - (100+Margin) + 100 - s3 - 10, s3, WHITE
        );
}

void UIManager::RefreshRenderTextures()
{
    if (WeaponUITexture.texture.width != GetRenderWidth()) {
        UnloadRenderTexture(WeaponUITexture);
        WeaponUITexture = LoadRenderTexture(GetRenderWidth(), WeaponUITexture.texture.height);
    }

    if (DeathScreen.texture.width != GetRenderWidth() || DeathScreen.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(DeathScreen);
        DeathScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    if (GameWinScreen.texture.width != GetRenderWidth() || GameWinScreen.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(GameWinScreen);
        GameWinScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    if (PauseScreen.texture.width != GetRenderWidth() || PauseScreen.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(PauseScreen);
        PauseScreen = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }
}

void UIManager::DisplayPowerupMeter()
{
    float s3 = 0;
    float s4 = 0;
    if (game->MainPlayer->MainPowerupSystem.CurrentPowerup != nullptr)
    {
        DrawRectangle(GetRenderWidth()-(125+Margin)*2, WeaponUITexture.texture.height - (100+Margin), 125, 100, ColorAlpha(BLACK, Alpha));

        s3 = 20;
        s4 = MeasureText("Powerup", s3);
        DrawText("Powerup", (int)(GetRenderWidth()-(125+Margin)*2 + 125/2 - s4/2),
        WeaponUITexture.texture.height - (100+Margin) + 100 - s3 - 10, s3, WHITE
            );
        float Number = game->MainPlayer->MainPowerupSystem.CurrentLength;
        Color MainColor = YELLOW;
        if (!game->MainPlayer->MainPowerupSystem.PowerupIsActive)
        {
            Number = game->MainPlayer->MainPowerupSystem.CurrentCooldown;
            MainColor = RED;
            if (Number <= 0)
            {
                MainColor = GREEN;
                Number = 0;
            }
        }
        Number = round(Number);
        s4 = MeasureText(game->MainPlayer->MainPowerupSystem.CurrentPowerup->Name.c_str(), s3);
        DrawText(game->MainPlayer->MainPowerupSystem.CurrentPowerup->Name.c_str(), (int)(GetRenderWidth()-(125+Margin)*2 + 125/2 - s4/2),
        WeaponUITexture.texture.height - (100+Margin) + 10, s3, WHITE
            );
        float s2 = 50;
        float s = MeasureText((to_string((int)Number) + "s").c_str(), s2);
        DrawText((to_string((int)Number) + "s").c_str(), (int)(GetRenderWidth()-(125+Margin)*2 + 125/2 - s/2),
        WeaponUITexture.texture.height - (100+Margin) + 50 - s2/2, s2, MainColor);
    }
}

void UIManager::DisplayInventory(bool Found, float Prev, float HighestWidth, float* HighestHeight)
{
    for (int i = 0; i < 3; i++)
    {
        std::string name = game->MainPlayer->MainWeaponsSystem.Weapons[i];
        if (name.empty())
            name = "Empty";
        Color MainColor = RED;
        if (game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex == i) {
            Found = true;
            WeaponSlotIndex = i;
        }
        float offset = 0;
        float size = 0;
        if (WeaponSlotIndex == i) {
            MainColor = GREEN;
            if (Found) {
                WeaponSlotOffset += 240 * GetFrameTime();
                WeaponSlotSize += 100 * GetFrameTime();
            }
            WeaponSlotOffset = clamp(WeaponSlotOffset, 0.0f, 60.0f);
            WeaponSlotSize = clamp(WeaponSlotSize, 0.0f, 15.0f);
            offset=WeaponSlotOffset;
            size=WeaponSlotSize;
        }
        if (MeasureText(name.c_str(), 20+size) > HighestWidth)
        {
            HighestWidth = MeasureText(name.c_str(), 20+size)+offset;
        }
        if (i == 2)
            *HighestHeight = Prev + 20 + size;
        DrawLineEx(Vector2{50, Prev+2.0f},
                Vector2{(50 + MeasureText(name.c_str(), 20+size)+offset), Prev+2.0f}, 4,
                       MainColor);
        DrawText(name.c_str(), 50, Prev+4.0f, 20+size, MainColor);
        Prev += 20 + size + 15;
    }
    if (!Found) {
        WeaponSlotOffset -= 240 * GetFrameTime();
        WeaponSlotSize -= 100 * GetFrameTime();
        WeaponSlotOffset = clamp(WeaponSlotOffset, 0.0f, 60.0f);
        WeaponSlotSize = clamp(WeaponSlotSize, 0.0f, 15.0f);
        if (WeaponSlotOffset <= 0 && WeaponSlotSize <= 0)
            WeaponSlotIndex = -1;
    }
}

void UIManager::DisplayCooldownMeter(float LowestX, float LowestY, float HighestWidth, float HighestHeight)
{
    if (game->MainPlayer != nullptr && game->MainPlayer->MainWeaponsSystem.CurrentWeapon != nullptr) {
        Rectangle r = {LowestX + HighestWidth + Margin*2, LowestY - Margin, 65, HighestHeight + Margin*2};
        DrawRectangle(r.x,r.y,r.width,r.height, ColorAlpha(BLACK, Alpha));
        float percent = min(game->MainPlayer->MainWeaponsSystem.AttackCooldowns[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex]/game->MainPlayer->MainWeaponsSystem.CurrentWeapon->Cooldown, 1.0f);
        Color g = WHITE;
        if (percent == 1.0f)
            g = GREEN;
        DrawRectangle(r.x + 10, r.y + 10 + ((r.height-20) * (1-percent)), r.width-20, (r.height-20) * percent, g);

        //ammo meter
        if (game->MainPlayer->MainWeaponsSystem.CurrentWeapon->Ammo > 0)
        {
            DrawRectangle(r.x + r.width + Margin, WeaponUITexture.texture.height - (100+Margin), 125, 100, ColorAlpha(BLACK, Alpha));
            float s2 = 70;
            float s = MeasureText(to_string(game->MainPlayer->MainWeaponsSystem.WeaponAmmo[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex]).c_str(), s2);
            DrawText(to_string(game->MainPlayer->MainWeaponsSystem.WeaponAmmo[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex]).c_str(), (int)(r.x + r.width + Margin + 125/2 - s/2),
            WeaponUITexture.texture.height - (100+Margin) + 10, s2, WHITE);

            float s3 = 20;
            float s4 = MeasureText("Ammo", s3);
            DrawText("Ammo", (int)(r.x + r.width + Margin + 125/2 - s4/2),
            WeaponUITexture.texture.height - (100+Margin) + 100 - s3 - 10, s3, WHITE
                );
        }
    }
}

void UIManager::DisplayHealthMeter()
{
    HealthBarAnimRot = Lerp(HealthBarAnimRot, 0, 3.5f * GetFrameTime());
    float limit = 25;
    FntSize = Lerp(FntSize, 92, 3.5f*GetFrameTime());
    float PlrHealth = game->MainPlayer->Health;
    if (LastHealth != PlrHealth)
    {
        float mov = 0.15f * (abs(PlrHealth-LastHealth)/8.0f);

        HealthBarAnimRot = max(min(limit * ((PlrHealth-LastHealth) / abs(PlrHealth-LastHealth)) * (mov / 0.1f), 35.0f), -35.0f);
        game->GameCamera.QuickZoom(PlrHealth - LastHealth > 0 ? 1.0f - min(0.15f, mov) : 1.0f + min(0.15f, mov), 0.1f);
    }
    if (LastHealth != PlrHealth && PlrHealth > LastHealth) {
        FntSize = 135;
    }
    float size = MeasureText((std::to_string((int)round(PlrHealth))+"%").c_str(), FntSize);
    DrawRectangle((WeaponUITexture.texture.width / 2.0f - size / 2.0f)-Margin,(WeaponUITexture.texture.height / 2.0f - FntSize/2)-Margin,size+(Margin*2),FntSize+(Margin*2),
        ColorLerp(ColorLerp(ColorAlpha((game->MainPlayer->isInvincible ? WHITE : BLACK), Alpha), RED, max(-HealthBarAnimRot,0.01f) / limit),GREEN,
            (FntSize - 92)/43));
    DrawTextPro(GetFontDefault(), (std::to_string((int)round(PlrHealth))+"%").c_str(),
        {WeaponUITexture.texture.width / 2.0f,
        WeaponUITexture.texture.height / 2.0f},
        {size/2, FntSize/2},
        HealthBarAnimRot,
        FntSize,
        10,
        GetHealthColor(game->MainPlayer->Health/game->MainPlayer->MaxHealth));
    LastHealth = PlrHealth;
}

void UIManager::DisplayCursor()
{
    if (!game->MainPlayer->IsPreparingForDash)
        {
            DrawTexturePro(game->GameShared->UIAssets.CursorImg, {0, 0, 36, 36}, {(float)GetMouseX(), (float)GetMouseY(), 27, 27}, {13.5f, 13.5f}, CursorRotation, YELLOW);
            if (game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex != -1)
            {
                if (game->MainPlayer->MainWeaponsSystem.AttackCooldowns[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex] >= game->MainPlayer->MainWeaponsSystem.CurrentWeapon->Cooldown)
                    CursorMiddleTrans = Lerp(CursorMiddleTrans, 1.0f, 10.5f * GetFrameTime());
                else
                    CursorMiddleTrans = Lerp(CursorMiddleTrans, 0.0f, 21.5f * GetFrameTime());
                if (game->MainPlayer->MainWeaponsSystem.CurrentWeapon->Ammo > 0)
                {
                    float goal = 360.0f * ((float)game->MainPlayer->MainWeaponsSystem.WeaponAmmo[game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex] / (float)game->MainPlayer->MainWeaponsSystem.CurrentWeapon->Ammo);
                    CursorRotation = Lerp(CursorRotation, goal, 2.5f * GetFrameTime());
                } else
                    CursorRotation = Lerp(CursorRotation, 0.0f, 2.5f * GetFrameTime());
            } else
            {
                CursorMiddleTrans = Lerp(CursorMiddleTrans, 0.0f, 10.5f * GetFrameTime());
                CursorRotation = Lerp(CursorRotation, 0.0f, 2.5f * GetFrameTime());
            }
            DrawTexturePro(game->GameResources.Textures["enemy"], {0, 0, 36, 36}, {(float)GetMouseX(), (float)GetMouseY(), 7.5f, 7.5f},
                    {3.75f, 3.75f}, CursorRotation, ColorAlpha(YELLOW, CursorMiddleTrans));
        } else {
            Vector2 Target = GetScreenToWorld2D(GetMousePosition(), game->GameCamera.RaylibCamera);
            float cx = game->MainPlayer->BoundingBox.x + game->MainPlayer->BoundingBox.width / 2;
            float cy = game->MainPlayer->BoundingBox.y + game->MainPlayer->BoundingBox.height / 2;
            float FinalAngle = atan2(cy - Target.y, cx - Target.x) * RAD2DEG - 90;
            DrawTexturePro(game->GameResources.Textures["arrow"], {0, 0, 80, 80}, {(float)GetMouseX(), (float)GetMouseY(), 27, 27}, {13.5f, 13.5f}, FinalAngle, ORANGE);
            CursorMiddleTrans = Lerp(CursorMiddleTrans, 0.0f, 10.5f * GetFrameTime());
            CursorRotation = Lerp(CursorRotation, 0.0f, 2.5f * GetFrameTime());
        }
}

void UIManager::DisplayTopHUD()
{
    std::string txt = to_string((int)game->GameScore);

    int font_size = 50;
    int width = MeasureText(txt.c_str(), font_size);

    float x = 50;
    float y = 50 + font_size;

    Rectangle rec = {x - 10, y - 10, width + 20.0f, font_size + 20.0f};
    DrawRectangleRec({rec.x - 10, rec.y - 10, rec.width + 20, rec.height + 20}, ColorAlpha(BLACK, UITransparency * 0.85f));
    DrawText(txt.c_str(), x, y, font_size, ColorAlpha(WHITE, UITransparency));
    DrawRectangleLinesEx(rec, 5, ColorAlpha(WHITE, UITransparency));

    std::string o_txt = "SCORE";

    DrawText(o_txt.c_str(), rec.x + rec.width/2 - MeasureText(o_txt.c_str(), font_size / 1.5f)/2, rec.y - (font_size/1.5f) - 15, font_size / 1.5f, ColorAlpha(WHITE, UITransparency));

    if (game->GameShared->DevMode)
    {
        x += width + 120;

        txt = to_string((int)(game->MainPlayer->StressLevel * 100.0f)) + "%";

        width = MeasureText(txt.c_str(), font_size);

        rec = {x - 10, y - 10, width + 20.0f, font_size + 20.0f};
        DrawRectangleRec({rec.x - 10, rec.y - 10, rec.width + 20, rec.height + 20}, ColorAlpha(BLACK, UITransparency * 0.85f));
        DrawText(txt.c_str(), x + StressShakePos.x, y + StressShakePos.y, font_size, ColorAlpha(GetHealthColor(1.0f - game->MainPlayer->StressLevel), UITransparency));
        DrawRectangleLinesEx(rec, 5, ColorAlpha(WHITE, UITransparency));

        o_txt = "STRESS";

        DrawText(o_txt.c_str(), rec.x + rec.width/2 - MeasureText(o_txt.c_str(), font_size / 1.5f)/2, rec.y - (font_size/1.5f) - 15, font_size / 1.5f, ColorAlpha(WHITE, UITransparency));
    }

    if (game->GameMode.LevelTimer > 0)
    {
        int minutes = (int)(this
            ->game->GameMode.LevelTimer / 60);
        int seconds = (int)(this->game->GameMode.LevelTimer - (minutes * 60.0f));
        std::string txt = to_string(minutes) + ":" + to_string(seconds);
        if (seconds < 10)
            txt = to_string(minutes) + ":0" + to_string(seconds);

        int font_size = 50;
        int width = MeasureText(txt.c_str(), font_size);

        float x = GetRenderWidth() / 2 - width / 2;
        float y = 50 + font_size;

        Rectangle rec = {x - 10, y - 10, width + 20.0f, font_size + 20.0f};
        DrawRectangleRec({rec.x - 10, rec.y - 10, rec.width + 20, rec.height + 20}, ColorAlpha(BLACK, (UITransparency * 0.75f)+0.25f));
        DrawText(txt.c_str(), x, y, font_size, ColorAlpha(WHITE, UITransparency));
        DrawRectangleLinesEx(rec, 5, ColorAlpha(WHITE, UITransparency));

        std::string o_txt = "TIME LEFT";
        if (game->GameMode.CurrentGameMode == "wave")
        {
            if (!game->GameMode.InWave)
                o_txt = "INTERMISSION " + to_string(game->GameMode.CurrentWave + 1);
            else
                o_txt = "WAVE " + to_string(game->GameMode.CurrentWave);
        }

        DrawText(o_txt.c_str(), rec.x + rec.width/2 - MeasureText(o_txt.c_str(), font_size / 1.5f)/2, rec.y - (font_size/1.5f) - 15, font_size / 1.5f, ColorAlpha(WHITE, UITransparency));
    }
}

void UIManager::DeathMenu()
{
    BeginTextureMode(DeathScreen);

    DrawTextPro(GetFontDefault(), "You died!", {GetRenderWidth()/2.0f, 250.0f}, {MeasureTextEx(GetFontDefault(), "You died!", 100, 10.0f).x/2.0f, 50.0f}, DeathTextAnimRot, 100, 10, ColorBrightness(RED, -0.3f));

    int ey = GetRenderHeight()-400;
    int es = 50;
    std::string txt = "YOU KILLED " + to_string(game->MainPlayer->Kills) + " ENEMIES";
    if (game->MainPlayer->Kills == 1)
        txt = "YOU KILLED 1 ENEMY";
    if (game->MainPlayer->Kills == 0)
        txt = "YOU DID NOT KILL ANY ENEMIES";

    std::string txt_2 = "PRESS E TO RESPAWN";
    float size = MeasureText(txt.c_str(), es);
    float size2 = MeasureText(txt_2.c_str(), es);

    std::string txt_3 = "FINAL SCORE: " + to_string(game->GameScore);

    DrawText(txt.c_str(), GetRenderWidth()/2 - size/2, ey-DeathTextAnimRot, es, ColorBrightness(RED, 0.4f));
    DrawText(txt_2.c_str(), GetRenderWidth()/2 - size2/2, ey+es*2-DeathTextAnimRot, es, ColorBrightness(RED, 0.4f));
    DrawText(txt_3.c_str(), GetRenderWidth()/2 - size2/2, ey+es-DeathTextAnimRot, es, ColorBrightness(RED, 0.4f));

    EndTextureMode();
}

void UIManager::PauseMenu() {
    BeginTextureMode(PauseScreen);
    ClearBackground(game->GameControls->IsControlDown("debug2") ? BLANK : ColorAlpha(BLACK, 0.35f));
    EndTextureMode();
    DrawTextureRec(PauseScreen.texture, Rectangle{0, 0, (float)PauseScreen.texture.width, -(float)PauseScreen.texture.height}, Vector2{0, (float)GetRenderHeight() - PauseScreen.texture.height}, WHITE);
    DrawRectangle(PauseScreen.texture.width/2 - 225, PauseScreen.texture.height/2-175,450, 350,ColorAlpha(BLACK,0.5f));
    game->Paused = !Button({(float)PauseScreen.texture.width/2 - (float)game->GameShared->UIAssets.ButtonImg.width/2,
        (float)PauseScreen.texture.height/2-100 - (float)game->GameShared->UIAssets.ButtonImg.height/2,
        (float)game->GameShared->UIAssets.ButtonImg.width, (float)game->GameShared->UIAssets.ButtonImg.height},
        GetMousePosition(), game->GameShared->UIAssets.ButtonImg, game->GameShared->UIAssets.ButtonClick, "RESUME");

    if (Button({(float)PauseScreen.texture.width/2 - (float)game->GameShared->UIAssets.ButtonImg.width/2,
        (float)PauseScreen.texture.height/2+100 - (float)game->GameShared->UIAssets.ButtonImg.height/2,
        (float)game->GameShared->UIAssets.ButtonImg.width, (float)game->GameShared->UIAssets.ButtonImg.height
    }, GetMousePosition(), game->GameShared->UIAssets.ButtonImg, game->GameShared->UIAssets.ButtonClick, "QUIT"))
        game->isReturning = true;
}

void UIManager::GameWin()
{
    BeginTextureMode(GameWinScreen);
    ClearBackground(ColorAlpha(GREEN, 0.2f));

    DrawTextPro(GetFontDefault(), "You won!", {GetRenderWidth()/2.0f, 250.0f},
        {MeasureTextEx(GetFontDefault(), "You won!", 150, 10.0f).x/2.0f, 50.0f}, DeathTextAnimRot, 150, 10,
        ColorBrightness(WHITE, -0.3f));

    int ey = GetRenderHeight()-400;
    int es = 50;
    std::string txt = "YOU KILLED " + to_string(game->MainPlayer->Kills) + " ENEMIES";
    if (game->MainPlayer->Kills == 1)
        txt = "YOU KILLED 1 ENEMY";
    if (game->MainPlayer->Kills == 0)
        txt = "YOU DID NOT KILL ANY ENEMIES";

    std::string txt_2 = "PRESS E TO GO TO MAIN MENU";
    float size = MeasureText(txt.c_str(), es);
    float size2 = MeasureText(txt_2.c_str(), es);

    std::string txt_3 = "FINAL SCORE: " + to_string(game->GameScore);

    DrawText(txt.c_str(), GetRenderWidth()/2 - size/2, ey-DeathTextAnimRot, es, ColorBrightness(WHITE, -0.1f));
    DrawText(txt_2.c_str(), GetRenderWidth()/2 - size2/2, ey+es*2-DeathTextAnimRot, es, ColorBrightness(WHITE, -0.1f));
    DrawText(txt_3.c_str(), GetRenderWidth()/2 - size2/2, ey+es-DeathTextAnimRot, es, ColorBrightness(WHITE, -0.1f));

    EndTextureMode();
    DrawTextureRec(GameWinScreen.texture, Rectangle{0, 0, (float)GameWinScreen.texture.width, -(float)GameWinScreen.texture.height}, Vector2{0, (float)GetRenderHeight() - GameWinScreen.texture.height}, ColorAlpha(WHITE, ((1-UITransparency)-0.5f)/0.5f));
}

void UIManager::DisplayRank()
{
    float RankLevel = max(min(game->MainPlayer->LogicProcessor.RankLevel, 1.0f), 0.0f);

    std::string RankClassification;

    float ClassSize = 1.0f / static_cast<float>(game->MainPlayer->LogicProcessor.RankClassifications.size());

    float MaxRankClassificationLvl = ceil(RankLevel / ClassSize) * ClassSize;
    float MinRankClassificationLvl = floor(RankLevel / ClassSize) * ClassSize;

    MaxRankClassificationLvl = max(min(MaxRankClassificationLvl, 1.0f), 0.0f);
    MinRankClassificationLvl = max(min(MinRankClassificationLvl, 1.0f), 0.0f);

    if (RankLevel >= MinRankClassificationLvl && RankLevel <= MaxRankClassificationLvl)
        RankClassification = game->MainPlayer->LogicProcessor.RankClassifications[static_cast<int>(max(min((RankLevel * game->MainPlayer->LogicProcessor.RankClassifications.size()),
            static_cast<float>(game->MainPlayer->LogicProcessor.RankClassifications.size()) - 1.0f), 0.0f))];

    float HighestTxSize = 0.0f;
    for (int i = 0; i < game->MainPlayer->ScoreChanges.size(); i++)
    {
        ScoreChange& ScoreChange = game->MainPlayer->ScoreChanges[i];
        float sz = MeasureText(ScoreChange.Reason.c_str(), 70)+15.0f;
        if (sz > HighestTxSize)
            HighestTxSize = sz;
    }

    float SizeGoal = static_cast<float>(MeasureText(RankClassification.c_str(), 70)) + 100.0f;
    float HeightGoal = 130 + game->MainPlayer->ScoreChanges.size() * 25.0f;

    SizeGoal = min(max(SizeGoal, max(HighestTxSize,100.0f)), 500.0f);
    HeightGoal = min(max(HeightGoal, 250.0f), 500.0f);

    RankSideSize = Lerp(RankSideSize, SizeGoal, 10.0f * game->GetGameDeltaTime());
    RankSizeHeight = Lerp(RankSizeHeight, HeightGoal, 10.0f * game->GetGameDeltaTime());

    Rectangle SideRectangle = {0, 0, RankSideSize, RankSizeHeight};
    SideRectangle.x = 50.0f;
    SideRectangle.y = GetRenderHeight() / 2 - SideRectangle.height / 2;

    DrawRectangleRec(SideRectangle, ColorAlpha(BLACK, 0.125f * UITransparency));

    Color OtherColor = Color{
        (unsigned char) (127.0f + sin(game->GetGameTime()) * 127.0f),
        (unsigned char) (127.0f + cos(game->GetGameTime()) * 127.0f),
        (unsigned char) (127.0f + sin(game->GetGameTime() + 10.0f) * 127.0f),
        255
    };
    Color RainbowColor = ColorAlpha(ColorLerp(WHITE, OtherColor, RankLevel * 1.5f), UITransparency);

    if (!RankClassification.empty())
        DrawText(RankClassification.c_str(), SideRectangle.x + 15 - StressShakePos.x * 2.5f, SideRectangle.y + 15 - StressShakePos.y * 2.5f, 70, RainbowColor);

    std::string PercentTxt = to_string((int) (RankLevel * 100.0f)) + "%";
    Vector2 EndPos ={SideRectangle.x + 15 + (SideRectangle.width - 15 - 15 - MeasureText(PercentTxt.c_str(), 30)) * RankLevel, SideRectangle.y + 15 + 15 + 70};
    DrawText(PercentTxt.c_str(), EndPos.x, EndPos.y-15.0f, 30, RainbowColor);
    DrawLineEx({
        SideRectangle.x + 15, SideRectangle.y + 15 + 15 + 70
    }, EndPos, 10, RainbowColor);

    for (int i = 0; i < static_cast<int>(min(static_cast<float>(game->MainPlayer->ScoreChanges.size()), 14.0f)); i++)
    {
        ScoreChange& ScoreChange = game->MainPlayer->ScoreChanges[i];
        float Trans = 1.0f - (game->GetGameTime() - ScoreChange.Time) / 10.0f;
        Trans = min(max(Trans, 0.25f), 1.0f);

        if (Trans > 0.0f)
            DrawText((ScoreChange.Reason + " (+" + to_string((int)round(ScoreChange.Points)) + ")").c_str(), SideRectangle.x + 15, EndPos.y + 15 + (i * 25), 25, ColorAlpha(ColorContrast(ScoreChange.ScoreColor, 0.5f), Trans * UITransparency));
    }
}

void UIManager::Quit() {
    UnloadRenderTexture(WeaponUITexture);
    UnloadRenderTexture(PauseScreen);
    UnloadRenderTexture(GameWinScreen);
    UnloadRenderTexture(DeathScreen);
}
