//
// Created by lalit on 6/11/2026.
//

#include "GameplayUI.h"
#include <algorithm>
#include <raymath.h>
#include "../../Game.h"

void GameplayUI::DisplayPowerupMeter()
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

void GameplayUI::DisplayKillMeter()
{
    DrawRectangle(GetRenderWidth()-(125+Margin), WeaponUITexture.texture.height - (100+Margin), 125, 100, ColorAlpha(BLACK, Alpha));
    float s2 = 70;

    int KillsNumber = game->MainPlayer->Kills - (game->MainPlayer->EnemyCombo > 1 ? game->MainPlayer->EnemyCombo-1 : 0);
    if (KillsNumber > 99)
        KillsNumber = 99;

    float TX_SZE_1 = MeasureText(to_string(KillsNumber).c_str(), s2);
    float TX_SZE_2 = game->MainPlayer->EnemyCombo > 1 ? MeasureText(("+"+to_string(game->MainPlayer->EnemyCombo-1)).c_str(), s2 / 2.5f) : 0;

    float s = TX_SZE_1 + TX_SZE_2 + 5;

    float CS = (game->GetGameTime() - game->MainPlayer->LastKilledAnEnemy) / game->MainPlayer->ComboTime;
    CS = min(max(CS, 0.0f), 1.0f);

    DrawText(to_string(KillsNumber).c_str(), (int)(GetRenderWidth()-(125+Margin) + 125/2 - s/2),
    WeaponUITexture.texture.height - (100+Margin) + 10, s2, WHITE);
    if (game->MainPlayer->EnemyCombo > 1)
        DrawText(("+"+to_string(game->MainPlayer->EnemyCombo-1)).c_str(), (int)(GetRenderWidth()-(125+Margin) + 125/2 - s/2 + TX_SZE_1 + 5), WeaponUITexture.texture.height - (100+Margin) + 10 + s2 - (s2/2.5f),
        s2 / 2.5f, ColorAlpha(GetHealthColor(1.0f-CS), 1.0f - (CS / 2.0f)));

    float s3 = 20;
    float s4 = MeasureText("Kills", s3);
    DrawText("Kills", (int)(GetRenderWidth()-(125+Margin) + 125/2 - s4/2),
    WeaponUITexture.texture.height - (100+Margin) + 100 - s3 - 10, s3, WHITE
        );
}

void GameplayUI::DisplayCooldownMeter(float LowestX, float LowestY, float HighestWidth, float HighestHeight)
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

void GameplayUI::DisplayHealthMeter()
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