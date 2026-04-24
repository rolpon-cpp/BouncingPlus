//
// Created by lalit on 3/8/2026.
//

#include "ResourceManager.h"

#include <fstream>
#include <iostream>

#include "../Game.h"
#include "../../entities/systems/Weapons.h"
#include "../../entities/systems/Powerups.h"
#include "filesystem"
#include "nlohmann/json.hpp"
namespace fs = std::filesystem;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

ResourceManager::ResourceManager(Game& game)
{
    this->game = &game;
    Textures = std::unordered_map<std::string, Texture2D>();
    Weapons = std::unordered_map<std::string, Weapon>();
    Powerups = std::unordered_map<std::string, Powerup*>();
    Shaders = std::unordered_map<std::string, Shader>();
    EnemyWeaponNamesList= std::vector<std::string>();
}

void ResourceManager::Load()
{
    std::string path = "assets/img";
    for (const auto & entry : fs::directory_iterator(path)) {
        std::string p = entry.path().filename().string();
        p.erase(p.end() - 4, p.end());
        Texture tex = LoadTexture(entry.path().string().c_str());
        Textures.insert({p, tex});
    }
    path = "assets/shaders";
    for (const auto & entry : fs::directory_iterator(path)) {
        std::string p = entry.path().filename().string();
        p.erase(p.end() - 5, p.end());

        if (p != "vertex")
        {
            Shader shader = LoadShader((path+"/vertex.glsl").c_str(),entry.path().string().c_str());
            Shaders.insert({p, shader});
        }
    }
    path = "assets/weapondata";
    for (const auto & entry : fs::directory_iterator(path)) {
        try
        {
            std::string p = entry.path().filename().string();
            p.erase(p.end() - 5, p.end());
            std::ifstream g(entry.path().c_str());
            if (!g.is_open()) {
                std::cout << "WARNING: RESOURCEMANAGER: Failed to open: "
                          << entry.path() << std::endl;
                continue;
            }
            nlohmann::json data = nlohmann::json::parse(g);

            Weapon wep = {};
            if (data.count("EnemiesCanUse") && data["EnemiesCanUse"].get<bool>())
                EnemyWeaponNamesList.push_back(p);
            if (data.count("isMelee"))
                wep.isMelee = data["isMelee"].get<bool>();
            if (data.count("Throw"))
                wep.Throwable = data["Throw"].get<bool>();
            if (data.count("ShakeScreen"))
                wep.ShakeScreen = data["ShakeScreen"].get<bool>();
            if (data.count("SlowdownOverTime"))
                wep.SlowdownOverTime = data["SlowdownOverTime"].get<bool>();
            if (data.count("PushbackForce"))
                wep.PushbackForce = data["PushbackForce"].get<float>();
            if (data.count("BulletLifetime"))
                wep.BulletLifetime = data["BulletLifetime"].get<float>();
            if (data.count("ChargeSpeed"))
                wep.ChargeSpeed = data["ChargeSpeed"].get<float>();
            if (data.count("Spread"))
            {
                wep.SpreadRange[0] = data["Spread"][0].get<float>();
                wep.SpreadRange[1] = data["Spread"][1].get<float>();
            }
            if (data.count("WeaponWeightSpeedMultiplier"))
                wep.WeaponWeightSpeedMultiplier = data["WeaponWeightSpeedMultiplier"].get<float>();
            if (data.count("Speed"))
                wep.Speed = data["Speed"].get<float>();
            if (data.count("WeaponSize"))
                wep.WeaponSize = data["WeaponSize"].get<float>();
            if (data.count("Ammo"))
                wep.Ammo = data["Ammo"].get<int>();
            if (data.count("ReloadTime"))
                wep.ReloadTime = data["ReloadTime"].get<double>();
            if (data.count("Size"))
                wep.Size = {data["Size"][0], data["Size"][1]};
            if (data.count("Damage"))
                wep.Damage = data["Damage"].get<float>();
            if (data.count("HealthGain"))
                wep.HealthGain = data["HealthGain"].get<float>();
            if (data.count("Cooldown"))
                wep.Cooldown = data["Cooldown"].get<float>();
            if (data.count("AngleRange"))
                wep.AngleRange = data["AngleRange"].get<float>();
            if (data.count("Range"))
                wep.Range = data["Range"].get<float>();
            if (data.count("Bullets"))
                wep.Bullets = data["Bullets"].get<int>();
            if (data.count("Intensity"))
                wep.Intensity = data["Intensity"].get<float>();
            if (data.count("texture"))
                wep.texture = data["texture"].get<string>();
            if (data.count("bullet_tex"))
                wep.BulletTexture = data["bullet_tex"].get<string>();
            if (data.count("sound"))
            {
                wep.sound = data["sound"].get<vector<string>>();
            }
            Weapons.insert({p, wep});
            g.close();
        } catch (...)
        {
            cout << "WARNING: WEAPON: Failed to read " << entry << "\n";
        }
    }

    auto* p = new SpeedPowerup();
    auto* s = new ShieldPowerup();
    auto* f = new FreezePowerup();
    auto* t = new TankyPowerup();
    Powerups.insert({"speed", p});
    Powerups.insert({"freeze", f});
    Powerups.insert({"shield", s});
    Powerups.insert({"tank", t});
}

void ResourceManager::Quit()
{
    for (auto& [name,value] : Powerups)
        delete value;
    for (auto& [name,value] : Textures)
        UnloadTexture(value);
    for (auto& [name,value] : Shaders)
        UnloadShader(value);
    EnemyWeaponNamesList.clear();
}
