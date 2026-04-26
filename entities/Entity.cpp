//
// Created by lalit on 8/26/2025.
//

#include "Entity.h"

#include <iostream>

#include "systems/Weapons.h"
#include <ostream>
#include <raylib.h>
#include <raymath.h>
#include <nlohmann/json.hpp>

#include "subentities/Bullet.h"
#include "../game/Game.h"

void Entity::Initialize(Texture2D &Texture, Rectangle BoundingBox, float Speed) {
    this->Texture = &Texture;
    this->BoundingBox = BoundingBox;
    this->Movement = Vector2{0, 0};
    this->Speed = Speed;
    this->Rotation = 0;
    this->ShouldDelete = false;
    this->CollisionsEnabled = true;
    this->MaxHealth = 100;
    this->WeaponWeightSpeedMultiplier = 1;
    this->Health = this->MaxHealth;
    this->Type = DefaultType;
    this->VelocityMovement = {0,0};
    this->VelocityPower = 0;
    this->LastVelBounceCoord = {0, 0};
    this->EntityColor = WHITE;
}

Entity::Entity(Texture2D &Texture, Rectangle BoundingBox, float Speed, Game &game) {
    this->game = &game;
    Initialize(Texture, BoundingBox, Speed);
}

Entity::Entity() {

}

Entity::~Entity() {

}

Vector2 Entity::GetCenter()
{
    return Vector2{BoundingBox.x + BoundingBox.width / 2, BoundingBox.y + BoundingBox.height / 2};
}

float Entity::GetSpeed() {
    return Speed * WeaponWeightSpeedMultiplier;
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
    if (entity->Type == PlayerType && game->MainPlayer->isInvincible)
        return;
    if (owner->ShouldDelete)
        return;
    if (owner->Health <= 0)
        return;
    if (HealthGain < 0)
        HealthGain = Damage;
    if (entity->Type == EnemyType) { // if victim is enemy, check for armor damage
        shared_ptr<Enemy> enemy = dynamic_pointer_cast<Enemy>(entity);
        if (owner->Type == PlayerType)
            enemy->AngeredRangeBypassTimer = enemy->AngeredRangeBypassTimerMax;
        if (enemy->Armor <= 0)
            enemy->Health -= Damage;
        else
            enemy->Armor -= Damage;
    } else { // if they are normal, just damage them normally
        entity->Health -= Damage;
        if (entity->Type == PlayerType)
            game->MainPlayer->LogicProcessor.DamageNotification({owner->BoundingBox.x + owner->BoundingBox.width/2, owner->BoundingBox.y + owner->BoundingBox.height/2});
    }

    // if entity dies, give owner health and increase kill count for player
    if (entity->Health <= 0) {
        entity->ShouldDelete = true;
        if (owner->Health > 0)
        {
            if (owner->Type != PlayerType)
                owner->Health += HealthGain;
            else if (!game->MainPlayer->isInvincible)
                owner->Health += HealthGain * game->LevelData[game->CurrentLevelName]["player"]["weapon_health_gain_buff"].get<float>();
        }
        if (owner->Type == PlayerType)
            game->MainPlayer->Kills += 1;
    }
}

