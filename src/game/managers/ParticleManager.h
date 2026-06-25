//
// Created by Rolpon on 10/19/2025.
//

#ifndef BOUNCINGPLUS_PARTICLESYSTEM_H
#define BOUNCINGPLUS_PARTICLESYSTEM_H
#include <vector>
#include <raylib.h>

#include "../../entities/systems/Effects.h"

class Game;

struct ParticleData {
    Vector2 StartPosition;
    float StartVelocity;
    Color StartColor;
    float VelocitySlowdown;
    float Size;
    double Lifetime;
    Color TargetColor;
};

struct Particle {
    ParticleData Data;
    Vector2 Position;
    Vector2 Target;
    float Velocity;
    double SpawnTime;
    Color ParticleColor;
    EffectData Effect;
};

class ParticleManager {
    Game *game;
    std::vector<Particle> Particles;
    public:
    ParticleManager();
    ParticleManager(Game *game);
    void ParticleEffect(ParticleData Data, float Angle, int AngleRange, int Amount, EffectData PEffectData = {DEFAULT, std::weak_ptr<Entity>(), 0, 0, 0});
    void Clear();
    void Update();
    void Quit();
};

#endif //BOUNCINGPLUS_PARTICLESYSTEM_H