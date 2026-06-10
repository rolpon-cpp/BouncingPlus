//
// Created by lalit on 6/6/2026.
//

#include "Progress.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "string"

Progress::Progress()
{
}

Progress::~Progress()
{
}

void Progress::SaveProgress()
{
    for (int i = 0; i < 5; i++)
    {
        try
        {
            char data[sizeof(SaveData)];
            memcpy(&data, &Data, sizeof(SaveData));

            std::ofstream outFile("data.bin", std::ios::out | std::ios::binary);
            outFile.write(data, sizeof(SaveData));
            outFile.close();
            return;
        } catch (std::exception& e)
        {
            std::cout << "Failed to save file! (" << e.what() << ")\n";
        }
    }
}

void Progress::LoadProgress()
{
    try
    {
        std::ifstream input("data.bin", std::ios::binary);
        std::vector<unsigned char> vbuffer(std::istreambuf_iterator<char>(input), {});

        char buffer[vbuffer.size()];
        memcpy(&buffer, vbuffer.data(), sizeof(buffer));

        if (vbuffer.size() < sizeof(SaveDataV4))
        {
            std::cout << "Failed to load save file! (wrong data size, minimum size: " << sizeof(SaveDataV4) << ", real size: " << buffer.size() << ")\n";
            return;
        }

        vbuffer.clear();

        // TODO: check if this code is gonna blow up or not

        uint32_t version;
        memcpy(&version, buffer, sizeof(uint32_t));
        if (version != SAVE_DATA_VERSION)
        {
            std::cout << "File is in the wrong version! (expected version: " << SAVE_DATA_VERSION << ", real version: " << version << ")\n";
            std::cout << "Attempting to convert...\n";

            SaveData test = ConvertOldSave(buffer);
            if (test.Version == 0)
            {
                std::cout << "Failed to convert save file!\n";
                return;
            }
            Data = test;
        } else
        {
            memcpy(&Data, buffer, sizeof(SaveData));
        }

        input.close();
    } catch (std::exception& e)
    {
        std::cout << "Failed to load save file! (unknown issue, review exception: " << e.what() << ")\n";
    }
}
