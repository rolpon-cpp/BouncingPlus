//
// Created by Rolpon on 6/11/2026.
//

#include "GameplayUI.h"
#include "../../../entities/subentities/player/Player.h"
#include "../../managers/CameraManager.h"
#include "../../ui/UIUtils.h"
#include <raymath.h>
#include "../../Game.h"

void GameplayUI::DisplayPowerupMeter()
{
    float s3 = 0;
    float s4 = 0;
    if (game->MainPlayer->MainPowerupSystem.CurrentPowerup != nullptr)
    {
        DrawRectangle(GetRenderWidth() - (125 + Margin) * 2, GetRenderHeight() - (100 + Margin), 125, 100,
                      ColorAlpha(BLACK, PanelAlpha));

        s3 = 20;
        s4 = MeasureText("Powerup", s3);
        DrawText("Powerup", (int)(GetRenderWidth() - (125 + Margin) * 2 + 125 / 2 - s4 / 2),
                 GetRenderHeight() - (100 + Margin) + 100 - s3 - 10, s3, WHITE
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
        DrawText(game->MainPlayer->MainPowerupSystem.CurrentPowerup->Name.c_str(),
                 (int)(GetRenderWidth() - (125 + Margin) * 2 + 125 / 2 - s4 / 2),
                 GetRenderHeight() - (100 + Margin) + 10, s3, WHITE
        );
        float s2 = 50;
        float s = MeasureText((to_string((int)Number) + "s").c_str(), s2);
        DrawText((to_string((int)Number) + "s").c_str(), (int)(GetRenderWidth() - (125 + Margin) * 2 + 125 / 2 - s / 2),
                 GetRenderHeight() - (100 + Margin) + 50 - s2 / 2, s2, MainColor);
    }
}

void GameplayUI::DisplayKillMeter()
{
    DrawRectangle(GetRenderWidth() - (125 + Margin), GetRenderHeight() - (100 + Margin), 125, 100,
                  ColorAlpha(BLACK, PanelAlpha));
    float s2 = 70;

    int KillsNumber = game->MainPlayer->Kills - (
        game->MainPlayer->EnemyCombo > 1 ? game->MainPlayer->EnemyCombo - 1 : 0);
    if (KillsNumber > 99)
        KillsNumber = 99;

    float TX_SZE_1 = MeasureText(std::to_string(KillsNumber).c_str(), s2);
    float TX_SZE_2 = game->MainPlayer->EnemyCombo > 1
                         ? MeasureText(("+" + std::to_string(game->MainPlayer->EnemyCombo - 1)).c_str(), s2 / 2.5f)
                         : 0;

    float s = TX_SZE_1 + TX_SZE_2 + 5;

    float CS = (game->GetGameTime() - game->MainPlayer->LastKilledAnEnemy) / game->MainPlayer->ComboTime;
    CS = min(max(CS, 0.0f), 1.0f);

    DrawText(std::to_string(KillsNumber).c_str(), (int)(GetRenderWidth() - (125 + Margin) + 125 / 2 - s / 2),
             GetRenderHeight() - (100 + Margin) + 10, s2, WHITE);
    if (game->MainPlayer->EnemyCombo > 1)
        DrawText(("+" + std::to_string(game->MainPlayer->EnemyCombo - 1)).c_str(),
                 (int)(GetRenderWidth() - (125 + Margin) + 125 / 2 - s / 2 + TX_SZE_1 + 5),
                 GetRenderHeight() - (100 + Margin) + 10 + s2 - (s2 / 2.5f),
                 s2 / 2.5f, ColorAlpha(GetHealthColor(1.0f - CS), 1.0f - (CS / 2.0f)));

    float s3 = 20;
    float s4 = MeasureText("Kills", s3);
    DrawText("Kills", (int)(GetRenderWidth() - (125 + Margin) + 125 / 2 - s4 / 2),
             GetRenderHeight() - (100 + Margin) + 100 - s3 - 10, s3, WHITE
    );
}

void GameplayUI::DisplayHealthMeter()
{
    HealthBarAnimRot = Lerp(HealthBarAnimRot, 0, 3.5f * GetFrameTime());
    float limit = 25;
    HealthBarFontSize = Lerp(HealthBarFontSize, 92, 3.5f * GetFrameTime());
    float PlrHealth = game->MainPlayer->Health;
    if (LastHealth != PlrHealth)
    {
        float mov = 0.15f * (abs(PlrHealth - LastHealth) / 8.0f);

        HealthBarAnimRot = max(min(limit * ((PlrHealth - LastHealth) / abs(PlrHealth - LastHealth)) * (mov / 0.1f),
                                   35.0f), -35.0f);
        game->GameCamera->QuickZoom(PlrHealth - LastHealth > 0 ? 1.0f - min(0.15f, mov) : 1.0f + min(0.15f, mov), 0.1f);
    }
    if (LastHealth != PlrHealth && PlrHealth > LastHealth)
    {
        HealthBarFontSize = 135;
    }
    float size = MeasureText((std::to_string((int)round(PlrHealth)) + "%").c_str(), HealthBarFontSize);
    DrawRectangle((GetRenderWidth() / 2.0f - size / 2.0f) - Margin,
                  (GetRenderHeight() *0.75f - HealthBarFontSize / 2) - Margin, size + (Margin * 2),
                  HealthBarFontSize + (Margin * 2),
                  ColorLerp(ColorLerp(ColorAlpha((game->MainPlayer->isInvincible ? WHITE : BLACK), PanelAlpha), RED,
                                      max(-HealthBarAnimRot, 0.01f) / limit), GREEN,
                            (HealthBarFontSize - 92) / 43));
    DrawTextPro(GetFontDefault(), (std::to_string((int)round(PlrHealth)) + "%").c_str(),
                {
                    GetRenderWidth() / 2.0f,
                    GetRenderHeight() * 0.75f
                },
                {size / 2, HealthBarFontSize / 2},
                HealthBarAnimRot,
                HealthBarFontSize,
                10,
                GetHealthColor(game->MainPlayer->Health / game->MainPlayer->MaxHealth));
    LastHealth = PlrHealth;
}