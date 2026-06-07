//
// Created by lalit on 2/2/2026.
//

#include "CatchBehavior.h"

#include <iostream>
#include <raymath.h>

#include "../Enemy.h"
#include "../../../../game/Game.h"

CatchBehavior::CatchBehavior()
{
    ResetDirection();
}

CatchBehavior::CatchBehavior(Enemy& Owner, Game& game) : EnemyBehavior(Owner,game)
{
    BehaviorType = CatchBehaviorType;
    ResetDirection();
}

CatchBehavior::~CatchBehavior()
{
}

void CatchBehavior::ResetDirection()
{
    // changes the direction
    CurrentDir = Vector2Normalize({GetRandomValue(1,2) == 1 ? -1.0f : 1.0f, GetRandomValue(1,2) == 1 ? -1.0f : 1.0f});
    CurrentSpeed = GetRandomValue(700, 1200);
}

void CatchBehavior::Update()
{
    // unequips any weapon, sets everything to proper values
    Owner->Speed = CurrentSpeed;
    Owner->Movement = CurrentDir;
    Owner->Rotation = 180 - (Vector2LineAngle({Owner->BoundingBox.x, Owner->BoundingBox.y}, LastPosition) * RAD2DEG);

    if (game->GetGameTime() - LastChangedDirCooldown >= 5.0f && !game->RayCastPoint(Owner->GetCenter(), Owner->GetCenter() + (CurrentDir * 100.0f)).HitAir)
    {
        ResetDirection();
        LastChangedDirCooldown= game->GetGameTime();
    }

    // player damage check
    float distance = Vector2Distance({Owner->BoundingBox.x, Owner->BoundingBox.y}, {game->MainPlayer->BoundingBox.x, game->MainPlayer->BoundingBox.y});
    if (distance < 600 && game->GetGameTime() - AttackCooldown >= 1.5f)
    {
        CurrentDir = Vector2Normalize(Vector2Subtract({game->MainPlayer->BoundingBox.x, game->MainPlayer->BoundingBox.y}, {Owner->BoundingBox.x, Owner->BoundingBox.y}));
        if (distance < 100)
        {
            Owner->DamageOther(game->MainPlayer, 10);
            game->GameCamera.ShakeCamera(0.15f);
            game->GameSounds.PlayGameSound("dash_hit", 0.1f);
            AttackCooldown = game->GetGameTime();
        }

    }

    // updating enemy behavior and last properties

    EnemyBehavior::Update();
    LastPosition = {Owner->BoundingBox.x, Owner->BoundingBox.y};
    LastTime = game->GetGameTime();
}
