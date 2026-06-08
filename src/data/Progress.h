//
// Created by lalit on 6/6/2026.
//

#ifndef BOUNCINGPLUS_SAVE_H
#define BOUNCINGPLUS_SAVE_H
#include <cstdint>
#include "../globals.h"

#pragma pack(push, 1)
struct SaveData {
    // Save file data (recognized by all versions)
    uint32_t Version = SAVE_DATA_VERSION;

    // Player Data
    uint32_t Money = 0;
    uint8_t PlayerColor[3] = {255, 100, 255};

    // Settings
    float FrameRate = 240;
    float Volume = 100;
    bool Fullscreen = false;
    bool CursorWindowLock = false;
    bool ShakeCamera = true;
    bool DevMode = true;
};
#pragma pack(pop)

class Progress
{
    public:
    SaveData Data = SaveData{};
    Progress();
    ~Progress();
    void SaveProgress();
    void LoadProgress();

};


#endif //BOUNCINGPLUS_SAVE_H