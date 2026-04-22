//
// Created by lalit on 10/16/2025.
//

#ifndef BOUNCINGPLUS_MENU_H
#define BOUNCINGPLUS_MENU_H
#include <raylib.h>
#include <string>

#include "../../level/LevelLoader.h"
#include "../core/SharedManager.h"

class Menu {

    static float MusicLevel;
    static void AudioCallback(void* buffer, unsigned int frames);

    std::string Map;
    std::string TargetMap;
    Vector2 MousePos;
    float CameraX;
    float Offset1, Offset2, Offset3;
    float CameraTargetX;
    float TitleImgY;
    float TitleImgOffsetY;
    float PlayButtonOffsetY;
    float SettingsButtonOffsetY;
    float CreditsButtonOffsetY;
    float MikuOffset;
    float MenuImgOffsetY;
    float BlackTransparency;
    bool MovingToGame;
    bool isStarting;
    SharedManager* Shared;

    float MenuMusicLevel = 0.0f;

    int CurrentLevelsPage;

    void LevelSelect();
    void Credits();

    public:
        Menu(SharedManager& GameSettings);
        void Update();
        void Reset();
        std::string LeaveMenu();
        void Quit();
};


#endif //BOUNCINGPLUS_MENU_H