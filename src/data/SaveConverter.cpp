//
// Created by lalit on 6/9/2026.
//

#include "SaveConverter.h"

#include <iostream>

#include "Progress.h"

std::vector<unsigned char> Version3_Convert(std::vector<unsigned char> save)
{
    std::vector<unsigned char> result;

    SaveDataV3* v3_data = (SaveDataV3*)save.data();

    SaveDataV4 newSaveData = SaveDataV4{};
    newSaveData.Version = 4;
    newSaveData.CursorWindowLock = v3_data->CursorWindowLock;
    memcpy(newSaveData.PlayerColor, v3_data->PlayerColor, sizeof(v3_data->PlayerColor));
    newSaveData.DevMode = v3_data->DevMode;
    newSaveData.FrameRate = v3_data->FrameRate;
    newSaveData.Money = v3_data->Money;
    newSaveData.Volume = v3_data->Volume;
    newSaveData.Fullscreen = v3_data->Fullscreen;
    newSaveData.ShakeCamera = v3_data->ShakeCamera;
    newSaveData.CosmeticParticleLimit = 400.0f;

    result.resize(sizeof(newSaveData));
    memcpy(result.data(), &newSaveData, sizeof(newSaveData));

    return result;
}

std::vector<unsigned char> Version4_Convert(std::vector<unsigned char> save)
{
    std::vector<unsigned char> result;

    SaveDataV4* v4_data = (SaveDataV4*)save.data();

    SaveData newSaveData = SaveData{};
    newSaveData.CursorWindowLock = v4_data->CursorWindowLock;
    memcpy(newSaveData.PlayerColor, v4_data->PlayerColor, sizeof(v4_data->PlayerColor));
    newSaveData.DevMode = v4_data->DevMode;
    newSaveData.FrameRate = v4_data->FrameRate;
    newSaveData.Money = v4_data->Money;
    newSaveData.Volume = v4_data->Volume;
    newSaveData.Fullscreen = v4_data->Fullscreen;
    newSaveData.ShakeCamera = v4_data->ShakeCamera;
    newSaveData.CosmeticParticleLimit = v4_data->CosmeticParticleLimit;

    result.resize(sizeof(newSaveData));
    memcpy(result.data(), &newSaveData, sizeof(newSaveData));

    return result;
}

SaveData ConvertSave(std::vector<unsigned char> save)
{
    uint32_t versionId;
    memcpy(&versionId, save.data(), sizeof(versionId));

    if (versionId != SAVE_DATA_VERSION)
    {
        std::cout << "Attempting conversion on " << versionId << "...\n";
        if (ConversionFunctions.contains(versionId))
        {
            std::vector<unsigned char> converted = ConversionFunctions[versionId](save);
            uint32_t versionId;
            memcpy(&versionId, converted.data(), sizeof(versionId));

            std::cout << "Conversion successful!" << (versionId == SAVE_DATA_VERSION ? "" : " (Still not modern, attempting another conversion...)") << "\n";
            return ConvertSave(converted);
        }
    } else
    {
        SaveData d;
        memcpy(&d, save.data(), sizeof(d));
        return d;
    }

    std::cout << "Failed conversion!\n";
    return SaveData{0};
}
