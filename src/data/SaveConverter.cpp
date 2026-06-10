//
// Created by lalit on 6/9/2026.
//

#include "SaveConverter.h"

#include <iostream>

#include "Progress.h"

void* Version4_Convert(void* save)
{
    SaveDataV4 v4_data;
    memcpy(&v4_data, save, sizeof(v4_data));

    SaveData* newSaveData = new SaveData{};
    newSaveData->CursorWindowLock = v4_data.CursorWindowLock;
    memcpy(newSaveData->PlayerColor, v4_data.PlayerColor, sizeof(v4_data.PlayerColor));
    newSaveData->DevMode = v4_data.DevMode;
    newSaveData->FrameRate = v4_data.FrameRate;
    newSaveData->Money = v4_data.Money;
    newSaveData->Volume = v4_data.Volume;
    newSaveData->Fullscreen = v4_data.Fullscreen;
    newSaveData->ShakeCamera = v4_data.ShakeCamera;

    delete static_cast<SaveData*>(save);
    return newSaveData;
}

SaveData ConvertOldSave(void* save)
{
    uint32_t version;
    memcpy(&version, save, sizeof(version));
    if (version == SAVE_DATA_VERSION)
    {
        SaveData f = *static_cast<SaveData*>(save);
        delete static_cast<SaveData*>(save);
        return f;
    }

    std::cout << "Attempting to convert " << version << " to version " << version + 1 << "...\n";
    if (ConversionFunctions.contains(version))
    {
        void* convertedObj = ConversionFunctions[version](save);
        uint32_t converted_version;
        memcpy(&converted_version, convertedObj, sizeof(converted_version));
        if (converted_version != SAVE_DATA_VERSION)
        {
            std::cout << "Conversion successful, but save file is still " << SAVE_DATA_VERSION - converted_version << " behind modern version. Attempting another conversion...\n";
            return ConvertOldSave(convertedObj);
        }
        std::cout << "Conversion successful!\n";
        SaveData f = *static_cast<SaveData*>(convertedObj);
        delete static_cast<SaveData*>(convertedObj);
        return f;
    }

    std::cout << "No conversion function found, failed to convert.\n";
    return SaveData{0};
}
