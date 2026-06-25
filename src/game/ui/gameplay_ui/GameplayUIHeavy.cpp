//
// Created by Rolpon on 6/11/2026.
//

#include <algorithm>

#include "GameplayUI.h"
#include <raymath.h>
#include "../../Game.h"
#include "../../../entities/subentities/player/Player.h"
#include "../../core/Controls.h"
#include "../../managers/ResourceManager.h"
#include "../../managers/GameModeManager.h"
#include "../../managers/CameraManager.h"
#include "../../core/SharedManager.h"

void GameplayUI::DisplayLevelTimer()
{
    if (game->GameMode->LevelTimer > 0)
    {
        int minutes = (int)(this
            ->game->GameMode->LevelTimer / 60);
        int seconds = (int)(this->game->GameMode->LevelTimer - (minutes * 60.0f));
        std::string txt = to_string(minutes) + ":" + to_string(seconds);
        if (seconds < 10)
            txt = to_string(minutes) + ":0" + to_string(seconds);

        int font_size = 50;
        int width = MeasureText(txt.c_str(), font_size);

        float x = GetRenderWidth() / 2 - width / 2;
        float y = 50 + font_size;

        Rectangle rec = {x - 10, y - 10, width + 20.0f, font_size + 20.0f};
        DrawRectangleRec({rec.x - 10, rec.y - 10, rec.width + 20, rec.height + 20}, ColorAlpha(BLACK, 0.5f * UITransparency));
        DrawText(txt.c_str(), x, y, font_size, ColorAlpha(WHITE, UITransparency));
        DrawRectangleLinesEx(rec, 5, ColorAlpha(WHITE, UITransparency));

        std::string o_txt = "TIME LEFT";
        if (game->GameMode->CurrentGameMode == "wave")
        {
            if (!game->GameMode->InWave)
                o_txt = "INTERMISSION " + to_string(game->GameMode->CurrentWave + 1);
            else
                o_txt = "WAVE " + to_string(game->GameMode->CurrentWave);
        }

        DrawText(o_txt.c_str(), rec.x + rec.width/2 - MeasureText(o_txt.c_str(), font_size / 1.5f)/2, rec.y - (font_size/1.5f) - 15, font_size / 1.5f, ColorAlpha(WHITE, UITransparency));
    }
}

void GameplayUI::DisplayInventory(bool Found, float Prev, float HighestWidth, float* HighestHeight)
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

void GameplayUI::DisplayInventoryIndicator()
{
    float InvIndTrans = ((game->GetGameTime() - game->MainPlayer->LastSwappedItem)-0.8f) / 0.35f;
    InvIndTrans = max(min(InvIndTrans, 1.0f), 0.0f);
    InvIndTrans= 1.0f-InvIndTrans;

    float Margin = 7.5f;
    float W = (Margin * 4.0f) + (32.0f * 3);
    float H = 32.0f + (Margin * 2.0f);
    Rectangle rectangle = {GetRenderWidth()/2.0f - W/2.0f, GetRenderHeight() - WeaponUITexture.texture.height - H - 20,
        W, H};
    DrawRectangleRec(rectangle, ColorAlpha(BLACK, 0.5f*InvIndTrans*UITransparency));
    for (int i = 0; i < 3; i++)
    {
        std::string WepName = game->MainPlayer->MainWeaponsSystem.Weapons[i];
        if (WepName.empty() || !game->GameResources->Weapons.contains(WepName) || game->GameResources->Weapons[WepName].Icon.empty() || !game->GameResources->Textures.contains(game->GameResources->Weapons[WepName].Icon))
            continue;
        Rectangle thisTileRect = {
            rectangle.x + (i * 32.0f) + ((i+1) * Margin),
            rectangle.y + Margin,
            32.0f,
            32.0f
        };
        if (i == game->MainPlayer->MainWeaponsSystem.CurrentWeaponIndex)
            DrawRectangleRec(thisTileRect, ColorAlpha(WHITE, 0.5f*InvIndTrans*UITransparency));
        DrawTexturePro(game->GameResources->Textures[game->GameResources->Weapons[WepName].Icon], {0, 0, 32.0f, 32.0f}, thisTileRect, {0.0f,0.0f}, 0.0f, ColorAlpha(WHITE, InvIndTrans * UITransparency));
    }
}

