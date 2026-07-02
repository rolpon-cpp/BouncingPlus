//
// Created by Rolpon on 8/26/2025.
//

#include "Entity.h"
#include <raylib.h>
#include <raymath.h>
#include <nlohmann/json.hpp>
#include "../game/Game.h"
#include "subentities/enemy/Enemy.h"
#include "subentities/player/Player.h"
#include "../game/managers/CameraManager.h"
#include "../game/managers/ParticleManager.h"
#include "../level/tiles/TileManager.h"
#include "../game/managers/GameModeManager.h"
#include "../game/core/SharedManager.h"

void Entity::Initialize(Texture2D& Texture, Rectangle BoundingBox, float Speed)
{
    this->Texture = &Texture;
    this->BoundingBox = BoundingBox;
    this->Movement = Vector2{0, 0};
    this->Speed = Speed;
    this->Rotation = 0;
    this->FrameStackSpeed = 0.0f;
    this->ShouldDelete = false;
    this->CollisionsEnabled = true;
    this->MaxHealth = 100;
    this->WeaponWeightSpeedMultiplier = 1;
    this->Health = this->MaxHealth;
    this->Type = DefaultEntityType;
    this->Priority = NearbyPlayerPriorityType;
    this->VelocityMovement = {0, 0};
    this->VelocityPower = 0;
    this->LastVelBounceCoord = {0, 0};
    this->EntityColor = WHITE;
}

Entity::Entity(Texture2D& Texture, Rectangle BoundingBox, float Speed, Game& game)
{
    this->game = &game;
    Initialize(Texture, BoundingBox, Speed);
}

Entity::Entity()
{
}

Entity::~Entity()
{
}

Vector2 Entity::GetCenter()
{
    return Vector2{BoundingBox.x + BoundingBox.width / 2, BoundingBox.y + BoundingBox.height / 2};
}

float Entity::GetSpeed()
{
    return (Speed * WeaponWeightSpeedMultiplier) + FrameStackSpeed;
}

void Entity::OnWallVelocityBump(float Power)
{
}

void Entity::DamageOther(std::shared_ptr<Entity> entity, float Damage, std::shared_ptr<Entity> owner, float HealthGain)
{
    if (owner == nullptr)
        owner = shared_from_this();

    if (entity->ShouldDelete)
        return;
    if (entity->Health <= 0)
        return;
    if (owner->ShouldDelete)
        return;
    if (owner->Health <= 0)
        return;

    if (entity->Type == PlayerEntityType)
        game->MainPlayer->LogicProcessor.DamageNotification(owner->GetCenter());

    if (entity->Type == PlayerEntityType && game->MainPlayer->isInvincible)
        return;

    int ParticleAmount = GetRandomValue(9, 15) * (Damage / 100.0f);
    if (ParticleAmount > 30)
        ParticleAmount = 30;
    game->GameParticles->ParticleEffect({
                                            entity->GetCenter(),
                                            GetRandomValue(2500, 3000) / 7.5f,
                                            RED,
                                            750.0f,
                                            3.5f,
                                            2.0f,
                                            ColorLerp(RED, ORANGE, GetRandomValue(1, 100) / 100.0f)
                                        },
                                        (180.0f - Vector2LineAngle(entity->GetCenter(), owner->GetCenter()) * RAD2DEG),
                                        15, ParticleAmount);

    if (entity->Type == EnemyEntityType)
    {
        // if victim is enemy, check for armor damage
        shared_ptr<Enemy> enemy = dynamic_pointer_cast<Enemy>(entity);
        if (owner->Type == PlayerEntityType)
            enemy->AngeredRangeBypassTimer = enemy->AngeredRangeBypassTimerMax;
        if (enemy->Armor <= 0)
            enemy->Health -= Damage;
        else
            enemy->Armor -= Damage;
    }
    else // if they are normal, just damage them normally
        entity->Health -= Damage;

    // if entity dies, give owner health and increase kill count for player
    if (entity->Health <= 0)
    {
        entity->ShouldDelete = true;
        if (owner->Health > 0)
        {
            if (owner->Type != PlayerEntityType)
                owner->Health += HealthGain;
            else if (!game->MainPlayer->isInvincible)
                owner->Health += HealthGain * game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()][
                    "player"]["weapon_health_gain_buff"].get<float>();
        }
        if (owner->Type == PlayerEntityType)
            game->MainPlayer->Kills += 1;
    }
}

