//
// Created by lalit on 10/16/2025.
//

#include "Menu.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include "UIUtils.h"
#include <raymath.h>

using namespace std;

float Menu::MusicLevel = 0.0f;
void Menu::AudioCallback(void* buffer, unsigned int frames)
{
    float *samples = (float *)buffer;

    float sum = 0.0f;
    for (unsigned int i = 0; i < frames * 2; i++) {
        sum += samples[i] * samples[i];
    }

    MusicLevel = sqrtf(sum / (frames * 2));
}

Menu::Menu(SharedManager& GameSettings)
{
    MusicLevel = 0.0f;
    this->Shared = &GameSettings;
    Reset();
    PlaySound(this->Shared->UIAssets.MikuMusic);

    AttachAudioStreamProcessor(this->Shared->UIAssets.MainMenuMusic.stream, this->AudioCallback);
}

void Menu::Reset()
{
    Map = "";
    TargetMap = "";
    TitleImgY = -Shared->UIAssets.TitleImg.height;
    TitleImgOffsetY = 0;
    PlayButtonOffsetY = -100;
    SettingsButtonOffsetY = -200;
    CreditsButtonOffsetY = -300;
    CameraX = 0;
    CameraTargetX = 0;
    Offset1 = 0;
    Offset2 = 0;
    Offset3 = 0;
    isStarting = true;
    CurrentLevelsPage = 0;
    BlackTransparency= 1.0f;
    MusicLevel = 0.0f;
    MikuOffset = 0.0f;
    MenuImgOffsetY = GetRandomValue(0, Shared->UIAssets.MenuImg.height);
    MovingToGame = false;
    MousePos = {0,0};
    Shared->ResetSettings();
}

