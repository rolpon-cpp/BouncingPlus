//
// Created by lalit on 3/14/2026.
//

#include "raylib.h"
#include "string"
#include "UIUtils.h"

void UIAssets::Load()
{
    TitleImg = LoadTexture("assets/ui/title.png");
    ButtonSmallImg = LoadTexture("assets/ui/button_small.png");
    ButtonSmallRedImg = LoadTexture("assets/ui/button_small_red.png");
    ButtonImg = LoadTexture("assets/ui/button.png");
    MenuImg = LoadTexture("assets/ui/menu_img.png");
    MikuImg = LoadTexture("assets/ui/miku.png");
    MikuMusic = LoadSound("assets/ui/lovely_cavity.mp3");
    ButtonClick = LoadSound("assets/ui/button_click.wav");
    SliderDrag = LoadSound("assets/ui/slider_dragging.wav");
    CursorImg = LoadTexture("assets/ui/cursor.png");
    YTImg = LoadTexture("assets/ui/youtube.png");
    RolponPFPImg = LoadTexture("assets/ui/rolpon_pfp.png");
    CozPFPImg = LoadTexture("assets/ui/coz_pfp.png");
    EggPFPImg = LoadTexture("assets/ui/egg_pfp.png");
    InkyPFPImg = LoadTexture("assets/ui/inky_pfp.png");
    MainMenuMusic = LoadMusicStream("assets/ui/main_menu.mp3");
}

void UIAssets::Quit()
{
    UnloadTexture(TitleImg);
    UnloadSound(MikuMusic);
    UnloadTexture(ButtonSmallImg);
    UnloadTexture(ButtonSmallRedImg);
    UnloadTexture(MenuImg);
    UnloadTexture(ButtonImg);
    UnloadTexture(MikuImg);
    UnloadTexture(CursorImg);
    UnloadSound(SliderDrag);
    UnloadSound(ButtonClick);
    UnloadTexture(RolponPFPImg);
    UnloadTexture(CozPFPImg);
    UnloadTexture(EggPFPImg);
    UnloadTexture(InkyPFPImg);
    UnloadTexture(YTImg);
    UnloadMusicStream(MainMenuMusic);
}

void Checkmark(Vector2 Position, Vector2 MousePos, Sound& CheckmarkClick,
                 Texture2D& ButtonSmallImg, Texture2D& ButtonSmallImgRed, std::string Text, bool* Value)
{

    float checkmarks_size = 100;

    float fnt_size = 35;
    float tx_size = MeasureText(Text.c_str(), fnt_size);
    float w = tx_size + checkmarks_size + 48;
    float h = fnt_size + 16;
    Position.x -= w/2;
    Rectangle rec = {Position.x, Position.y,w, h};

    DrawRectangleRec({rec.x,rec.y,rec.width,rec.height}, ColorAlpha(BLACK, 0.5f));
    if (!Text.empty())
        DrawText(Text.c_str(), Position.x + 16, Position.y + (rec.height / 2.0f) - (fnt_size / 2.0f), fnt_size, WHITE);

    DrawTexturePro(ButtonSmallImg, {0,0,56,56}, {Position.x + tx_size + 32, rec.y + rec.height/2 - 20, 40,40}, {0, 0}, 0, WHITE);
    DrawTexturePro(ButtonSmallImgRed, {0,0,56,56}, {rec.x + rec.width - 56, rec.y + rec.height/2 - 20, 40,40}, {0, 0}, 0, WHITE);

    Vector2 HighlightPos = {Position.x + tx_size + 32, rec.y + rec.height/2 - 20};
    Vector2 OtherHighlightPos = {rec.x + rec.width - 56, rec.y + rec.height/2 - 20};
    if (!*Value)
    {
        HighlightPos = {rec.x + rec.width - 56, rec.y + rec.height/2 - 20};
        OtherHighlightPos = {Position.x + tx_size + 32, rec.y + rec.height/2 - 20};
    }

    Rectangle CheckmarkRect = {HighlightPos.x, HighlightPos.y, 40, 40};
    Rectangle OtherCheckmarkRect = {OtherHighlightPos.x, OtherHighlightPos.y, 40, 40};
    DrawRectangleLinesEx(CheckmarkRect, 5, WHITE);

    if (CheckCollisionPointRec(MousePos, OtherCheckmarkRect) && IsMouseButtonPressed(0))
    {
        *Value = !*Value;
        if (!IsSoundPlaying(CheckmarkClick))
            PlaySound(CheckmarkClick);
    }
}

