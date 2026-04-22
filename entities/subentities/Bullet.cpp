//
// Created by lalit on 8/29/2025.
//

#include "Bullet.h"
#include "typeinfo"
#include <cmath>
#include <ostream>
#include <iostream>
#include <raymath.h>

#include "Enemy.h"
#include "math.h"
#include "../../game/Game.h"

Bullet::Bullet(float X, float Y, float Angle, Vector2 Size, float Speed, float Damage, float Lifetime, Texture2D &BulletTexture, shared_ptr<Entity> Owner, Game &game) : Entity(BulletTexture, {0, 0, 1, 1}, Speed, game) {
    this->Speed = Speed;
    this->Type = BulletType;
    this->ExistenceTimer = 0;
    this->BoundingBox = Rectangle{X - (15 * Size.x / 2.0f), Y - (7.5f * Size.y / 2.0f), 15 * Size.x, 7.5f * Size.y};
    this->Texture=&BulletTexture;
    this->ShouldDelete = false;
    this->SlowdownOverTime = false;
    this->HealthGain = Damage;
    this->Lifetime=Lifetime;
    this->Speed=Speed;
    this->game = &game;
    this->FirePoint = {X, Y};
    this->Rotation = Angle;
    this->RotGoal = Rotation;
    this->Damage = Damage;
    this->OwnerPtr = Owner;

    float cX = -cos(Rotation * (2 * PI / 360))*100;
    float cY = -sin(Rotation * (2 * PI / 360))*100;

    this->Movement = {cX, cY};
}

Bullet::Bullet() {

}

Bullet::~Bullet() {

}

std::pair<bool,vector<Vector2>> Bullet::BulletCollision()
{

    std::vector<Vector2> BouncedTiles;
    bool can_move = true;

    int tile_x = static_cast<int> (BoundingBox.x / game->GameTiles.TileSize);
    int tile_y = static_cast<int> (BoundingBox.y / game->GameTiles.TileSize);

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int curr_tile_x = tile_x + x - 1;
            int curr_tile_y = tile_y + y - 1;
            int tile_id = game->GameTiles.GetTileAt(curr_tile_x, curr_tile_y);
            if (game->GameTiles.TileTypes[tile_id] == WallTileType) {
                float bbox_x = curr_tile_x * game->GameTiles.TileSize;
                float bbox_y = curr_tile_y * game->GameTiles.TileSize;
                Rectangle bbox = Rectangle{bbox_x, bbox_y, game->GameTiles.TileSize, game->GameTiles.TileSize};
                if (CheckCollisionCircleRec(GetCenter(), BoundingBox.height, bbox)) {
                    can_move = false;
                    if (tile_id == 2)
                        ShouldDelete = true;
                    if (tile_id == 1)
                    {
                        bool found_tile = false;

                        for (Vector2 d : LastBouncedCoordinates){
                            if (d == Vector2{(float)curr_tile_x,(float)curr_tile_y})
                            {
                                found_tile = true;
                                break;
                            }
                        }

                        if (!found_tile)
                            BouncedTiles.push_back(Vector2{(float)curr_tile_x,(float)curr_tile_y});
                    }
                }
            }
        }
    }

    return make_pair(can_move,BouncedTiles);
}

