//
// Created by lalit on 1/31/2026.
//

#include "WeaponBehavior.h"

#include <iostream>

#include "../../../../game/Game.h"
#include <raymath.h>

WeaponBehavior::WeaponBehavior()
{
}

WeaponBehavior::~WeaponBehavior()
{
}

WeaponBehavior::WeaponBehavior(Enemy& Owner, Game& game) : EnemyBehavior(Owner,game)
{
    BehaviorType = WeaponBehaviorType;
    CoverRefreshTimer = 0;
    CoverPosition = {0,0};
    FoundCover = false;
    CoverSearching = false;
    Target = Owner.GetCenter();
}

void WeaponBehavior::MoveForCover()
{
    float center_x = Owner->BoundingBox.x + (Owner->BoundingBox.width / 2);
    float center_y = Owner->BoundingBox.y + (Owner->BoundingBox.height / 2);

    float plr_center_x = game->MainPlayer->BoundingBox.x + (game->MainPlayer->BoundingBox.width / 2);
    float plr_center_y = game->MainPlayer->BoundingBox.y + (game->MainPlayer->BoundingBox.height / 2);

    if (game->GetGameTime() - CoverRefreshTimer >= 0.5f)
    {
        FoundCover = false;
        float AngleToPlayer = 180 - Vector2LineAngle({center_x,center_x},{plr_center_x,plr_center_y}) * RAD2DEG;
        for (int i = 0; i < 60; i++)
        {
            float Angle = i * 6.0f;
            if (abs(Angle - AngleToPlayer) <= 120)
                continue;
            float X = cos(Angle * (2 * PI / 360))*2000;
            float Y = sin(Angle * (2 * PI / 360))*2000;
            RayCastData d = game->RayCastPoint({center_x,center_y}, {center_x + X,center_y + Y});
            if ((!d.HitAir) || Vector2Distance({plr_center_x,plr_center_y}, d.HitPosition) >= 500) // if we hit wall?
            {
                RayCastData p = game->RayCastPoint({plr_center_x,plr_center_y}, d.HitPosition);
                if (Vector2Distance(p.HitPosition, d.HitPosition) >= 150 && !p.HitAir && Vector2Distance({plr_center_x,plr_center_y}, d.HitPosition) >= 150)
                {
                    CoverPosition = d.HitPosition;
                    FoundCover = true;
                    break;
                }
            }
        }
        CoverRefreshTimer = game->GetGameTime();
    }
    if (FoundCover)
    {
        Owner->Movement = Vector2Normalize(Vector2Subtract(CoverPosition, {center_x,center_y}));
    } else
    {
        Owner->Movement = Vector2Normalize(Vector2Subtract({center_x,center_y},{plr_center_x,plr_center_y}));
    }

    Owner->Health += Owner->HealthRegenRate * game->GetGameDeltaTime();
}

bool WeaponBehavior::FindPlayer()
{
    if (game->RayCast(Owner->GetCenter(), game->MainPlayer->GetCenter()))
    {
        Target = game->MainPlayer->GetCenter();
        return true;
    }
    int casts = 15;
    float StartPlayerAngle = 180.0f - Vector2LineAngle(Owner->GetCenter(), game->MainPlayer->GetCenter()) * RAD2DEG;
    for (int i = 0; i < casts; i++)
    {
        float Angle = (360.0f / casts) * i;
        Angle -= 180;
        Angle += StartPlayerAngle;

        float cX = cos(Angle * (2 * PI / 360)) * 100;
        float cY = sin(Angle * (2 * PI / 360)) * 100;
        Vector2 Direction =Vector2Normalize({cX,cY});
        Vector2 Origin = Owner->GetCenter();

        bool HitSet = false;
        Vector2 HitSetP = game->MainPlayer->GetCenter();

        int tries = 0;
        while (tries < 3)
        {
            RayCastData d = game->RayCastPoint(Origin, Origin + (Vector2Normalize(Direction) * 800.0f));

            if (!HitSet)
            {
                HitSetP = d.HitPosition;
                HitSet = true;
            }

            float RayAngle = 180.0f - Vector2LineAngle(Origin, d.HitPosition) * RAD2DEG;
            float PlayerAngle = 180.0f - Vector2LineAngle(Origin, game->MainPlayer->GetCenter()) * RAD2DEG;
            if (abs(RayAngle - PlayerAngle) <= 5 && game->RayCast(Origin,game->MainPlayer->GetCenter()))
            {
                Target = HitSetP;
                return true;
            }

            int tile_x = (int) (d.HitPosition.x / game->GameTiles.TileSize);
            int tile_y = (int) (d.HitPosition.y / game->GameTiles.TileSize);

            if (!d.HitAir && d.HitTile == 1)
            {
                Vector2 Normal = {0, 0};

                Rectangle bbox = {tile_x * game->GameTiles.TileSize,tile_y * game->GameTiles.TileSize, game->GameTiles.TileSize, game->GameTiles.TileSize};

                float DeltaX = d.HitPosition.x - (bbox.x + bbox.width / 2);
                float DeltaY = d.HitPosition.y - (bbox.y + bbox.width / 2);

                float OverlapX = (bbox.width / 2) - abs(DeltaX);
                float OverlapY = (bbox.height / 2) - abs(DeltaY);

                if (OverlapX < OverlapY)
                    Normal += {DeltaX > 0.0f ? 1.0f : -1.0f, 0};
                else if (OverlapY < OverlapX)
                    Normal += {0, DeltaY > 0.0f ? 1.0f : -1.0f};
                else if (OverlapY == OverlapX)
                    Normal = Vector2Negate(Vector2Normalize(Direction));

                Normal = Vector2Normalize(Normal);

                Direction = Vector2Normalize(Direction);
                float Dot = Vector2DotProduct(Direction, Normal);
                Direction -= Vector2Multiply(Normal, {2 * Dot, 2 * Dot});
                Direction = Vector2Normalize(Direction);
                Origin = d.HitPosition;
            } else
            {
                break;
            }

            tries++;
        }
    }
    return false;
}

