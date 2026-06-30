//
// Created by Rolpon on 10/24/2025.
//

#include "EntityManager.h"
#include <memory>
#include <unordered_map>
#include <vector>

#include "CameraManager.h"
#include "../Game.h"
#include "../../entities/Entity.h"
#include "../../entities/subentities/player/Player.h"

EntityManager::EntityManager()
{
}

EntityManager::EntityManager(Game* game)
{
    this->EntityMemoryUsage = 0.0f;
    this->game = game;

#ifndef PLATFORM_WEB
    PhysicsFPS = 200.0f;
#else
    PhysicsFPS = 120.0f;
#endif

    PhysicsAccumulator = 0.0f;

    Entities = std::unordered_map<EntityType, std::vector<std::shared_ptr<Entity>>>();
    EntityPhysicsBlacklist = std::vector<EntityType>();

    for (int i = 0; i < EndOfEntityType; ++i)
    {
        Entities.insert({(EntityType)i, std::vector<std::shared_ptr<Entity>>()});
    }

    EntityPhysicsBlacklist.push_back(SpawnerEntityType);
    EntityPhysicsBlacklist.push_back(TurretEntityType);
}

EntityManager::~EntityManager()
{
}

bool EntityManager::EntityFitsPriority(std::shared_ptr<Entity> EntityToCheck)
{
    if (EntityToCheck->Priority == AlwaysPriorityType)
        return true;
    if (EntityToCheck->Priority == NeverPriorityType)
        return false;

    float Distance = Vector2Distance(EntityToCheck->GetCenter(), game->MainPlayer->GetCenter());
    if (EntityToCheck->Priority == CloseToPlayerPriorityType && Distance <= 1850)
        return true;
    if (EntityToCheck->Priority == NearbyPlayerPriorityType && Distance <= 3500)
        return true;
    if (EntityToCheck->Priority == LargeAreaNearbyPlayerPriorityType && Distance <= 6100)
        return true;

    return false;
}

void EntityManager::AddEntity(EntityType Type, std::shared_ptr<Entity> EntityToAdd)
{
    EntityMemoryUsage += sizeof(*EntityToAdd);
    Entities[Type].push_back(EntityToAdd);
}

void EntityManager::EntityUpdate()
{
    if (!game->MainPlayer->ShouldDelete)
        game->MainPlayer->Update();

    for (int e = 0; e < EndOfEntityType; ++e)
    {
        if (static_cast<EntityType>(e) == PlayerEntityType)
            continue;
        std::vector<shared_ptr<Entity>>* array = &Entities[(EntityType)e];
        for (int i = 0; i < array->size(); i++)
        {
            if (shared_ptr<Entity> entity = array->at(i); entity != nullptr && !entity->ShouldDelete && EntityFitsPriority(entity))
            {
                entity->Update();
            }
        }
    }
}

void EntityManager::EntityPhysicsUpdate()
{
    PhysicsAccumulator += game->GetGameDeltaTime();
    double g = game->GetGameTime();

    while (PhysicsAccumulator >= 1.0f / PhysicsFPS)
    {
        double start = game->GetGameTime();

        if (!game->MainPlayer->ShouldDelete)
            game->MainPlayer->PhysicsUpdate(1.0f / PhysicsFPS, g + (game->GetGameTime() - start));

        for (int e = 0; e < EndOfEntityType; ++e)
        {
            if (static_cast<EntityType>(e) == PlayerEntityType)
                continue;

            bool BlacklistedType = false;
            for (EntityType OtherType : EntityPhysicsBlacklist)
            {
                if (OtherType == e)
                {
                    BlacklistedType = true;
                    break;
                }
            }

            if (BlacklistedType)
                continue;

            std::vector<shared_ptr<Entity>>* EntityArray = &Entities[(EntityType)e];
            for (auto entity : *EntityArray)
            {
                if (entity != nullptr  && !entity->ShouldDelete && EntityFitsPriority(entity))
                {
                    entity->PhysicsUpdate(1.0f / PhysicsFPS, g + (game->GetGameTime() - start));
                }
            }
        }
        PhysicsAccumulator -= 1.0f / PhysicsFPS;
    }
}

void EntityManager::EntityClear()
{
    for (int e = 0; e < EndOfEntityType; e++)
    {
        std::vector<shared_ptr<Entity>>* array = &Entities[(EntityType)e];
        int old_size = array->size();
        (void)std::erase_if(*array, [this](shared_ptr<Entity>& e)
        {
            if (e && (e->ShouldDelete || e->Health <= 0.0f))
            {
                e->OnDeath();
                e->OnDelete();
                if (e != game->MainPlayer)
                {
                    e.reset();
                    EntityMemoryUsage -= sizeof(*e);
                }
                return true;
            }
            return false;
        });
        if (game->DebugDraw)
        {
            std::string f = "ENTITY CATEGORY: " + to_string(e) + ", OLD SIZE: " + to_string(old_size) + ", NEW SIZE: " +
                to_string(array->size());
            DrawText(f.c_str(), static_cast<int>(static_cast<float>(800) + this->game->GameCamera->RaylibCamera.target.x),
                     static_cast<int>(static_cast<float>(50 + 10 * e) + this->game->GameCamera->RaylibCamera.target.y), 10, WHITE);
        }
    }
}

void EntityManager::Update()
{
    EntityUpdate();
    EntityPhysicsUpdate();
    EntityClear();
}

void EntityManager::Clear()
{
    for (int e = 0; e < EndOfEntityType; e++)
    {
        std::vector<shared_ptr<Entity>>* array = &Entities[(EntityType)e];
        for (int i = 0; i < array->size(); i++)
        {
            if (shared_ptr<Entity>& entity = array->at(i); entity != nullptr)
            {
                entity->OnDelete();
                entity.reset();
            }
        }
        array->clear();
    }
    EntityMemoryUsage = 0;
}

void EntityManager::Quit()
{
    Clear();
    EntityPhysicsBlacklist.clear();
    Entities.clear();
}