void Menu::LevelSelect()
{
    Vector2 LevelSelectPanelSize = Vector2{(float)GetRenderWidth() - 200.0f, (float)GetRenderHeight() - 180.0f};
    Rectangle LevelSelectPanelRectangle = {(float)-GetRenderWidth()+ (float)GetRenderWidth()/2.0f - LevelSelectPanelSize.x / 2.0f, 25+(Offset2/1.5f), LevelSelectPanelSize.x, LevelSelectPanelSize.y};

    DrawRectangleRec({LevelSelectPanelRectangle.x - CameraX, LevelSelectPanelRectangle.y, LevelSelectPanelRectangle.width, LevelSelectPanelRectangle.height}, ColorAlpha(BLACK, 0.5f));

    Rectangle play_bbox = {(GetRenderWidth()/2.0f) - GetRenderWidth() - (int)(Shared->UIAssets.ButtonImg.width/2.0f), GetRenderHeight() - 106 + Offset1,150,56};
    if (Button({play_bbox.x - CameraX, play_bbox.y, play_bbox.width, play_bbox.height}, GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "BACK")) {
        CameraTargetX=0;
    }
    DrawText("LEVEL SELECT", (int)(GetRenderWidth()/2.0f) - (int)(MeasureText("LEVEL SELECT", 55)/2.0f)-CameraX-GetRenderWidth(), LevelSelectPanelRectangle.y+25, 55, WHITE);

    DrawLineEx(Vector2{LevelSelectPanelRectangle.x + 25.0f - CameraX, LevelSelectPanelRectangle.y + 100}, Vector2{LevelSelectPanelRectangle.x + LevelSelectPanelSize.x - 25.0f - CameraX, LevelSelectPanelRectangle.y + 100}, 4, WHITE);

    float rh = 110.0f;

    float ow = LevelSelectPanelRectangle.width - 70 - (GetRenderWidth()-900);

    int fi = 0;
    int mi = 0;
    for (auto& [name, data] : Shared->LevelData)
    {
        fi = data["order"];
        if (fi >= (CurrentLevelsPage + 1) * 3 || fi < CurrentLevelsPage * 3)
            continue;
        int i = fi % 3;
        mi = max(i, mi);
        float rw = (float)MeasureText(name.c_str(), 90)+20.0f;

        Rectangle r = {LevelSelectPanelRectangle.x + 50+ (GetRenderWidth()-900) + ow/2 - rw/2, (float)165 + (140 * i) + LevelSelectPanelRectangle.y, rw, rh};
        DrawRectangleRec({r.x-CameraX,r.y,r.width,r.height}, ColorAlpha(BLACK, 0.5f));
        Color c = RED;
        if (TargetMap==name) {
            c = GREEN;
            DrawRectangle(LevelSelectPanelRectangle.x + 25 -CameraX, 200 + LevelSelectPanelRectangle.y, GetRenderWidth()-900, GetRenderHeight()-450, ColorAlpha(BLACK, 0.5f));
            DrawText(name.c_str(), -(GetRenderWidth()-145)-CameraX, 220 + LevelSelectPanelRectangle.y, 50, WHITE);

            std::string description = data["description"];

            if (!description.empty())
            {
                float alloc_size = GetRenderWidth() - 940;
                int lines = ceil(MeasureText(description.c_str(), 25) / alloc_size);

                int str_prog = 0;
                int last_str_prog = 0;

                for (int i = 0; i < lines; i++)
                {
                    last_str_prog = str_prog;
                    while (MeasureText(description.substr(last_str_prog, str_prog - last_str_prog).c_str(), 25) < alloc_size && str_prog < description.length())
                        str_prog++;

                    std::string txt = description.substr(last_str_prog, str_prog - last_str_prog);
                    if (txt.front() == ' ')
                        txt.erase(txt.begin());
                    //description.substr(max((float)(i * (description.size() / lines)), 0.0f), min((i+1) * (description.size() / lines), description.size())).c_str()

                    DrawText(txt.c_str(), -(GetRenderWidth()-145)-CameraX, 270 + (35 * i) + LevelSelectPanelRectangle.y, 25, WHITE);
                }
            }

            DrawText(("Difficulty level: "+to_string(data["difficulty"])).c_str(), -(GetRenderWidth()-145)-CameraX, 200 + GetRenderHeight()-450 - 70 - 64 + LevelSelectPanelRectangle.y, 25, ColorBrightness(RED, -1 + ((float)data["difficulty"] * 0.25f) ));
            Rectangle play_bbox = {(float)-(GetRenderWidth()-145), 200 + GetRenderHeight()-450 - 70 + LevelSelectPanelRectangle.y,150,56};
            if (Button({play_bbox.x - CameraX, play_bbox.y, play_bbox.width, play_bbox.height}, GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "PLAY")) {
                MovingToGame = true;
            }
        }

        DrawText(name.c_str(), r.x + 10 - CameraX, r.y + 10, 90, c);
        if (CheckCollisionPointRec(MousePos, r)) {
            DrawRectangleLinesEx({r.x-CameraX,r.y,r.width,r.height}, 4, WHITE);
            TargetMap = IsMouseButtonPressed(0) ? (TargetMap != name ? name : "") : TargetMap;
        }
    }

    float rw = 450.0f;
    Rectangle r = {LevelSelectPanelRectangle.x + 50+ (GetRenderWidth()-900) + ow/2 - rw/2, (float)165 + 140 * (mi+1) + LevelSelectPanelRectangle.y, rw, rh};
    DrawRectangleRec({r.x-CameraX,r.y,r.width,r.height}, ColorAlpha(BLACK, 0.5f));

    if (Button({r.x + r.width*0.85f - 25 - CameraX, r.y + r.height/2 - 25, 50, 50}, GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, ">"))
        CurrentLevelsPage += 1;

    if (Button({r.x + r.width * 0.15f - 25 - CameraX, r.y + r.height/2 - 25, 50, 50}, GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "<"))
        CurrentLevelsPage -= 1;

    if (CurrentLevelsPage < 0)
        CurrentLevelsPage = 0;
    if (CurrentLevelsPage >= Shared->LevelData.size() / 3)
        CurrentLevelsPage = Shared->LevelData.size() / 3;
    std::string pageTxt = "PAGE " + to_string(CurrentLevelsPage + 1);
    DrawText(pageTxt.c_str(), r.x + r.width / 2 - MeasureText(pageTxt.c_str(), 50)/2 - CameraX, r.y + r.height/2 - 25, 50, WHITE);
}

