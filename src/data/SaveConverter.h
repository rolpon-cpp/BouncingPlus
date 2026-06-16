//
// Created by lalit on 6/9/2026.
//

#ifndef BOUNCINGPLUS_SAVECONVERTER_H
#define BOUNCINGPLUS_SAVECONVERTER_H

#include <cstdint>
#include <unordered_map>
#include <vector>

struct SaveData;

#pragma pack(push, 1)
struct SaveDataV4 {
    std::uint32_t Version;
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

#pragma pack(push, 1)
struct SaveDataV3 {
    uint32_t Version;
    uint32_t Money = 0;
    uint8_t PlayerColor[3];
    float FrameRate;
    float Volume;
    bool Fullscreen;
    bool CursorWindowLock;
    bool ShakeCamera;
    bool DevMode;
};
#pragma pack(pop)

std::vector<unsigned char> Version4_Convert(std::vector<unsigned char> save);
std::vector<unsigned char> Version3_Convert(std::vector<unsigned char> save);

inline std::unordered_map<uint32_t, std::vector<unsigned char>(*)(std::vector<unsigned char> save)> ConversionFunctions {
        {3, &Version3_Convert},
        {4, &Version4_Convert}
};

SaveData ConvertSave(std::vector<unsigned char> save);

#endif //BOUNCINGPLUS_SAVECONVERTER_H