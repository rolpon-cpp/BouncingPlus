//
// Created by lalit on 5/23/2026.
//

#include "DwellerBehavior.h"
#include "../Enemy.h"
#include "../../../game/Game.h"
#include "raylib.h"
#include "raymath.h"

DwellerBehavior::DwellerBehavior()
{
}

DwellerBehavior::DwellerBehavior(Enemy& Owner, Game& game) : EnemyBehavior(Owner, game)
{
    BehaviorType = DwellerBehaviorType;
    Owner.RenderHealthBar = false;

    Vector2 Pos = Owner.GetCenter();
    Owner.BoundingBox.width = game.GameTiles.TileSize;
    Owner.BoundingBox.height = game.GameTiles.TileSize;

    Owner.BoundingBox.x = Pos.x - game.GameTiles.TileSize / 2.0f;
    Owner.BoundingBox.y = Pos.y - game.GameTiles.TileSize / 2.0f;

    Owner.Armor = 0.0f;
    Owner.Texture = &game.GameResources.Textures[GetRandomValue(0,100) <= 50 ? "delete_wall" : "bouncy_wall"];

    ChooseNewMovementSpot();
}

void DwellerBehavior::ChooseNewMovementSpot()
{
    HasMovedIntoPlace = false;

    float RandomX = (float) GetRandomValue(-360, 360);
    float RandomY = (float) GetRandomValue(-360, 360);
    Vector2 MovementDir = Vector2Normalize({RandomX,RandomY});

    RayCastData d = game->RayCastPoint(Owner->GetCenter(), Owner->GetCenter() + (MovementDir * 900));
    
    d.HitPosition.x /= game->GameTiles.TileSize;
    d.HitPosition.y /= game->GameTiles.TileSize;
    
    d.HitPosition.x = round(d.HitPosition.x);
    d.HitPosition.y = round(d.HitPosition.y);
    
    d.HitPosition.x *= game->GameTiles.TileSize;
    d.HitPosition.y *= game->GameTiles.TileSize;
    
    if (!d.HitAir)
        PlaceSpot = d.HitPosition;
    else
        PlaceSpot = Owner->GetCenter();
    
    TravelDistance = Vector2Distance(Owner->GetCenter(), PlaceSpot)+1;
    TravelStartTime = this->game->GetGameTime();
}

DwellerBehavior::~DwellerBehavior()
{
}

void DwellerBehavior::Update()
{
    Owner->weaponsSystem.CanDisplayWeaponTex= false;
    if (!HasMovedIntoPlace)
    {
        Owner->Movement = PlaceSpot;
        
        if (Vector2Distance(Owner->GetCenter(), PlaceSpot) <= 36.0f)
            HasMovedIntoPlace = true;
        
        if (!HasMovedIntoPlace)
        {
            float TimePassed = this->game->GetGameTime() - TravelStartTime;
            float ExpectedDistance = TimePassed * Owner->Speed;
            if (ExpectedDistance >= TravelDistance * 1.25f)
                ChooseNewMovementSpot();
        }
    } else
    {
        float PlrDistance = Vector2Distance(Owner->GetCenter(), game->MainPlayer->GetCenter());
        if (PlrDistance < 150)
        {
            Owner->Movement = Vector2Subtract(Owner->GetCenter(), game->MainPlayer->GetCenter());
            Owner->weaponsSystem.Attack(game->MainPlayer->GetCenter());
        }
    }

    EnemyBehavior::Update();
}