void Bullet::PhysicsUpdate(float dt, double time) {

    if (SlowdownOverTime)
        Speed = Lerp(Speed, 0, 10*ExistenceTimer*dt);

    double dist = std::sqrt((Movement.x * Movement.x) + (Movement.y * Movement.y));
    if (dist != 0) {
        Vector2 FinalMovement = Vector2Normalize(Movement) * Speed;

        BoundingBox.x += FinalMovement.x * dt;
        BoundingBox.y += FinalMovement.y * dt;

        auto p = BulletCollision();
        vector<Vector2> BouncedTiles=p.second;

        if (BouncedTiles.size() > 0)
        {
            Vector2 Normal = {0, 0};

            float LowestX = 99999;
            float LowestY = 99999;
            float HigherX = -99999;
            float HigherY = -99999;
            
            for (Vector2 TilePos : BouncedTiles)
            {
                Rectangle TileBBox = {TilePos.x * game->GameTiles.TileSize, TilePos.y * game->GameTiles.TileSize, game->GameTiles.TileSize, game->GameTiles.TileSize};
                LowestX = min(TileBBox.x, LowestX);
                LowestY = min(TileBBox.y, LowestY);
                HigherX = max(TileBBox.x, HigherX);
                HigherY = max(TileBBox.y, HigherY);
            }

            HigherX += game->GameTiles.TileSize;
            HigherY += game->GameTiles.TileSize;

            Rectangle bbox = {LowestX, LowestY, HigherX - LowestX, HigherY - LowestY};

            if (!(bbox.width == game->GameTiles.TileSize*2 && bbox.height == game->GameTiles.TileSize*2))
            {
                float DeltaX = GetCenter().x - (bbox.x + bbox.width / 2);
                float DeltaY = GetCenter().y - (bbox.y + bbox.width / 2);

                float OverlapX = ((bbox.width / 2) + (BoundingBox.width / 2)) - abs(DeltaX);
                float OverlapY = ((bbox.height / 2) + (BoundingBox.width / 2)) - abs(DeltaY);

                if (OverlapX < OverlapY)
                    Normal += {DeltaX > 0.0f ? 1.0f : -1.0f, 0};
                else if (OverlapY < OverlapX)
                    Normal += {0, DeltaY > 0.0f ? 1.0f : -1.0f};
                else if (OverlapY == OverlapX)
                    Normal = Vector2Negate(Vector2Normalize(Movement));
            } else
            {
                Normal = Vector2Negate(Vector2Normalize(Movement));
            }

            Normal = Vector2Normalize(Normal);

            auto Owner = OwnerPtr.lock();
            if (Owner != nullptr && Owner->Type == PlayerType)
            {
                float Bonus = Owner->Health / 5.0f;
                HealthGain += min(Bonus / 20.0f, 5.0f);
                Damage += min(Bonus, 5.0f);
            }

            if (IsVisible())
                game->GameTiles.DistortArea(Distortion{
                game->RayCastPoint(GetCenter(), Vector2Add(GetCenter(), Vector2Multiply(Vector2Normalize(Movement), {100, 100}))).HitPosition,
                3.0f,
                BoundingBox.width * 12.5f
                });


            Bounce(Normal);
            LastBouncedCoordinates = BouncedTiles;
        }

        if (!p.first)
        {
            BoundingBox.x -= FinalMovement.x * dt;
            BoundingBox.y -= FinalMovement.y * dt;
        }
    }
}

void Bullet::Bounce(Vector2 Normal)
{
    Movement = Vector2Normalize(Movement);
    float Dot = Vector2DotProduct(Movement, Normal);
    Movement -= Vector2Multiply(Normal, {2 * Dot, 2 * Dot});
    Movement = Vector2Normalize(Movement);
    RotGoal = 180.0f - Vector2LineAngle({0,0},Movement) * RAD2DEG;
}

void Bullet::Attack(shared_ptr<Entity> entity) {
    auto Owner = OwnerPtr.lock();
    if (CheckCollisionCircleRec({BoundingBox.x,BoundingBox.y}, BoundingBox.width, entity->BoundingBox) && !entity->ShouldDelete) {
        DamageOther(entity, Damage, Owner, HealthGain);
        ShouldDelete = true;
    }
}

void Bullet::Update() {
    ExistenceTimer += game->GetGameDeltaTime();
    Rotation = Lerp(Rotation, RotGoal, min(25.0f * (Speed / 400.0f) * game->GetGameDeltaTime(), 1.0f));
    if (!SlowdownOverTime) {

        if (ExistenceTimer >= Lifetime) {
            ShouldDelete = true;
        }
    } else if (Speed < 50) {
        ShouldDelete = true;
    }
    auto Owner = OwnerPtr.lock();
    for (shared_ptr entity : game->GameEntities.Entities[EnemyType]) {
        if (entity != nullptr && entity != Owner && !entity->ShouldDelete) {
            Attack(entity);
        }
    }
    if (Owner == nullptr && !DD) {
        Damage /= 2.0f;
        DD = true;
    }
    if (Owner != game->MainPlayer)
        Attack(game->MainPlayer);
    Entity::Update();
}