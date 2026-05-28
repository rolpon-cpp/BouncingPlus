//
// Created by lalit on 10/28/2025.
//

#ifndef BOUNCINGPLUS_SPAWNER_H
#define BOUNCINGPLUS_SPAWNER_H
#include "../../Entity.h"
#include <raylib.h>
class Spawner : public Entity {
public:
    Vector2 StartPos;
    Vector2 RandPoint;
    int EntitiesSpawned;
    float PosMultiplier;
    float RotMultiplier;
    double SpawnTimer;
    double SpawnerIsActive;
    double SpawnerRageCooldown;

    float EnemyDifficulty;
    double SpawnCooldown;

    double Timer;
    float DistF;
    float RandomNumbers[8];
    Spawner(Game &game, float MyX, float MyY);
    Spawner();
    ~Spawner()override;
    void Update()override;
    void Render();
};


#endif //BOUNCINGPLUS_SPAWNER_H