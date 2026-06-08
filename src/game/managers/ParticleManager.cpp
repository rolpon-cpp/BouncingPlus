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
        ParticleRenderLimit = 500;
    #else
        ParticleRenderLimit = 175;
    #endif
}

void ParticleManager::ParticleEffect(ParticleData Data, float Angle, int AngleRange, int Amount, EffectData PEffectData) {
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
        newParticle.Effect = PEffectData;
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

    int ParticlesRendered = 0;

    std::erase_if(Particles, [this](Particle& p)
    {
        double Percent = (game->GetGameTime() - p.SpawnTime) / p.Data.Lifetime;
        bool sd = Percent >= 1.0f;
        if (Vector2Distance(p.Position, game->MainPlayer->GetCenter()) >= 1000 && p.Effect.Type == DEFAULT)
            sd = true;

        if (!sd)
        {
            p.Velocity -= p.Data.VelocitySlowdown * game->GetGameDeltaTime();
            if (p.Velocity <= 0)
                p.Velocity = 0;

            int tile = game->GameTiles.TileTypes[game->GameTiles.GetTileAtWorldCoords(p.Position)];
            if (tile != WallTileType && tile != EnemyWallTileType)
                p.Position += p.Target * p.Velocity * game->GetGameDeltaTime();

            p.ParticleColor = ColorLerp(p.ParticleColor, p.Data.TargetColor, min((float)Percent / 0.2f, 1.0f));

            if (p.Effect.Type != DEFAULT)
            {
                for (shared_ptr entity : game->GameEntities.Entities[EnemyType]) {
                    auto enemy = dynamic_pointer_cast<Enemy>(entity);
                    if (entity != nullptr && entity != p.Effect.Owner.lock() && !entity->ShouldDelete) {
                        if (CheckCollisionCircleRec(p.Position, p.Data.Size/2.0f, entity->BoundingBox))
                        {
                            enemy->MainEffectsSystem.AddEffect(p.Effect);
                            sd = true;
                            return sd;
                        }
                    }
                }
                if (game->MainPlayer != p.Effect.Owner.lock() && CheckCollisionCircleRec(p.Position, p.Data.Size/2.0f, game->MainPlayer->BoundingBox))
                {
                    game->MainPlayer->MainEffectsSystem.AddEffect(p.Effect);
                    sd = true;
                    return sd;
                }
            }
        }
        return sd;
    });

    for (int i = 0; i < Particles.size(); i++)
    {
        Particle &p = Particles[i];
        Vector2 ScreenPos = GetWorldToScreen2D(p.Position, game->GameCamera.RaylibCamera);

        double Percent = (game->GetGameTime() - p.SpawnTime) / p.Data.Lifetime;

        if (ParticlesRendered < ParticleRenderLimit && ScreenPos.x >= 0 && ScreenPos.x < GetRenderWidth() && ScreenPos.y >= 0 && ScreenPos.y < GetRenderHeight())
        {
            DrawRectanglePro({
                p.Position.x - game->GameCamera.RaylibCamera.target.x,
                p.Position.y - game->GameCamera.RaylibCamera.target.y,
                p.Data.Size,p.Data.Size},
                {p.Data.Size/2, p.Data.Size/2},
                (game->GetGameTime() - p.SpawnTime) * 100 * (1-Percent),
                ColorAlpha(p.ParticleColor, Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)
                );

            DrawCircleGradient(
            p.Position.x - game->GameCamera.RaylibCamera.target.x,
                p.Position.y - game->GameCamera.RaylibCamera.target.y,
                p.Data.Size/1.1f,
                ColorAlpha(p.Data.TargetColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)),
                ColorAlpha(p.ParticleColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)));

            ParticlesRendered++;
        }
    }
    EndBlendMode();

    game->GameCamera.EndRenderTexture();

    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(ParticlesTexture.texture,{0,0,(float)ParticlesTexture.texture.width, (float)-ParticlesTexture.texture.height}, {
        game->GameCamera.RaylibCamera.target.x, game->GameCamera.RaylibCamera.target.y, (float)ParticlesTexture.texture.width, (float)ParticlesTexture.texture.height
    }, {0, 0}, 0, WHITE);
    EndBlendMode();

    if (game->DebugDraw)
    {
        float siz = MeasureText((string("particles: ") +to_string(Particles.size())).c_str(), 50.0f);
        DrawText((string("particles: ") +to_string(Particles.size())).c_str(), (int)(game->MainPlayer->GetCenter().x - siz/2.0f), (int)(game->MainPlayer->GetCenter().y + 100.0f), 50.0f, PURPLE);
    }

}

void ParticleManager::Quit() {
    if (IsRenderTextureValid(ParticlesTexture))
        UnloadRenderTexture(ParticlesTexture);
}
