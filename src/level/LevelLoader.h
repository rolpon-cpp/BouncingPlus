//
// Created by Rolpon on 10/20/2025.
//

#ifndef LEVELLOADER_H
#define LEVELLOADER_H
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::unordered_map<std::string, json> GetLevelsData();
void ReloadLevel(std::string LevelName, std::map<std::string, json>& LevelData);

#endif //LEVELLOADER_H