void Entity::PhysicsUpdate(float DeltaTime, double time) {
    if (abs(VelocityPower) > 0) {
        VelocityPower += 4500.0f * DeltaTime * (VelocityPower > 0 ? -1 : 1);
        if (abs(VelocityPower) < 5)
            VelocityPower = 0;
    }
    Vector2 mov = Vector2Normalize(Movement);
    Vector2 vel = Vector2Normalize(VelocityMovement);
    Vector2 FinalMovement = Vector2{(mov.x * GetSpeed()) + (vel.x * VelocityPower), (mov.y * GetSpeed()) + (vel.y * VelocityPower)};
    if (Vector2Distance({0,0}, vel) != 0 && CollisionsEnabled) {
        BoundingBox.x += FinalMovement.x * DeltaTime;
        BoundingBox.y += FinalMovement.y * DeltaTime;
        bool f = false;
        int tile_x = static_cast<int> (BoundingBox.x / game->GameTiles.TileSize);
        int tile_y = static_cast<int> (BoundingBox.y / game->GameTiles.TileSize);
        for (int y = 0; y < 3; y++) {
            if (f)
                break;
            for (int x = 0; x < 3; x++) {
                int curr_tile_x = tile_x + x - 1;
                int curr_tile_y = tile_y + y - 1;
                Vector2 coord = {(float)curr_tile_x, (float)curr_tile_y};
                int tile_id = game->GameTiles.GetTileAt(coord);
                float bbox_x = curr_tile_x * game->GameTiles.TileSize;
                float bbox_y = curr_tile_y * game->GameTiles.TileSize;
                Rectangle bbox = Rectangle{bbox_x, bbox_y, game->GameTiles.TileSize, game->GameTiles.TileSize};
                if (
                    (game->GameTiles.TileTypes[tile_id] == WallTileType || (Type == EnemyType ? game->GameTiles.TileTypes[tile_id] == EnemyWallTileType : false))
                    && CheckCollisionRecs(BoundingBox, bbox) && coord != LastVelBounceCoord) {

                    int dir_hit = -1; // -1 = none, 0 = left, 1 = up, 2 = right, 3 = down
                    int i= 0;
                    int dirs[4] = {0, 0, 0, 0};

                    Rectangle left_rect = bbox;
                    left_rect.x -= bbox.width;

                    Rectangle right_rect = bbox;
                    right_rect.x += bbox.width;

                    Rectangle up_rect = bbox;
                    up_rect.y -= bbox.height;

                    Rectangle down_rect = bbox;
                    down_rect.y += bbox.height;

                    if (CheckCollisionRecs(BoundingBox, left_rect)) {
                        dir_hit = 0;
                        dirs[i] = dir_hit;
                        i += 1;
                    }

                    if (CheckCollisionRecs(BoundingBox, right_rect)) {
                        dir_hit = 2;
                        dirs[i] = dir_hit;
                        i += 1;
                    }
                    if (CheckCollisionRecs(BoundingBox, up_rect)) {
                        dir_hit = 1;
                        dirs[i] = dir_hit;
                        i += 1;
                    }
                    if (CheckCollisionRecs(BoundingBox, down_rect)) {
                        dir_hit = 3;
                        dirs[i] = dir_hit;
                        i += 1;
                    }

                    bool negate = true;

                    float eRotation = std::atan2(VelocityMovement.y, VelocityMovement.x) * (180.0f/3.141592653589793238463f);

                    if (i < 2 | dirs[0] == dirs[1]) {
                        if (dir_hit == 1 || dir_hit == 3) {
                            eRotation = -eRotation;
                        } else {
                            eRotation = 180 - eRotation;
                        }
                    } else {
                        negate = false;
                    }

                    float X = -cos(eRotation * (2 * PI / 360));
                    float Y = -sin(eRotation * (2 * PI / 360));

                    if (negate) {
                        VelocityMovement = Vector2{-X, -Y};
                    } else {
                        VelocityMovement = Vector2{X, Y};
                    }

                    OnWallVelocityBump(VelocityPower);
                    VelocityPower /= 2.15f;
                    LastVelBounceCoord = coord;
                    f =true;
                    break;
                }
            }
        }
        BoundingBox.x -= FinalMovement.x * DeltaTime;
        BoundingBox.y -= FinalMovement.y * DeltaTime;
        FinalMovement = Vector2{(mov.x * GetSpeed()) + (vel.x * VelocityPower), (mov.y * GetSpeed()) + (vel.y * VelocityPower)};
    }
    if (Vector2Distance({0,0}, FinalMovement) > 0) {
        if (CollisionsEnabled) {

            BoundingBox.x += FinalMovement.x * DeltaTime;
            bool can_move_x = true;
            int tile_x = static_cast<int> (BoundingBox.x / game->GameTiles.TileSize);
            int tile_y = static_cast<int> (BoundingBox.y / game->GameTiles.TileSize);
            for (int y = 0; y < 3; y++) {
                for (int x = 0; x < 3; x++) {
                    int curr_tile_x = tile_x + x - 1;
                    int curr_tile_y = tile_y + y - 1;
                    Vector2 coord = {(float)curr_tile_x, (float)curr_tile_y};
                    int tile_id = game->GameTiles.GetTileAt(coord);
                    float bbox_x = curr_tile_x * game->GameTiles.TileSize;
                    float bbox_y = curr_tile_y * game->GameTiles.TileSize;
                    if (game->DebugDraw) {
                        DrawRectangleRec(
                            {
                                bbox_x,
                                bbox_y,
                                game->GameTiles.TileSize,
                                game->GameTiles.TileSize,
                            }, ColorAlpha(GREEN, 0.15f));
                    }
                    if ((game->GameTiles.TileTypes[tile_id] == WallTileType || (Type == EnemyType ? game->GameTiles.TileTypes[tile_id] == EnemyWallTileType : false))) {
                        Rectangle bbox = Rectangle{bbox_x, bbox_y, game->GameTiles.TileSize, game->GameTiles.TileSize};
                        if (CheckCollisionRecs(BoundingBox, bbox)) {
                            //float e_cx = BoundingBox.x + (BoundingBox.width / 2.0f);
                            //float t_cx = bbox_x + (game->MainTileManager.TileSize / 2.0f);
                            //normal_x = (t_cx > e_cx ? 1 : -1);
                            //normal_x = (t_cx == e_cx ? 0 : normal_x);
                            can_move_x = false;
                            break;
                        }
                    }
                }
                if (!can_move_x) {
                    break;
                }
            }
            if (!can_move_x) {
                BoundingBox.x -= FinalMovement.x * DeltaTime;
            }

            BoundingBox.y += FinalMovement.y * DeltaTime;
            can_move_x = true;
            tile_x = static_cast<int> (BoundingBox.x / game->GameTiles.TileSize);
            tile_y = static_cast<int> (BoundingBox.y / game->GameTiles.TileSize);
            for (int y = 0; y < 3; y++) {
                for (int x = 0; x < 3; x++) {
                    int curr_tile_x = tile_x + x - 1;
                    int curr_tile_y = tile_y + y - 1;
                    Vector2 coord = {(float)curr_tile_x, (float)curr_tile_y};
                    int tile_id = game->GameTiles.GetTileAt(coord);
                    if ((game->GameTiles.TileTypes[tile_id] == WallTileType || (Type == EnemyType ? game->GameTiles.TileTypes[tile_id] == EnemyWallTileType : false))) {
                        float bbox_x = curr_tile_x * game->GameTiles.TileSize;
                        float bbox_y = curr_tile_y * game->GameTiles.TileSize;
                        Rectangle bbox = Rectangle{bbox_x, bbox_y, game->GameTiles.TileSize, game->GameTiles.TileSize};
                        if (CheckCollisionRecs(BoundingBox, bbox)) {
                            //float e_cy = BoundingBox.y + (BoundingBox.height / 2.0f);
                            //float t_cy = bbox_y + (game->MainTileManager.TileSize / 2.0f);
                            //normal_y = (t_cy > e_cy ? 1 : -1);
                            //normal_y = (t_cy == e_cy ? 0 : normal_y);
                            can_move_x = false;
                            break;
                        }
                    }
                }
                if (!can_move_x) {
                    break;
                }
            }
            if (!can_move_x) {
                BoundingBox.y -= FinalMovement.y * DeltaTime;
            }
        } else {
            BoundingBox.x += FinalMovement.x * DeltaTime;
            BoundingBox.y += FinalMovement.y * DeltaTime;
        }


    }
}

