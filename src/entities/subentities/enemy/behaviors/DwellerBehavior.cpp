//
// Created by Rolpon on 5/23/2026.
//

#include "DwellerBehavior.h"
#include "../../../../game/managers/ResourceManager.h"
#include "../../../../level/tiles/TileManager.h"
#include "../Enemy.h"
#include "../../player/Player.h"
#include "../../../../game/core/GameMisc.h"
#include "../../../../game/Game.h"
#include "raylib.h"
#include "raymath.h"

DwellerBehavior::DwellerBehavior()
{
}

DwellerBehavior::DwellerBehavior(Enemy& Owner, Game& game) : EnemyBehavior(Owner, game)
{
    BehaviorType = DwellerEnemyBehaviorType;
    Owner.RenderHealthBar = false;

    Vector2 Pos = Owner.GetCenter();
    Owner.BoundingBox.width = game.GameTiles->TileSize;
    Owner.BoundingBox.height = game.GameTiles->TileSize;

    Owner.BoundingBox.x = Pos.x - game.GameTiles->TileSize / 2.0f;
    Owner.BoundingBox.y = Pos.y - game.GameTiles->TileSize / 2.0f;

    Owner.Armor = 0.0f;
    Owner.Speed = 300.0f;
    Owner.Texture = &game.GameResources->Textures[GetRandomValue(0, 100) <= 50 ? "delete_wall" : "bouncy_wall"];

    ChooseNewMovementSpot();
}

void DwellerBehavior::ChooseNewMovementSpot()
{
    HasMovedIntoPlace = false;

    float RandomX = (float)GetRandomValue(-360, 360);
    float RandomY = (float)GetRandomValue(-360, 360);
    Vector2 MovementDir = Vector2Normalize({RandomX, RandomY});

    Vector2 Targ = Owner->GetCenter() + (MovementDir * 2000);
    RayCastData d = game->GameMiscTools->RayCastPoint(Owner->GetCenter(), Targ);

    d.HitPosition.x /= game->GameTiles->TileSize;
    d.HitPosition.y /= game->GameTiles->TileSize;

    d.HitPosition.x = (int)(d.HitPosition.x);
    d.HitPosition.y = (int)(d.HitPosition.y);

    d.HitPosition.x *= game->GameTiles->TileSize;
    d.HitPosition.y *= game->GameTiles->TileSize;

    PlaceSpot = d.HitPosition;

    TravelDistance = Vector2Distance(Owner->GetCenter(), PlaceSpot) + 1;
    TravelStartTime = this->game->GetGameTime();

    /*
    cout << "DWELLER " << GetRandomValue(0,100) << endl;
    cout << "TARGET: " << Targ.x << " " << Targ.y << "\n";
    cout << "RAYCAST HIT: " << d.HitPosition.x << " " << d.HitPosition.y << "\n";
    cout << "PLACE SPOT: " << PlaceSpot.x << " " << PlaceSpot.y << "\n";
    cout << "MY POSITION: " << Owner->GetCenter().x << " " << Owner->GetCenter().y << "\n";
    */
}

DwellerBehavior::~DwellerBehavior()
{
}


void DwellerBehavior::Update()
{
    if (game->DebugDraw)
        DrawCircleV(PlaceSpot, game->GameTiles->TileSize / 4.0f, ColorAlpha(RED, 0.5f));
    Owner->MainWeaponsSystem.CanDisplayWeaponTex = false;
    if (!HasMovedIntoPlace)
    {
        Owner->Movement = Vector2Subtract(PlaceSpot, Owner->GetCenter());

        if (Vector2Distance(Owner->GetCenter(), PlaceSpot) <= game->GameTiles->TileSize)
            HasMovedIntoPlace = true;

        if (!HasMovedIntoPlace)
        {
            float TimePassed = this->game->GetGameTime() - TravelStartTime;
            float ExpectedDistance = TimePassed * Owner->Speed;
            if (ExpectedDistance >= TravelDistance * 1.5f)
                ChooseNewMovementSpot();
        }
    }
    else
    {
        float PlrDistance = Vector2Distance(Owner->GetCenter(), game->MainPlayer->GetCenter());
        if (PlrDistance < 250 && game->GameMiscTools->RayCast(Owner->GetCenter(), game->MainPlayer->GetCenter()))
        {
            Owner->Movement = Vector2Subtract(game->MainPlayer->GetCenter(), Owner->GetCenter());
            Owner->MainWeaponsSystem.Attack(game->MainPlayer->GetCenter());
        }
        else
        {
            PlaceSpot = Owner->GetCenter();
            PlaceSpot.x = (int)(PlaceSpot.x / game->GameTiles->TileSize) * game->GameTiles->TileSize;
            PlaceSpot.y = (int)(PlaceSpot.y / game->GameTiles->TileSize) * game->GameTiles->TileSize;
            Owner->Movement = Vector2Subtract(PlaceSpot, Owner->GetCenter());
            if (Vector2Distance(Owner->GetCenter(), PlaceSpot) <= 2.5f)
                Owner->Movement = {0, 0};
        }
    }

    EnemyBehavior::Update();
}