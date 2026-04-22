//
// Created by lalit on 3/14/2026.
//

#ifndef BOUNCINGPLUS_UIUTILS_H
#define BOUNCINGPLUS_UIUTILS_H
#include <raylib.h>
#include <string>

class UIAssets
{
    public:
    Texture2D TitleImg;
    Texture2D MenuImg;
    Texture2D ButtonImg;
    Texture2D ButtonSmallImg;
    Texture2D ButtonSmallRedImg;
    Texture2D MikuImg;
    Texture2D CursorImg;
    Texture2D YTImg;

    Texture2D RolponPFPImg;
    Texture2D EggPFPImg;
    Texture2D CozPFPImg;
    Texture2D InkyPFPImg;

    Sound MikuMusic;
    Sound SliderDrag;
    Sound ButtonClick;

    Music MainMenuMusic;

    void Load();
    void Quit();
};

Rectangle Slider(Vector2 Position, Vector2 MousePos, Sound& SliderDrag,
                 Texture2D& ButtonSmallImg, std::string Text, float* Value,
                 float* LastPlayedProgress, bool* PrevState, float Min, float Max);
bool Button(Rectangle ButtonRectangle, Vector2 MousePos, Texture2D& ButtonImg, Sound& ButtonClick, std::string Text);
void Checkmark(Vector2 Position, Vector2 MousePos, Sound& CheckmarkClick,
                 Texture2D& ButtonSmallImg, Texture2D& ButtonSmallImgRed, std::string Text, bool* Value);
Rectangle Panel(Rectangle rectangle, std::string text, float Offset1);

#endif //BOUNCINGPLUS_UIUTILS_H