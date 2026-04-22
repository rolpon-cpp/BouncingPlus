//
// Created by lalit on 4/20/2026.
//

#ifndef BOUNCINGPLUS_EFFECTS_H
#define BOUNCINGPLUS_EFFECTS_H
#include <memory>
#include <vector>

class Entity;
class Game;

struct Effect
{
    void Affect();
};

class Effects
{
    std::weak_ptr<Entity> Owner;
    std::vector<Effect> CurrentEffects;
    Game* game;
    Effects(std::shared_ptr<Entity> Owner, Game &game);
    Effects();
    void Update();
    virtual ~Effects();
};


#endif //BOUNCINGPLUS_EFFECTS_H