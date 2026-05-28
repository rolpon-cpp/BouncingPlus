//
// Created by lalit on 1/26/2026.
//

#include "GameModeManager.h"

#include <iostream>

#include <nlohmann/json.hpp>
#include "../../entities/subentities/stationary/Spawner.h"
#include "../Game.h"
#include <raymath.h>

GameModeManager::~GameModeManager()
{
}

GameModeManager::GameModeManager()
{
}

GameModeManager::GameModeManager(Game& game)
{
    this->game = &game;
    Clear();
}

void GameModeManager::PrepareGameMode(nlohmann::json Data)
{
    Clear();
    if (Data.contains("game"))
    {
        if (Data["game"].contains("mode"))
            this->CurrentGameMode = Data["game"]["mode"].get<std::string>();
        if (Data["game"].contains("timer"))
            this->LevelTimer = Data["game"]["timer"].get<float>();
        if (Data["game"].contains("time_pass_speed"))
            this->LevelGameSpeed = Data["game"]["time_pass_speed"].get<float>();
    }


    game->GameSpeed = LevelGameSpeed;
}

void GameModeManager::Update()
{
    if (LevelTimer > 0)
        LevelTimer -= game->GetGameDeltaTime();

    if (game->GameShared->LevelData[game->CurrentLevelName]["game"]["win"].get<std::string>() == "kill_all_enemies")
    {
        std::vector<shared_ptr<Entity>> array = game->GameEntities.Entities[EnemyType];
        if (array.size() == 0)
            WonLevel = true;
    }

    if (CurrentGameMode == "wave")
    {
        if (LevelTimer <= 0 && InWave)
        {
            InWave = false;
            LevelTimer = max(15.0f - CurrentWave * 1.25f, 5.0f);
        } else if (LevelTimer <= 0 && !InWave)
        {
            InWave = true;
            LevelTimer = min(17.5f + (CurrentWave * 4.0f), 60.0f);
            CurrentWave += 1;
        }
        std::vector<shared_ptr<Entity>> array = game->GameEntities.Entities[SpawnerType];
        for (int i = 0; i < array.size(); i++) {
            if (shared_ptr<Spawner> entity = dynamic_pointer_cast<Spawner>(array.at(i)); entity != nullptr and !entity->ShouldDelete) {
                entity->SpawnerIsActive = LevelTimer > 0 && InWave ? 999 : 0;
                entity->SpawnCooldown = LevelTimer > 0 && InWave ? max(11.0f - (CurrentWave * 1.25f), 1.0f) : 999;
                entity->EnemyDifficulty = LevelTimer > 0 && InWave ? min(CurrentWave * 0.15f, 1.0f) : -1.0f;
            }
        }

        std::vector<shared_ptr<Entity>> enemyArray = game->GameEntities.Entities[EnemyType];
        for (int i = 0; i < enemyArray.size(); i++) {
            if (shared_ptr<Enemy> entity = dynamic_pointer_cast<Enemy>(enemyArray.at(i)); entity != nullptr and !entity->ShouldDelete) {
                if (Vector2Distance({entity->BoundingBox.x, entity->BoundingBox.y},{game->MainPlayer->BoundingBox.x,game->MainPlayer->BoundingBox.y}) > 4000)
                {
                    entity->ShouldDelete = true;
                }
            }
        }
    }
}

void GameModeManager::Clear()
{
    this->CurrentBoss = nullptr;
    this->CurrentBossName = "";
    this->LevelTimer = -1;
    this->CurrentGameMode = "";
    this->CurrentWave = 0;
    this->InWave = false;
    this->WonLevel = false;
}

void GameModeManager::Quit()
{
}
