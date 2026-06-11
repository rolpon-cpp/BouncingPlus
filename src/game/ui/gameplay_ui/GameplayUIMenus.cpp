//
// Created by lalit on 6/11/2026.
//

#include "GameplayUI.h"
#include "../../Game.h"

void GameplayUI::DeathMenu()
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

void GameplayUI::PauseMenu() {
    BeginTextureMode(PauseScreen);
    ClearBackground(game->GameControls->IsControlDown("debug2") ? BLANK : ColorAlpha(BLACK, 0.65f));
    EndTextureMode();

    if (!WasPausedLast)
        IsChangingSettings = false;

    DrawTextureRec(PauseScreen.texture, Rectangle{0, 0, (float)PauseScreen.texture.width, -(float)PauseScreen.texture.height},
            Vector2{0, (float)GetRenderHeight() - PauseScreen.texture.height}, WHITE);

    if (!IsChangingSettings)
    {

        float TxtSize = MeasureText("BouncingBallz", 36.0f);
        Rectangle r = {0, 0, 43.5f + TxtSize, 41.0f};

        Rectangle BaseRect = {PauseScreen.texture.width/2.0f - 225, PauseScreen.texture.height/2.0f-175,450, 350};
        DrawRectangleRec(BaseRect,ColorAlpha(BLACK,0.5f));

        r.x = BaseRect.x + BaseRect.width/2.0f - r.width/2.0f;
        r.y = BaseRect.y - r.height;

        DrawRectangleRec(r, ColorAlpha(BLACK,0.5f));
        DrawTexture(game->GameResources.Textures["player"], r.x + 2.5f, r.y + 2.5f, PINK);
        DrawText("BouncingBallz", r.x + 41.0f, r.y + 2.5f, 36.0f, WHITE);

        game->Paused = !Button({(float)PauseScreen.texture.width/2 - (float)game->GameShared->SharedUIAssets.ButtonImg.width/2,
            (float)PauseScreen.texture.height/2-100 - (float)game->GameShared->SharedUIAssets.ButtonImg.height/2,
            (float)game->GameShared->SharedUIAssets.ButtonImg.width, (float)game->GameShared->SharedUIAssets.ButtonImg.height},
            GetMousePosition(), game->GameShared->SharedUIAssets.ButtonImg, game->GameShared->SharedUIAssets.ButtonClick, "RESUME");

        IsChangingSettings = Button(
            {(float)PauseScreen.texture.width/2 - (float)game->GameShared->SharedUIAssets.ButtonImg.width/2,
            (float)PauseScreen.texture.height/2 - (float)game->GameShared->SharedUIAssets.ButtonImg.height/2,
            (float)game->GameShared->SharedUIAssets.ButtonImg.width, (float)game->GameShared->SharedUIAssets.ButtonImg.height},
            GetMousePosition(), game->GameShared->SharedUIAssets.ButtonImg, game->GameShared->SharedUIAssets.ButtonClick, "SETTINGS");

        if (Button({(float)PauseScreen.texture.width/2 - (float)game->GameShared->SharedUIAssets.ButtonImg.width/2,
            (float)PauseScreen.texture.height/2+100 - (float)game->GameShared->SharedUIAssets.ButtonImg.height/2,
            (float)game->GameShared->SharedUIAssets.ButtonImg.width, (float)game->GameShared->SharedUIAssets.ButtonImg.height
        }, GetMousePosition(), game->GameShared->SharedUIAssets.ButtonImg, game->GameShared->SharedUIAssets.ButtonClick, "QUIT"))
            game->isReturning = true;

#ifndef PLATFORM_WEB
        if (Button({(float)PauseScreen.texture.width - 320.0f, 20.0f, 300.0f, 50.0f},GetMousePosition(),
            game->GameShared->SharedUIAssets.ButtonImg, game->GameShared->SharedUIAssets.ButtonClick, "RELOAD WEAPONS"))
            game->GameResources.LoadWeaponData();
#endif
    } else
    {
        game->GameShared->DisplaySettings({0.0f,0.0f}, 0.0f, 0.0f);
        IsChangingSettings = !Button({
                   (GetRenderWidth() / 2.0f) - 75.0f,
                   GetRenderHeight() - 106.0f, 150, 56
               }, GetMousePosition(),
               game->GameShared->SharedUIAssets.ButtonImg, game->GameShared->SharedUIAssets.ButtonClick, "BACK");
    }

    WasPausedLast = true;
}

void GameplayUI::GameWin()
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