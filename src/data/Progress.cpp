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
        } catch (std::exception e)
        {
            std::cout << "Failed to save file! (" << e.what() << ")\n";
        }
    }
}

void Progress::LoadProgress()
{
    try
    {
        std::ifstream input( "data.bin", std::ios::binary );
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

        if (buffer.size() != sizeof(SaveData))
        {
            std::cout << "Failed to load save file! (wrong data size, expected size: " << sizeof(SaveData) << ", real size: " << buffer.size() << ")\n";
            return;
        }

        uint32_t version;
        memcpy(&version, buffer.data(), sizeof(uint32_t));
        if (version != SAVE_DATA_VERSION)
        {
            std::cout << "Failed to load save file! (wrong version, expected version: " << SAVE_DATA_VERSION << ", real version: " << version << ")\n";
            return;
        }

        memcpy(&Data, buffer.data(), sizeof(SaveData));

        buffer.clear();
        input.close();
    } catch (std::exception e)
    {
        std::cout << "Failed to load save file! (unknown issue, review exception: " << e.what() << ")\n";
    }
}
