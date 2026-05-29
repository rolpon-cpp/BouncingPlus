//
// Created by lalit on 4/20/2026.
//

#ifndef BOUNCINGPLUS_EFFECTS_H
#define BOUNCINGPLUS_EFFECTS_H
#include <memory>
#include <vector>

#include "raylib.h"

class Entity;
class Game;

class Effect
{
public:
    double Duration;
    double ImpactTime;
    Effect(double Duration, double ImpactTime);
    Effect(double ImpactTime);
    virtual void Update(std::shared_ptr<Entity> Owner);
    virtual ~Effect();
};

class Burning : public Effect
{
public:
    float Damage;
    double LastDidFireParticle;
    Burning(double ImpactTime);
    Burning(float Damage, double Duration, double ImpactTime);
    void Update(std::shared_ptr<Entity> Owner);
    ~Burning();
};

class Swiftness : public Effect
{
public:
    float SpeedInc;
    double LastDidParticle;
    Vector2 LastPos;
    Swiftness(double ImpactTime);
    Swiftness(float SpeedInc, double Duration, double ImpactTime);
    void Update(std::shared_ptr<Entity> Owner);
    ~Swiftness();
};

class Effects
{
public:
    void RemoveOldestEffect();
    std::weak_ptr<Entity> Owner;
    std::vector<Effect*> CurrentEffects;
    Game* game;
    Effects(std::shared_ptr<Entity> Owner, Game &game);
    Effects();
    virtual ~Effects() = default;
    void AddEffect(Effect* effect);
    void Update();
    void Cleanup();
};


#endif //BOUNCINGPLUS_EFFECTS_H