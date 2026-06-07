//
// Created by Lalit on 10/20/2025.
//

#include "LevelLoader.h"
#include <fstream>
#include <iostream>
#include <raylib.h>
#include "vector"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

std::unordered_map<std::string,json> GetLevelsData()
{
    std::unordered_map<std::string,json> level_data = std::unordered_map<std::string,json>();
    std::vector<std::string> level_order = std::vector<std::string>();

    try
    {

        std::ifstream g("assets/maps/LevelOrder.txt");
        std::string line;
        while(std::getline(g,line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (!line.empty())
                level_order.push_back(line);
        }

        for (int i = 0; i < level_order.size(); i++)
        {
            std::string c = "assets/maps/" + level_order[i] + "/metadata.json";

            std::ifstream f(c);
            json h = json::parse(f);
            level_data[level_order[i]] = h;
            f.close();
        }

        g.close();
    } catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    for (int i = 0; i < level_order.size(); i++)
    {
        std::string s = level_order[i];
        if (level_data.count(s))
            level_data[s]["order"] = i;
    }

    int i = level_order.size();
    for (auto &[name, val] : level_data)
    {
        if (!val.count("order"))
        {
            val["order"] = i;
            i++;
        }
    }

    return level_data;
}

void ReloadLevel(std::string LevelName, std::map<std::string, json>& LevelData)
{
    try
    {
        std::string c = "assets/maps/" + LevelName + "/metadata.json";
        std::ifstream f(c);
        json h = json::parse(f);
        LevelData[LevelName] = h;
        f.close();
    } catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}
