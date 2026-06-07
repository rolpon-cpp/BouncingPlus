//
// Created by lalit on 10/19/2025.
//

#ifndef BOUNCINGPLUS_PARTICLESYSTEM_H
#define BOUNCINGPLUS_PARTICLESYSTEM_H
#include <vector>
#include <raylib.h>

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
};

class ParticleManager {
    Game *game;
    std::vector<Particle> Particles;
    RenderTexture ParticlesTexture;
    int ParticleRenderLimit;
    public:
    ParticleManager();
    ParticleManager(Game &game);
    void ParticleEffect(ParticleData Data, float Angle, int AngleRange, int Amount);
    void Clear();
    void Update();
    void Quit();
};

#endif //BOUNCINGPLUS_PARTICLESYSTEM_H