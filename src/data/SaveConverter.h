//
// Created by lalit on 6/9/2026.
//

#ifndef BOUNCINGPLUS_SAVECONVERTER_H
#define BOUNCINGPLUS_SAVECONVERTER_H

#include <unordered_map>

struct SaveData;

#pragma pack(push, 1)
struct SaveDataV4 {
    uint32_t Version;
    uint32_t Money;
    uint8_t PlayerColor[3];
    float FrameRate;
    float Volume;
    bool Fullscreen;
    bool CursorWindowLock;
    bool ShakeCamera;
    bool DevMode;
    float CosmeticParticleLimit;
};
#pragma pack(pop)

void* Version4_Convert(void* save);

inline std::unordered_map<uint32_t, void*(*)(void* save)> ConversionFunctions {
        {4, &Version4_Convert}
};

SaveData ConvertOldSave(void* save);

#endif //BOUNCINGPLUS_SAVECONVERTER_H