//
// Created by Rolpon on 10/19/2025.
//

#include "ParticleManager.h"
#include "../../entities/subentities/player/Player.h"
#include "../../level/tiles/TileManager.h"
#include "../../game/managers/SoundManager.h"
#include "../../game/managers/EntityManager.h"
#include "../../game/managers/CameraManager.h"
#include "../../game/core/SharedManager.h"
#include "../../level/tiles/TileType.h"
#include <iostream>
#include <raymath.h>

#include "../Game.h"

ParticleManager::ParticleManager()
{
}

ParticleManager::ParticleManager(Game* game)
{
    this->game = game;
    Particles = std::vector<Particle>();
}

void ParticleManager::ParticleEffect(ParticleData Data, float Angle, int AngleRange, int Amount, EffectData PEffectData)
{
    for (int i = 0; i < Amount; i++)
    {
        Particle newParticle = {
            Data,
            Data.StartPosition,
            {0, 0},
            Data.StartVelocity + GetRandomValue(-50, 50),
            game->GetGameTime(),
            Data.StartColor
        };
        newParticle.Position += {(float)GetRandomValue(-15, 15), (float)GetRandomValue(-15, 15)};
        newParticle.Data.TargetColor = ColorBrightness(Data.TargetColor, GetRandomValue(-30, 15) / 100.0f);
        newParticle.Data.Lifetime += GetRandomValue(-35, 35) / 100.0f;
        newParticle.Data.Size -= (float)GetRandomValue(-2, 2);
        float RandomAngle = Angle - (AngleRange / 2) + GetRandomValue(0, AngleRange);
        float X = cos(RandomAngle * (2 * PI / 360)) * 100;
        float Y = sin(RandomAngle * (2 * PI / 360)) * 100;
        newParticle.Target = Vector2Normalize({X, Y});
        newParticle.Effect = PEffectData;
        Particles.push_back(newParticle);
    }
}

void ParticleManager::Clear()
{
    Particles.clear();
}

void ParticleManager::Update()
{
    BeginBlendMode(BLEND_ADDITIVE);

    std::erase_if(Particles, [this](Particle& p)
    {
        double Percent = (game->GetGameTime() - p.SpawnTime) / p.Data.Lifetime;
        bool sd = Percent >= 1.0f;
        if (Vector2Distance(p.Position, game->MainPlayer->GetCenter()) >= 3000 && p.Effect.Type == DEFAULT)
        {
            sd = true;
            return sd;
        }
        if (!sd && p.Effect.Type == DEFAULT && Particles.size() > game->GameShared->CosmeticParticleLimit)
        {
            sd = true;
            return sd;
        }

        if (!sd)
        {
            p.Velocity -= p.Data.VelocitySlowdown * game->GetGameDeltaTime();
            if (p.Velocity <= 0)
                p.Velocity = 0;

            int tile = game->GameTiles->TileTypes[game->GameTiles->GetTileAtWorldCoords(p.Position)];
            if (tile != WallTileType && tile != EnemyWallTileType)
                p.Position += p.Target * p.Velocity * game->GetGameDeltaTime();

            p.ParticleColor = ColorLerp(p.ParticleColor, p.Data.TargetColor, std::min((float)Percent / 0.2f, 1.0f));

            if (p.Effect.Type != DEFAULT)
            {
                for (std::shared_ptr entity : game->GameEntities->Entities[EnemyType])
                {
                    if (entity != nullptr && entity != p.Effect.Owner.lock() && !entity->ShouldDelete)
                    {
                        auto enemy = dynamic_pointer_cast<Enemy>(entity);
                        if (CheckCollisionCircleRec(p.Position, p.Data.Size / 2.0f, entity->BoundingBox))
                        {
                            if (auto o = p.Effect.Owner.lock())
                            {
                                if (p.Effect.Type == BURNING && o->Type == PlayerType && !enemy->MainEffectsSystem.
                                    HasEffect(BURNING))
                                {
                                    game->MainPlayer->LogicProcessor.IncreaseScore("Enemy Burn", 15.0f, RED);
                                }
                            }
                            enemy->MainEffectsSystem.AddEffect(p.Effect);
                            sd = true;
                            return sd;
                        }
                    }
                }
                if (game->MainPlayer != p.Effect.Owner.lock() && CheckCollisionCircleRec(
                    p.Position, p.Data.Size / 2.0f, game->MainPlayer->BoundingBox))
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
        Particle& p = Particles[i];
        Vector2 ScreenPos = GetWorldToScreen2D(p.Position, game->GameCamera->RaylibCamera);

        double Percent = (game->GetGameTime() - p.SpawnTime) / p.Data.Lifetime;

        if (ScreenPos.x >= 0 && ScreenPos.x < GetRenderWidth() && ScreenPos.y >= 0 && ScreenPos.y < GetRenderHeight())
        {
            DrawRectanglePro({
                                 p.Position.x,
                                 p.Position.y,
                                 p.Data.Size, p.Data.Size
                             },
                             {p.Data.Size / 2, p.Data.Size / 2},
                             (game->GetGameTime() - p.SpawnTime) * 100 * (1 - Percent),
                             ColorAlpha(p.ParticleColor, Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)
            );

            DrawCircleGradient(
                p.Position.x,
                p.Position.y,
                p.Data.Size / 1.1f,
                ColorAlpha(p.Data.TargetColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)),
                ColorAlpha(p.ParticleColor, 0.4f * (Percent >= 0.8f ? 1.0f - (Percent - .8f) / .2f : 1.0f)));
        }
    }
    EndBlendMode();

    if (game->DebugDraw)
    {
        float siz = MeasureText((std::string("particles: ") + std::to_string(Particles.size())).c_str(), 50.0f);
        DrawText((std::string("particles: ") + std::to_string(Particles.size())).c_str(),
                 (int)(game->MainPlayer->GetCenter().x - siz / 2.0f), (int)(game->MainPlayer->GetCenter().y + 100.0f),
                 50.0f, PURPLE);
    }
}

void ParticleManager::Quit()
{
    Clear();
}