void Entity::PhysicsUpdate(float DeltaTime, double time)
{
    if (abs(VelocityPower) > 0)
    {
        VelocityPower += 4500.0f * DeltaTime * (VelocityPower > 0 ? -1 : 1);
        if (abs(VelocityPower) < 5)
            VelocityPower = 0;
    }

    Vector2 EntityMovement = Vector2Normalize(Movement);
    Vector2 EntityVelocityMovement = Vector2Normalize(VelocityMovement);

    Vector2 FinalEntityMovement = Vector2{
        EntityMovement.x * GetSpeed(), EntityMovement.y * GetSpeed()
    };

    Vector2 FinalVelocityMovement = Vector2{
        EntityVelocityMovement.x * VelocityPower, EntityVelocityMovement.y * VelocityPower
    };

    if (!CollisionsEnabled)
    {
        BoundingBox.x += FinalVelocityMovement.x * DeltaTime;
        BoundingBox.y += FinalVelocityMovement.y * DeltaTime;
        BoundingBox.x += EntityMovement.x * DeltaTime;
        BoundingBox.y += EntityMovement.y * DeltaTime;
    } else
    {
        if (FinalEntityMovement.x != 0.0f || FinalEntityMovement.y != 0.0f)
        {
            auto tile_types = std::vector<TileType>();
            if (Type == EnemyEntityType)
                tile_types.push_back(EnemyWallTileType);

            BoundingBox.x += FinalEntityMovement.x * DeltaTime;
            CollisionData XCollision = game->GameTiles->IsColliding(BoundingBox, tile_types);
            if (XCollision.HitWall)
            {
                BoundingBox.x -= FinalEntityMovement.x * DeltaTime;
            }

            BoundingBox.y += FinalEntityMovement.y * DeltaTime;
            CollisionData YCollision = game->GameTiles->IsColliding(BoundingBox, tile_types);
            if (YCollision.HitWall)
            {
                BoundingBox.y -= FinalEntityMovement.y * DeltaTime;
            }
        }

        if (FinalVelocityMovement.x != 0.0f || FinalVelocityMovement.y != 0.0f)
        {
            BoundingBox.x += FinalVelocityMovement.x * DeltaTime;
            CollisionData XCollision = game->GameTiles->IsColliding(BoundingBox, {EnemyWallTileType});
            if (XCollision.HitWall)
            {
                BoundingBox.x -= FinalVelocityMovement.x * DeltaTime;
            }

            BoundingBox.y += FinalVelocityMovement.y * DeltaTime;
            CollisionData YCollision = game->GameTiles->IsColliding(BoundingBox, {EnemyWallTileType});
            if (YCollision.HitWall)
            {
                BoundingBox.y -= FinalVelocityMovement.y * DeltaTime;
            }

            if (XCollision.Normal.x != 0.0f && XCollision.Normal.y != 0.0f)
            {
                VelocityMovement = Vector2Reflect(VelocityMovement, XCollision.Normal);
                VelocityPower -= VelocityPower / 10.0f;
            } else if (YCollision.Normal.x != 0.0f && YCollision.Normal.y != 0.0f)
            {
                VelocityMovement = Vector2Reflect(VelocityMovement, YCollision.Normal);
                VelocityPower -= VelocityPower / 10.0f;
            }
        }
    }

    FrameStackSpeed = 0.0f;
}

bool Entity::IsVisible()
{
    return IsVisible(BoundingBox);
}

bool Entity::IsVisible(Rectangle bbox)
{
    Vector2 MyPosOnScreen = GetWorldToScreen2D(Vector2{bbox.x + bbox.width / 2, bbox.y + bbox.height / 2},
                                               game->GameCamera->RaylibCamera);
    return MyPosOnScreen.x > -BoundingBox.width &&
        MyPosOnScreen.x < GetRenderWidth() &&
        MyPosOnScreen.y > -BoundingBox.height &&
        MyPosOnScreen.y < GetRenderHeight();
}

void Entity::Update()
{
    if (Health <= 0)
    {
        ShouldDelete = true;
    }

    bool is_visible = IsVisible();
    if (is_visible && Texture != nullptr)
    {
        DrawTexturePro(*Texture, Rectangle{
                           0, 0, static_cast<float>(Texture->width), static_cast<float>(Texture->height)
                       },
                       Rectangle{
                           BoundingBox.x + BoundingBox.width / 2, BoundingBox.y + BoundingBox.height / 2,
                           BoundingBox.width,
                           BoundingBox.height
                       }, Vector2{BoundingBox.width / 2, BoundingBox.height / 2}, Rotation, EntityColor);
    }
    if (game->DebugDraw && is_visible)
        DrawRectangleRec({
                             BoundingBox.x,
                             BoundingBox.y,
                             BoundingBox.width,
                             BoundingBox.height
                         }, ColorAlpha(PINK, 0.5f));
}

void Entity::OnDelete()
{
}

void Entity::OnDeath()
{
}