void Menu::Credits()
{
    Rectangle CreditsPanelRectangle = Panel({GetRenderWidth()*2 + GetRenderWidth()/2.0f - ((float)GetRenderWidth() - 200.0f)/2 - CameraX, 25, (float)GetRenderWidth() - 200.0f, (float)GetRenderHeight() - 180.0f}, "CREDITS", Offset1);

    Vector2 CreditsPanelSize = {CreditsPanelRectangle.width, CreditsPanelRectangle.height};

    DrawTexture(Shared->UIAssets.RolponPFPImg, CreditsPanelRectangle.x + 25 , CreditsPanelRectangle.y + 125.0f, WHITE);
    DrawTexture(Shared->UIAssets.CozPFPImg, CreditsPanelRectangle.x + 25 , CreditsPanelRectangle.y + 406.0f, WHITE);

    DrawTexture(Shared->UIAssets.InkyPFPImg, CreditsPanelRectangle.x + 25 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 125.0f, WHITE);
    DrawTexture(Shared->UIAssets.EggPFPImg, CreditsPanelRectangle.x + 25 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 406.0f, WHITE);

    float R = 127.0f + 127.0f * sin(GetTime());
    float G = 127.0f + 127.0f * cos(GetTime());
    float B = 127.0f + 127.0f * sin(GetTime() + 10);

    DrawText("Rolpon", CreditsPanelRectangle.x + 291 , CreditsPanelRectangle.y + 125.0f, 45.0f, Color{(unsigned char) R, (unsigned char) G, (unsigned char) B, 255});

    DrawText("Coz", CreditsPanelRectangle.x + 291 , CreditsPanelRectangle.y + 406.0f, 45.0f, PURPLE);

    DrawText("inkyrblx", CreditsPanelRectangle.x + 291 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 125.0f, 45.0f, BROWN);

    DrawText("eggjsoto", CreditsPanelRectangle.x + 291 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 406.0f, 45.0f, ORANGE);

    // rolpon
    DrawText("Owner & Programmer", CreditsPanelRectangle.x + 291 , CreditsPanelRectangle.y + 170.0f, 20, WHITE);

    // coz
    DrawText("Game Director & Playtester", CreditsPanelRectangle.x + 291 , CreditsPanelRectangle.y + 451.0f, 20, WHITE);

    // inky
    DrawText("Music & SFX Composer", CreditsPanelRectangle.x + 291 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 170.0f, 20, WHITE);

    // jay
    DrawText("Artist & Texture Maker", CreditsPanelRectangle.x + 291 + (CreditsPanelSize.x-50)/2 , CreditsPanelRectangle.y + 406.0f + 45, 20, WHITE);

    float ThankYouTextWidth = MeasureText("Finally, thank YOU, for playing!", 40);
    DrawText("Finally, thank YOU, for playing!", CreditsPanelRectangle.x + CreditsPanelSize.x/2 - ThankYouTextWidth/2 , CreditsPanelRectangle.y + CreditsPanelSize.y - 50, 40, WHITE);

    float FntSize = 24;
    float TxSize = MeasureText("Support me on YouTube!", FntSize);
    Rectangle r = {0, 0, TxSize + FntSize + 10, FntSize + 10};
    r.x = GetRenderWidth() - r.width;
    r.y = GetRenderHeight() - r.height;

    float TransparencyDiv = 2.0f;

    if (CheckCollisionPointRec(GetMousePosition(), r))
    {
        TransparencyDiv = 1.0f;
        if (IsMouseButtonPressed(0))
            OpenURL("https://www.youtube.com/@rolpon2871");
    }

    DrawRectangleRec(r, ColorAlpha(BLACK, 0.5f / TransparencyDiv));

    DrawText("Support me on YouTube!", r.x + 5, r.y + 5, FntSize, ColorAlpha(WHITE, 1.0f / TransparencyDiv));
    DrawTextureEx(Shared->UIAssets.YTImg, Vector2{r.x + 12 + TxSize, r.y + 5}, 0, FntSize / 512.0f, ColorAlpha(WHITE, 1.0f / TransparencyDiv));
}

