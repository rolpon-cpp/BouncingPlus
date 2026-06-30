//
// Created by Rolpon on 1/31/2026.
//

#include "EnemyBehavior.h"

#include <iostream>

#include "../Enemy.h"
#include "../../../../game/Game.h"

EnemyBehavior::EnemyBehavior()
{
}

EnemyBehavior::~EnemyBehavior()
{
}

EnemyBehavior::EnemyBehavior(Enemy& Owner, Game& game)
{
    BehaviorType = DefaultEnemyBehaviorType;
    this->Owner = &Owner;
    this->game = &game;
}

void EnemyBehavior::Update()
{
}