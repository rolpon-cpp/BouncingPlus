//
// Created by lalit on 2/2/2026.
//

#ifndef BOUNCINGPLUS_CATCHBEHAVIOR_H
#define BOUNCINGPLUS_CATCHBEHAVIOR_H
#include <raylib.h>

#include "EnemyBehavior.h"


class CatchBehavior : public EnemyBehavior
{
    void ResetDirection();
public:
    Vector2 CurrentDir = {0, 0}; // current enemy movement direction
    float CurrentSpeed = 0; // current enemy speed
    Vector2 LastPosition = {0, 0}; // last frame position
    double LastTime = 0; // last frame time
    double AttackCooldown = 0; // attack cooldown
    CatchBehavior();
    CatchBehavior(Enemy& Owner, Game& game);
    ~CatchBehavior();
    void Update();
};


#endif //BOUNCINGPLUS_CATCHBEHAVIOR_H