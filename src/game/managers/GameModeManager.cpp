//
// Created by Rolpon on 1/26/2026.
//

#include "GameModeManager.h"
#include "./ParticleManager.h"
#include "./ResourceManager.h"
#include "./EntityManager.h"
#include "../core/SharedManager.h"
#include <nlohmann/json.hpp>
#include "../../entities/subentities/stationary/Spawner.h"
#include "../../entities/subentities/player/Player.h"
#include "../../entities/subentities/enemy/Enemy.h"
#include "../Game.h"
#include <raymath.h>

GameModeManager::~GameModeManager()
{
}

GameModeManager::GameModeManager()
{
}

GameModeManager::GameModeManager(Game* game)
{
    this->game = game;
    Clear();
}

void GameModeManager::PrepareGameMode(nlohmann::json Data, std::string LevelName)
{
    Clear();
    this->CurrentLevelName = LevelName;
    if (Data.contains("game"))
    {
        if (Data["game"].contains("win"))
            WinCondition = Data["game"]["win"].get<std::string>();
        if (Data["game"].contains("mode"))
            this->CurrentGameMode = Data["game"]["mode"].get<std::string>();
        if (Data["game"].contains("timer"))
            this->LevelTimer = Data["game"]["timer"].get<float>();
        if (Data["game"].contains("time_pass_speed"))
            this->LevelGameSpeed = Data["game"]["time_pass_speed"].get<float>();
    }
    game->GameSpeed = LevelGameSpeed;
}

void GameModeManager::PrepareForInfiniteMode()
{
    InfiniteMode = true;
    PrepareGameMode(game->GameShared->LevelData["Infinite"], "Infinite");
}

void GameModeManager::TriggerGameWin()
{
    WonLevel = true;
    game->FinalLevelCompletionScore = game->GameScore;
    game->GameShared->Progress.Data.Money += static_cast<int>(round(game->FinalLevelCompletionScore / 100.0f));
}

void GameModeManager::Update()
{
    if (LevelTimer > 0)
        LevelTimer -= game->GetGameDeltaTime();

    if (!WonLevel)
    {
        std::vector<std::shared_ptr<Entity>> array = game->GameEntities->Entities[EnemyType];
        if (WinCondition == "kill_all_enemies" && array.size() == 0)
            TriggerGameWin();
        else if (WinCondition == "complete_10_waves" && CurrentWave >= 10)
            TriggerGameWin();
        else if (WinCondition == "complete_9999999_waves" && CurrentWave >= 9999999)
            TriggerGameWin();
    }

    if (CurrentGameMode == "wave")
    {
        if (CurrentWave < 10)
        {
            if (LevelTimer <= 0 && InWave)
            {
                InWave = false;
                LevelTimer = std::max(15.0f - CurrentWave * 1.25f, 5.0f);
            }
            else if (LevelTimer <= 0 && !InWave)
            {
                InWave = true;
                LevelTimer = std::min(17.5f + (CurrentWave * 4.0f), 60.0f);
                CurrentWave += 1;
            }

            std::vector<std::shared_ptr<Entity>> array = game->GameEntities->Entities[SpawnerType];
            for (int i = 0; i < array.size(); i++)
            {
                if (std::shared_ptr<Spawner> entity = dynamic_pointer_cast<Spawner>(array.at(i)); entity != nullptr and
                !entity->ShouldDelete
                )
                {
                    entity->SpawnerIsActive = LevelTimer > 0 && InWave ? 999 : 0;
                    entity->SpawnCooldown = LevelTimer > 0 && InWave
                                                ? std::max(11.0f - (CurrentWave * 0.75f), 1.0f)
                                                : 999;
                    entity->EnemyDifficulty = LevelTimer > 0 && InWave ? std::min(CurrentWave * 0.05f, 1.0f) : -1.0f;
                }
            }

            std::vector<std::shared_ptr<Entity>> enemyArray = game->GameEntities->Entities[EnemyType];
            for (int i = 0; i < enemyArray.size(); i++)
            {
                if (std::shared_ptr<Enemy> entity = dynamic_pointer_cast<Enemy>(enemyArray.at(i)); entity != nullptr and
                !entity->ShouldDelete
                )
                {
                    if (Vector2Distance({entity->BoundingBox.x, entity->BoundingBox.y}, {
                                            game->MainPlayer->BoundingBox.x, game->MainPlayer->BoundingBox.y
                                        }) > 4000)
                    {
                        entity->ShouldDelete = true;
                    }
                }
            }
        }
        else
        {
            InWave = false;
            LevelTimer = 0;
            std::vector<std::shared_ptr<Entity>> array = game->GameEntities->Entities[SpawnerType];
            for (int i = 0; i < array.size(); i++)
            {
                if (std::shared_ptr<Spawner> entity = dynamic_pointer_cast<Spawner>(array.at(i)); entity != nullptr and
                !entity->ShouldDelete
                )
                {
                    entity->SpawnerIsActive = false;
                }
            }
        }
    }
}

std::string GameModeManager::GetCurrentLevelName()
{
    return CurrentLevelName;
}

void GameModeManager::Clear()
{
    this->CurrentBoss = nullptr;
    this->InfiniteMode = false;
    this->CurrentBossName = "";
    this->WinCondition = "";
    this->CurrentLevelName = "";
    this->LevelTimer = -1;
    this->CurrentGameMode = "";
    this->CurrentWave = 0;
    this->InWave = false;
    this->WonLevel = false;
}

void GameModeManager::Quit()
{
}