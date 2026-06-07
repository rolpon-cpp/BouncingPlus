//
// Created by lalit on 3/8/2026.
//

#ifndef BOUNCINGPLUS_RESOURCEMANAGER_H
#define BOUNCINGPLUS_RESOURCEMANAGER_H
#include <unordered_map>
#include <string>
#include <vector>
#include "raylib.h"
using namespace std;

class Game;
class Weapon;
class Powerup;

class ResourceManager
{
public:
    Game* game;
    unordered_map<std::string, Texture2D> Textures;
    unordered_map<std::string, Shader> Shaders;
    unordered_map<std::string, Weapon> Weapons;
    unordered_map<std::string, Powerup*> Powerups;
    std::vector<std::string> EnemyWeaponNamesList;
    ResourceManager();
    ~ResourceManager();
    ResourceManager(Game& game);
    void LoadWeaponData();
    void Load();
    void Quit();
};


#endif //BOUNCINGPLUS_RESOURCEMANAGER_H