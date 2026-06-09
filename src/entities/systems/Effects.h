//
// Created by lalit on 4/20/2026.
//

#ifndef BOUNCINGPLUS_EFFECTS_H
#define BOUNCINGPLUS_EFFECTS_H
#include <map>
#include <memory>
#include <vector>

#include "raylib.h"

class Entity;
class Game;

enum EffectType
{
    DEFAULT, BURNING, SWIFTNESS
};

struct EffectData
{
    EffectType Type = DEFAULT;
    std::weak_ptr<Entity> Owner; // make sure this isnt a nullptr before using!!!
    float Power = 0;
    float Reward = 0;
    float Duration = 0;
};

class Effect
{
public:
    EffectType Type = DEFAULT;
    double Duration;
    double ImpactTime;
    Effect(double Duration, double ImpactTime);
    Effect(double ImpactTime);
    virtual void Update(std::shared_ptr<Entity> ImpactedEntity);
    virtual ~Effect();
};

class Burning : public Effect
{
public:
    float Damage;
    Color OG_Color;
    std::weak_ptr<Entity> Owner = std::weak_ptr<Entity>();
    double OwnerReward;
    int SmokeParticleCount;
    double LastDidFireParticle;
    double LastDidSFX;
    float GradientProg;
    Burning(double ImpactTime);
    Burning(float Damage, double Duration, double ImpactTime);
    void SetOwner(std::shared_ptr<Entity> Owner);
    void SetOwnerReward(double Reward);
    void Update(std::shared_ptr<Entity> ImpactedEntity);
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
    void Update(std::shared_ptr<Entity> ImpactedEntity);
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
    bool HasEffect(EffectType Type);
    void AddEffect(Effect* effect);
    void AddEffect(EffectData type);
    void Update();
    void Cleanup();
};


#endif //BOUNCINGPLUS_EFFECTS_H