bool Entity::IsVisible()
{
    return IsVisible(BoundingBox);
}

bool Entity::IsVisible(Rectangle bbox) {
    Vector2 MyPosOnScreen = GetWorldToScreen2D(Vector2{bbox.x + bbox.width/2, bbox.y + bbox.height/2}, game->GameCamera.RaylibCamera);
    return MyPosOnScreen.x > -BoundingBox.width &&
        MyPosOnScreen.x < GetRenderWidth() &&
        MyPosOnScreen.y > -BoundingBox.height &&
        MyPosOnScreen.y < GetRenderHeight();
}

void Entity::Update() {
    if (Health <= 0) {
        ShouldDelete = true;
    }

    bool is_visible = IsVisible();
    if (is_visible && Texture != nullptr)
    {
        DrawTexturePro(*Texture, Rectangle{0, 0, static_cast<float> (Texture->width), static_cast<float> (Texture->height)},
                       Rectangle{BoundingBox.x + BoundingBox.width/2, BoundingBox.y + BoundingBox.height/2, BoundingBox.width,
                                 BoundingBox.height}, Vector2{BoundingBox.width/2,BoundingBox.height/2}, Rotation, EntityColor);
    }
    if (game->DebugDraw && is_visible)
        DrawRectangleRec({
            BoundingBox.x,
            BoundingBox.y,
            BoundingBox.width,
            BoundingBox.height
        }, ColorAlpha(PINK, 0.5f));
}

void Entity::OnDelete() {

}

void Entity::OnDeath()
{

}
