//
// Created by lalit on 10/19/2025.
//

#include "ParticleManager.h"

#include <iostream>
#include <raymath.h>

#include "../Game.h"

ParticleManager::ParticleManager() {
}

ParticleManager::ParticleManager(Game &game) {
    this->game = &game;
    Particles = std::vector<Particle>();
    ParticlesTexture = LoadRenderTexture(1,1);
    #ifndef PLATFORM_WEB
        ParticleRenderLimit = 175;
    #else
        ParticleRenderLimit = 50;
    #endif
}

void ParticleManager::ParticleEffect(ParticleData Data, float Angle, int AngleRange, int Amount) {
    for (int i = 0; i < Amount; i++) {
        Particle newParticle = {
            Data,
            Data.StartPosition,
            {0, 0},
            Data.StartVelocity + GetRandomValue(-50,50),
            game->GetGameTime(),
            Data.StartColor
        };
        newParticle.Position += {(float) GetRandomValue(-15, 15), (float) GetRandomValue(-15, 15)};
        newParticle.Data.TargetColor = ColorBrightness(Data.TargetColor, GetRandomValue(-30, 15) / 100.0f);
        newParticle.Data.Lifetime += GetRandomValue(-35, 35) / 100.0f;
        newParticle.Data.Size -= (float) GetRandomValue(-2, 2);
        float RandomAngle = Angle - (AngleRange / 2) + GetRandomValue(0, AngleRange);
        float X = cos(RandomAngle * (2 * PI / 360))*100;
        float Y = sin(RandomAngle * (2 * PI / 360))*100;
        newParticle.Target = Vector2Normalize({X, Y});
        Particles.push_back(newParticle);
    }
}

void ParticleManager::Clear() {
    if (IsRenderTextureValid(ParticlesTexture))
        UnloadRenderTexture(ParticlesTexture);
    ParticlesTexture = LoadRenderTexture(1,1);
    Particles.clear();
}

void ParticleManager::Update() {

    if (ParticlesTexture.texture.width != GetRenderWidth() || ParticlesTexture.texture.height != GetRenderHeight()) {
        if (IsRenderTextureValid(ParticlesTexture))
            UnloadRenderTexture(ParticlesTexture);
        ParticlesTexture = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    game->GameCamera.BeginRenderTexture(ParticlesTexture);

    ClearBackground(BLANK);

    BeginBlendMode(BLEND_ADDITIVE);

    int ParticlesToProcess = Particles.size();
    if (ParticlesToProcess > ParticleRenderLimit)
        ParticlesToProcess = ParticleRenderLimit;

    for (int i = ParticlesToProcess - 1; i >= 0; i--)
    {
        Particle &p = Particles[i];

        double Percent = (game->GetGameTime() - p.SpawnTime) / p.Data.Lifetime;

        p.Velocity -= p.Data.VelocitySlowdown * game->GetGameDeltaTime();
        if (p.Velocity <= 0)
            p.Velocity = 0;

        int tile_x = (int)(p.Position.x / game->GameTiles.TileSize);
        int tile_y = (int)(p.Position.y / game->GameTiles.TileSize);

        if (game->GameTiles.TileTypes[game->GameTiles.GetTileAt(tile_x, tile_y)] != WallTileType)
            p.Position += p.Target * p.Velocity * game->GetGameDeltaTime();

        p.ParticleColor = ColorLerp(p.ParticleColor, p.Data.TargetColor, min((float)Percent / 0.2f, 1.0f));

        DrawRectanglePro({p.Position.x - game->GameCamera.RaylibCamera.target.x,
            p.Position.y - game->GameCamera.RaylibCamera.target.y,
            p.Data.Size,p.Data.Size},
            {p.Data.Size/2, p.Data.Size/2},
            (game->GetGameTime() - p.SpawnTime) * 100 * (1-Percent),
            ColorAlpha(p.ParticleColor, Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)
            );

        Vector2 ScreenPos = {p.Position.x - game->GameCamera.RaylibCamera.target.x,
            p.Position.y - game->GameCamera.RaylibCamera.target.y};

        DrawCircleGradient(
        ScreenPos.x, ScreenPos.y,
            p.Data.Size/1.1f,
            ColorAlpha(p.Data.TargetColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)),
            ColorAlpha(p.ParticleColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)));

        if (Percent >= 1.0f || (ScreenPos.x < 0 || ScreenPos.x > GetRenderWidth() || ScreenPos.y < 0 || ScreenPos.y > GetRenderHeight()))
            Particles.erase(Particles.begin() + i);
    }
    EndBlendMode();

    game->GameCamera.EndRenderTexture();

    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(ParticlesTexture.texture,{0,0,(float)ParticlesTexture.texture.width, (float)-ParticlesTexture.texture.height}, {
        game->GameCamera.RaylibCamera.target.x, game->GameCamera.RaylibCamera.target.y, (float)ParticlesTexture.texture.width, (float)ParticlesTexture.texture.height
    }, {0, 0}, 0, WHITE);
    EndBlendMode();

}

void ParticleManager::Quit() {
    if (IsRenderTextureValid(ParticlesTexture))
        UnloadRenderTexture(ParticlesTexture);
}
