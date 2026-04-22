//
// Created by Lalit on 10/24/2025.
//

#include "EntityManager.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include "../Game.h"
#include "../../entities/Entity.h"

EntityManager::EntityManager()
{
}

EntityManager::EntityManager(Game& game)
{
    this->game = &game;
    PhysicsFPS = 240.0f;
    PhysicsAccumulator = 0;
    Entities = std::unordered_map<EntityType, std::vector<shared_ptr<Entity>>>();
    for (int i = 0; i < End; ++i) {
        Entities.insert({(EntityType) i, std::vector<shared_ptr<Entity>>()});
    }
}

EntityManager::~EntityManager()
{
}

void EntityManager::AddEntity(EntityType Type, std::shared_ptr<Entity> EntityToAdd)
{
    Entities[Type].push_back(EntityToAdd);
}

void EntityManager::EntityUpdate()
{
    for (int e = 0; e < End; ++e) {
        std::vector<shared_ptr<Entity>> *array = &Entities[(EntityType)e];
        for (int i = 0; i < array->size(); i++) {
            if (shared_ptr<Entity> entity = array->at(i); entity != nullptr and !entity->ShouldDelete) {
                entity->Update();
            }
        }
    }
}

void EntityManager::EntityPhysicsUpdate()
{
    PhysicsAccumulator += GetFrameTime();
    double g = game->GetGameTime();

    while (PhysicsAccumulator >= 1.0f/(PhysicsFPS*game->GameSpeed)) {
        double start = GetTime();
        for (int e = 0; e < End; ++e) {
            std::vector<shared_ptr<Entity>> *array = &Entities[(EntityType)e];
            for (int i = 0; i < array->size(); i++) {
                if (shared_ptr<Entity> entity = array->at(i); entity != nullptr and !entity->ShouldDelete) {
                    entity->PhysicsUpdate(1.0f/PhysicsFPS, g + (GetTime() - start));
                }
            }
        }
        PhysicsAccumulator -= 1.0f/(PhysicsFPS*game->GameSpeed);
    }
}

void EntityManager::EntityClear()
{
    for (int e = 0; e < End; e++) {
        std::vector<shared_ptr<Entity>> *array = &Entities[(EntityType) e];
        int old_size = array->size();
        std::erase_if(*array, [this](shared_ptr<Entity>& e) {
            if (e && e->ShouldDelete) {
                e->OnDeath();
                e->OnDelete();
                if (e != game->MainPlayer) {
                    e.reset();
                }
                return true;
            }
            return false;
        });
        if (game->DebugDraw) {
            std::string f = "ENTITY CATEGORY: " + to_string(e) + ", OLD SIZE: " + to_string(old_size) + ", NEW SIZE: " + to_string(array->size());
            DrawText(f.c_str(), 800+game->GameCamera.RaylibCamera.target.x, 50 + 10*e +game->GameCamera.RaylibCamera.target.y, 10, WHITE);
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
    for (int e = 0; e < End; e++) {
        std::vector<shared_ptr<Entity>>* array = &Entities[(EntityType)e];
        for (int i = 0; i < array->size(); i++) {
            if (shared_ptr<Entity>& entity = array->at(i); entity != nullptr) {
                entity->OnDelete();
                entity.reset();
            }
        }
        array->clear();
    }
}

void EntityManager::Quit()
{
    Clear();
}
