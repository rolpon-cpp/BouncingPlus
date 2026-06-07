//
// Created by lalit on 6/6/2026.
//

#ifndef BOUNCINGPLUS_SAVE_H
#define BOUNCINGPLUS_SAVE_H
#include <cstdint>

#define SAVE_DATA_VERSION 1

#pragma pack(push, 1)
struct SaveData {
    uint32_t Version = SAVE_DATA_VERSION;
    uint32_t Money = 0;
    uint8_t PlayerColor[3] = {255, 100, 255};
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