void WeaponBehavior::Update()
{

    Owner->Movement = Vector2{0,0};

    // programming is so fun and awesome!!!!!!!!

    float center_x = Owner->BoundingBox.x + (Owner->BoundingBox.width / 2);
    float center_y = Owner->BoundingBox.y + (Owner->BoundingBox.height / 2);

    // checks for enemy anger
    if (Owner->AngeredRangeBypassTimer > 0)
        Owner->AngeredRangeBypassTimer -= game->GetGameDeltaTime();

    if (Owner->AngeredRangeBypassTimer <= 0)
        Owner->AngeredRangeBypassTimer = 0;
    if (Owner->MainWeaponsSystem.CurrentWeapon != nullptr && Owner->MainWeaponsSystem.TimeStartedReloading == -1 && Owner->MainWeaponsSystem.CurrentWeapon->Ammo > 0 && Owner->MainWeaponsSystem.WeaponAmmo[Owner->MainWeaponsSystem.CurrentWeaponIndex] <= 0)
        Owner->MainWeaponsSystem.Reload();

    // distance/firing checks
    float plr_center_x = game->MainPlayer->BoundingBox.x + (game->MainPlayer->BoundingBox.width / 2);
    float plr_center_y = game->MainPlayer->BoundingBox.y + (game->MainPlayer->BoundingBox.height / 2);

    float distance = std::sqrt(std::pow(plr_center_x - center_x, 2) + std::pow(plr_center_y - center_y, 2));

    bool Attacking = false;

    CoverSearching = Owner->RemainingHealthOfOriginalHealth <= 0.6f;
    if ((distance <= 800 && (distance <= 36 || FindPlayer())) || Owner->AngeredRangeBypassTimer > 0.0f) {
        if (distance >= 100 && !CoverSearching) {
            Vector2 othermov = {0,0};
            othermov.x += -(Target.x - center_x) / distance * Owner->Speed * (Owner->MainWeaponsSystem.CurrentWeapon != nullptr ? (Owner->MainWeaponsSystem.CurrentWeapon->isMelee ? -1.0f : 1.0f) : 1.0f);
            othermov.y += -(Target.y - center_y) / distance * Owner->Speed * (Owner->MainWeaponsSystem.CurrentWeapon != nullptr ? (Owner->MainWeaponsSystem.CurrentWeapon->isMelee ? -1.0f : 1.0f) : 1.0f);
            Owner->MoveAwayFromWalls();
            Owner->Movement=Vector2Lerp(othermov, Owner->Movement,0.5f);
        }

        if (game->MainPlayer->Speed >= 200 && (Owner->MainWeaponsSystem.CurrentWeapon != nullptr ? !Owner->MainWeaponsSystem.CurrentWeapon->isMelee : true))
        {
            Target = Vector2Add(Target, Vector2Multiply(Vector2Normalize(game->MainPlayer->Movement), {game->MainPlayer->Speed * 0.2f,game->MainPlayer->Speed * 0.2f}));
        }

        game->MainPlayer->EnemiesDetected += 1;

        Attacking = true;
        Owner->MainWeaponsSystem.Attack(Target);
    } else if (Owner->WanderingEnabled && !CoverSearching) {
        Owner->Wander(); // enemy wandering
        Target = Owner->GetCenter() + Owner->Movement;
    }

    if (CoverSearching) {
        MoveForCover();
        if (!Attacking)
            Target = Owner->GetCenter() + Owner->Movement;
    }

    EnemyBehavior::Update();
}