Rectangle Slider(Vector2 Position, Vector2 MousePos, Sound& SliderDrag,
                 Texture2D& ButtonSmallImg, std::string Text, float* Value,
                 float* LastPlayedProgress, bool* PrevState, float Min, float Max)
{
    float percentage = std::max(std::min((*Value - Min) / (Max - Min), 1.0f), 0.0f);

    float slider_size = 350;

    float fnt_size = 35;
    float tx_size = MeasureText(Text.c_str(), fnt_size);
    float w = tx_size + slider_size + 48;
    float h = fnt_size + 16;
    Position.x -= w/2;
    Rectangle rec = {Position.x, Position.y,w, h};

    Rectangle slider_rec = {Position.x + tx_size + 32, Position.y + (rec.height / 2) - (fnt_size / 2.5f), slider_size, fnt_size / 1.25f};

    Rectangle green_slider_rec = {slider_rec.x, slider_rec.y, slider_rec.width * percentage, slider_rec.height};

    Rectangle red_slider_rec = {slider_rec.x + (slider_rec.width * percentage), slider_rec.y, slider_rec.width * (1.0f - percentage), slider_rec.height};

    bool colliding = CheckCollisionPointRec(MousePos, slider_rec);
    bool clicking = IsMouseButtonDown(0);
    if (!clicking)
        *PrevState = false;
    bool dragging = (*PrevState || colliding) && clicking;

    if (dragging)
    {
        *PrevState = true;
        float distance = std::min(std::max(MousePos.x - slider_rec.x, 0.0f), slider_size);
        float percent = distance / slider_size;
        if (percent >= 0.95f)
            percent = 1.0f;
        if (percent <= 0.05f)
            percent = 0.0f;
        if (percent >= 0.45f && percent <= 0.55f)
            percent = 0.5f;
        *Value = Min + (Max - Min) * percent;
        if (abs(*Value - *LastPlayedProgress) >= 0.1f && !IsSoundPlaying(SliderDrag))
        {
            PlaySound(SliderDrag);
            *LastPlayedProgress = *Value;
        }

    }

    DrawRectangleRec({rec.x,rec.y,rec.width,rec.height}, ColorAlpha(BLACK, 0.5f));
    DrawRectangleRec({green_slider_rec.x,green_slider_rec.y,green_slider_rec.width,green_slider_rec.height}, GREEN);
    DrawRectangleRec({red_slider_rec.x,red_slider_rec.y,red_slider_rec.width,red_slider_rec.height}, RED);
    if (!Text.empty())
        DrawText(Text.c_str(), Position.x + 16, Position.y + (rec.height / 2.0f) - (fnt_size / 2.0f),fnt_size, WHITE);

    float siz = fnt_size + 5;
    if (colliding)
        siz = fnt_size + 7;
    DrawTexturePro(ButtonSmallImg, {0, 0, 56, 56}, {green_slider_rec.x + green_slider_rec.width, green_slider_rec.y + green_slider_rec.height/2, siz, siz}, {siz / 2, siz / 2}, 0, WHITE);

    float ind_fnt_size = 20;
    float ind_size = MeasureText(std::to_string((int)*Value).c_str(), ind_fnt_size);
    DrawTextEx(GetFontDefault(), std::to_string((int)*Value).c_str(), {green_slider_rec.x + green_slider_rec.width - ind_size/2, green_slider_rec.y + green_slider_rec.height/2 - ind_fnt_size/2}, ind_fnt_size, 1, BLACK);

    return rec;
}

bool Button(Rectangle ButtonRectangle, Vector2 MousePos, Texture2D& ButtonImg, Sound& ButtonClick, std::string Text) {

    int f= ButtonRectangle.height - 6.0f;
    if (CheckCollisionPointRec(MousePos, ButtonRectangle))
    {
        ButtonRectangle.width += 20;
        ButtonRectangle.height += 20;
        ButtonRectangle.x -= 10;
        ButtonRectangle.y -=10;
        f += 10;
    }

    float tx_size = MeasureText(Text.c_str(), f);
    float mul = f / (tx_size / (ButtonRectangle.width-10));
    mul= std::min(mul,ButtonRectangle.height);
    tx_size = MeasureText(Text.c_str(), mul);
    DrawTexturePro(ButtonImg, {0,0,7.0f,(float)ButtonImg.height}, {ButtonRectangle.x, ButtonRectangle.y, 7, ButtonRectangle.height}, {0, 0}, 0, WHITE);
    DrawTexturePro(ButtonImg, {(float)ButtonImg.width - 7.0f,0,7.0f,(float)ButtonImg.height}, {ButtonRectangle.x + ButtonRectangle.width - 7, ButtonRectangle.y, (ButtonRectangle.width/ButtonImg.width) * 7.0f, ButtonRectangle.height}, {0, 0}, 0, WHITE);
    DrawTexturePro(ButtonImg, {7.0f,0,(float)ButtonImg.width-14.0f,(float)ButtonImg.height}, {ButtonRectangle.x + 7, ButtonRectangle.y, ButtonRectangle.width - 14.0f, ButtonRectangle.height}, {0, 0}, 0, WHITE);
    if (!Text.empty())
        DrawText(Text.c_str(), ButtonRectangle.x + ButtonRectangle.width/2 - tx_size/2, ButtonRectangle.y + ButtonRectangle.height/2 - (mul/2), mul, WHITE);

    if (CheckCollisionPointRec(MousePos, ButtonRectangle)) {
        DrawRectangleLinesEx({ButtonRectangle.x,ButtonRectangle.y,ButtonRectangle.width,ButtonRectangle.height}, 4, WHITE);
        if (IsMouseButtonPressed(0) && !IsSoundPlaying(ButtonClick))
        {
            PlaySound(ButtonClick);
        }
        return IsMouseButtonPressed(0);
    }
    return false;
}

Rectangle Panel(Rectangle rectangle, std::string text, float Offset1)
{
    Vector2 PanelSize = Vector2{rectangle.width, rectangle.height};
    Rectangle PanelRectangle = {rectangle.x, rectangle.y - Offset1, PanelSize.x, PanelSize.y};

    DrawRectangleRec({PanelRectangle.x, PanelRectangle.y, PanelRectangle.width, PanelRectangle.height}, ColorAlpha(BLACK, 0.5f));

    float TextWidth = MeasureText(text.c_str(), 55.0f);
    DrawText(text.c_str(), PanelRectangle.x + PanelRectangle.width/2.0f - TextWidth/2.0f, PanelRectangle.y + 25.0f, 55.0f, WHITE);

    DrawLineEx(Vector2{PanelRectangle.x + 25.0f, PanelRectangle.y + 100}, Vector2{PanelRectangle.x + PanelSize.x - 25.0f, PanelRectangle.y + 100}, 4, WHITE);

    return PanelRectangle;
}
