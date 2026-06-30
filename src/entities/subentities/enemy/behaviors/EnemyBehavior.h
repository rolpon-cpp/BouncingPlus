//
// Created by Rolpon on 1/31/2026.
//

#ifndef BOUNCINGPLUS_ENEMYBEHAVIOR_H
#define BOUNCINGPLUS_ENEMYBEHAVIOR_H
#include "EnemyBehaviorType.h"

class Enemy;
class Game;

// default enemy behavior class
class EnemyBehavior
{
public:
    Enemy* Owner;
    Game* game;
    EnemyBehaviorType BehaviorType;
    EnemyBehavior();
    EnemyBehavior(Enemy& Owner, Game& game);
    virtual ~EnemyBehavior();
    virtual void Update();
};


#endif //BOUNCINGPLUS_ENEMYBEHAVIOR_H