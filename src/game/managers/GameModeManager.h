//
// Created by lalit on 1/26/2026.
//

#ifndef BOUNCINGPLUS_GAMEMODE_H
#define BOUNCINGPLUS_GAMEMODE_H

#include <nlohmann/json_fwd.hpp>

#include "string"

class Game;
class Entity;

/*
 Game Modes:
boss
wave
default
 */

class GameModeManager
{
    void TriggerGameWin();
    public:
    Game *game;
    Entity* CurrentBoss;
    std::string CurrentBossName;
    std::string CurrentGameMode;
    std::string WinCondition;
    int CurrentWave;
    double LevelGameSpeed;
    bool InWave;
    double LevelTimer;
    bool WonLevel;
    ~GameModeManager();
    GameModeManager();
    GameModeManager(Game &game);
    void PrepareGameMode(nlohmann::json Data);
    void Update();
    void Clear();
    void Quit();
};


#endif //BOUNCINGPLUS_GAMEMODE_H