void GameplayUI::DisplayRank()
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
    int len = static_cast<int>(min(static_cast<float>(game->MainPlayer->ScoreChanges.size()), 7.0f));
    for (int i = 0; i < len; i++)
    {
        int rI = game->MainPlayer->ScoreChanges.size() - i - 1;
        ScoreChange& ScoreChange = game->MainPlayer->ScoreChanges[rI];
        float sz = MeasureText((ScoreChange.Reason + " (+" + to_string((int)round(ScoreChange.Points)) + ")").c_str(), 25)+30.0f;
        if (sz > HighestTxSize)
            HighestTxSize = sz;
    }
    float TotalScoreSZ = MeasureText(("TOTAL SCORE: " + to_string((int)game->GameScore)).c_str(), 25) + 30.0f;
    if (TotalScoreSZ > HighestTxSize)
        HighestTxSize = TotalScoreSZ;

    float SizeGoal = static_cast<float>(MeasureText(RankClassification.c_str(), 70)) + 100.0f;
    float HeightGoal = 130 + ((len + 1) * 25.0f);

    SizeGoal = min(max(SizeGoal, max(HighestTxSize,100.0f)), 700.0f);
    HeightGoal = min(max(HeightGoal, 250.0f), 500.0f);

    RankSideSize = Lerp(RankSideSize, SizeGoal, 10.0f * game->GetGameDeltaTime());
    RankSizeHeight = Lerp(RankSizeHeight, HeightGoal, 10.0f * game->GetGameDeltaTime());

    Rectangle SideRectangle = {0, 0, RankSideSize, RankSizeHeight};
    SideRectangle.x = 50.0f;
    SideRectangle.y = GetRenderHeight() / 2 - SideRectangle.height / 2;

    DrawRectangleRec(SideRectangle, ColorAlpha(BLACK, 0.25f * UITransparency));

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

    for (int i = 0; i < len; i++)
    {
        int rI = game->MainPlayer->ScoreChanges.size() - i - 1;
        ScoreChange& ScoreChange = game->MainPlayer->ScoreChanges[rI];
        float Trans = 1.0f - (game->GetGameTime() - ScoreChange.Time) / 10.0f;
        Trans = min(max(Trans, 0.25f), 1.0f);

        if (Trans > 0.0f)
            DrawText((ScoreChange.Reason + " (+" + to_string((int)round(ScoreChange.Points)) + ")").c_str(), SideRectangle.x + 15, EndPos.y + 15 + (i * 25), 25, ColorAlpha(ColorContrast(ScoreChange.ScoreColor, 0.5f), Trans * UITransparency));
    }
    DrawText(("TOTAL SCORE: " + to_string((int)game->GameScore)).c_str(), SideRectangle.x + 15, max(EndPos.y + 15 + (len * 25), SideRectangle.y + SideRectangle.height - 40),25, ColorAlpha(WHITE, UITransparency));
}

void GameplayUI::DisplayCursor()
{
    if (!game->MainPlayer->IsPreparingForDash)
        {
            DrawTexturePro(game->GameShared->SharedUIAssets.CursorImg, {0, 0, 36, 36},
                {(float)GetMouseX(), (float)GetMouseY(), 27, 27}, {13.5f, 13.5f}, CursorRotation, YELLOW);
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
            DrawTexturePro(game->GameResources->Textures["enemy"], {0, 0, 36, 36}, {(float)GetMouseX(), (float)GetMouseY(), 7.5f, 7.5f},
                    {3.75f, 3.75f}, CursorRotation, ColorAlpha(YELLOW, CursorMiddleTrans));
        } else {
            Vector2 Target = GetScreenToWorld2D(GetMousePosition(), game->GameCamera->RaylibCamera);
            float cx = game->MainPlayer->BoundingBox.x + game->MainPlayer->BoundingBox.width / 2;
            float cy = game->MainPlayer->BoundingBox.y + game->MainPlayer->BoundingBox.height / 2;
            float FinalAngle = atan2(cy - Target.y, cx - Target.x) * RAD2DEG - 90;
            DrawTexturePro(game->GameResources->Textures["arrow"], {0, 0, 80, 80}, {(float)GetMouseX(), (float)GetMouseY(), 27, 27}, {13.5f, 13.5f}, FinalAngle, ORANGE);
            CursorMiddleTrans = Lerp(CursorMiddleTrans, 0.0f, 10.5f * GetFrameTime());
            CursorRotation = Lerp(CursorRotation, 0.0f, 2.5f * GetFrameTime());
        }
}