void Menu::Update() {
    if (!MovingToGame) {
        CameraX = Lerp(CameraX, CameraTargetX * GetRenderWidth(), 2.0f * GetFrameTime());
        MousePos = {(float)GetMouseX() + CameraX, (float)GetMouseY()};
    }

    if (!IsMusicStreamPlaying(Shared->UIAssets.MainMenuMusic))
        PlayMusicStream(Shared->UIAssets.MainMenuMusic);
    UpdateMusicStream(Shared->UIAssets.MainMenuMusic);

    MenuMusicLevel = Lerp(MenuMusicLevel, MusicLevel, 8.5f * GetFrameTime());

    Offset1 = -sin((GetTime()-20.0f) * 3.5f) * 15;
    Offset2 = -sin((GetTime()+20.0f) * 3.5f) * 15;
    Offset3 = -sin(GetTime() * 3.5f) * 15;

    Offset1 += MenuMusicLevel * 80.5f;
    Offset2 += MenuMusicLevel * 60.5f;
    Offset3 += MenuMusicLevel * 40.5f;

    TitleImgY = Lerp(TitleImgY, -100, 5 * GetFrameTime());
    TitleImgOffsetY = Offset1;
    if (abs(TitleImgY + 100) <= 10)
        PlayButtonOffsetY = Lerp(PlayButtonOffsetY, 550, 5 * GetFrameTime());
    if (abs(PlayButtonOffsetY - 550) <= 45)
        SettingsButtonOffsetY = Lerp(SettingsButtonOffsetY, 624, 7 * GetFrameTime());
    if (abs(SettingsButtonOffsetY - 624) <= 45)
        CreditsButtonOffsetY = Lerp(CreditsButtonOffsetY, 698, 9 * GetFrameTime());

    MenuImgOffsetY += GetFrameTime()*15;
    float zoom = 1.45f;
    DrawTexturePro(Shared->UIAssets.MenuImg, {0, MenuImgOffsetY, 1184.0f, 736.0f},
        {(float)GetRenderWidth() / 2.0f - CameraX/10, (float)GetRenderHeight() / 2.0f, (float)GetRenderWidth() * zoom, (float)GetRenderHeight() * zoom},
        {(float)GetRenderWidth() * zoom / 2.0f, (float)GetRenderHeight() * zoom / 2.0f},
        0,
        WHITE
        );

    DrawTexture(Shared->UIAssets.TitleImg, (int)(GetRenderWidth()/2.0f) - (int)(Shared->UIAssets.TitleImg.width/2.0f)-CameraX, (int)TitleImgY - (int)TitleImgOffsetY, WHITE);

    Shared->DisplaySettings(Vector2{-CameraX + GetRenderWidth(), 0}, Offset1, Offset2);

    Rectangle play_bbox = {(GetRenderWidth()/2.0f) - (int)(Shared->UIAssets.ButtonImg.width/2.0f), (float)PlayButtonOffsetY +Offset3,150,56};
    if (Button({play_bbox.x - CameraX, play_bbox.y, play_bbox.width, play_bbox.height}, GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "PLAY")) {
        CameraTargetX = -1;
    }

    Credits();

    if (Button({GetRenderWidth()*2 + (GetRenderWidth() / 2.0f) - 75.0f - CameraX, GetRenderHeight() - 106.0f + (Offset1 + Offset3)/2, 150, 56}, GetMousePosition(),
        Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "BACK") ||
        Button({GetRenderWidth() + (GetRenderWidth() / 2.0f) - 75.0f - CameraX, GetRenderHeight() - 106.0f + (Offset2 + Offset3)/2, 150, 56}, GetMousePosition(),
            Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "BACK"))
        CameraTargetX=0;

    if (Button({(GetRenderWidth()/2.0f) - (int)(Shared->UIAssets.ButtonImg.width/2.0f) - CameraX, (float)SettingsButtonOffsetY + (Offset2 + Offset3)/2,150,56}
        ,GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "SETTINGS"))
        CameraTargetX=1;

    if (Button({(GetRenderWidth()/2.0f) - (int)(Shared->UIAssets.ButtonImg.width/2.0f) - CameraX, (float)CreditsButtonOffsetY +Offset2,150,56},
        GetMousePosition(), Shared->UIAssets.ButtonImg, Shared->UIAssets.ButtonClick, "CREDITS"))
        CameraTargetX=2;

    if (isStarting && BlackTransparency > 0)
        BlackTransparency -= 0.65f * GetFrameTime();
    else
    {
        isStarting = false;
    }
    if (MovingToGame)
        BlackTransparency += 0.65f * GetFrameTime();
    if (MovingToGame && BlackTransparency >= 1.0f) {
        Map = TargetMap;
        StopSound(Shared->UIAssets.MikuMusic);
        StopMusicStream(Shared->UIAssets.MainMenuMusic);
    }

    DrawTexture(Shared->UIAssets.MikuImg, -75, GetRenderHeight() - 20 + MikuOffset, WHITE);
    if (GetMouseX() < 250 && GetMouseY() > GetRenderHeight() - 70) {
        MikuOffset = Lerp(MikuOffset, -500, 10*GetFrameTime());
        if (!IsSoundPlaying(Shared->UIAssets.MikuMusic)) {
            SetSoundVolume(Shared->UIAssets.MikuMusic, 0.2f);
            ResumeSound(Shared->UIAssets.MikuMusic);
            SetMusicVolume(Shared->UIAssets.MainMenuMusic, 0);
        }
    } else {
        MikuOffset = Lerp(MikuOffset, 0, 10*GetFrameTime());
        PauseSound(Shared->UIAssets.MikuMusic);

        SetMusicVolume(Shared->UIAssets.MainMenuMusic, 1.0f - BlackTransparency);
    }

    LevelSelect();

    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), ColorAlpha(BLACK, BlackTransparency));
}

std::string Menu::LeaveMenu() {
    return Map;
}

void Menu::Quit() {
    Reset();
    DetachAudioStreamProcessor(Shared->UIAssets.MainMenuMusic.stream, AudioCallback);
}