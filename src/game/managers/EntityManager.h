//
// Created by Lalit on 10/24/2025.
//

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H
#include <memory>
#include <unordered_map>
#include <vector>
#include "../../entities/EntityType.h"

class Game;
class Entity;

class EntityManager {
    void EntityUpdate();
    void EntityPhysicsUpdate();
    void EntityClear();
    float PhysicsAccumulator;
    Game* game;
public:
    std::unordered_map<EntityType, std::vector<std::shared_ptr<Entity>>> Entities;
    float PhysicsFPS;
    EntityManager();
    EntityManager(Game &game);
    ~EntityManager();
    void AddEntity(EntityType Type, std::shared_ptr<Entity> EntityToAdd);
    void Update();
    void Clear();
    void Quit();
};



#endif //ENTITYMANAGER_H
