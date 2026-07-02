//
// Created by Rolpon on 6/11/2026.
//

#include "GameMisc.h"
#include "../../level/tiles/TileManager.h"
#include "Controls.h"
#include "../Game.h"
#include "../../entities/subentities/player/Player.h"
#include "../managers/ResourceManager.h"
#include "raymath.h"
#include "../../entities/systems/Weapons.h"

GameMisc::GameMisc()
{
}

GameMisc::GameMisc(Game* game)
{
    this->game = game;

    GameProfiler = BaseProfiler();

    AverageDeltaTime = 0.0f;
    LastAverageDeltaTime = 0.0f;
    LastDeltaTime = 0.0f;
    StutterCooldown = 0.0f;
    Stutters = 0;
    LastStartedRecordingDelta = GetTime();
    WeaponPickups = std::vector<WeaponPickup>();
    FreezeZones = std::vector<std::pair<Rectangle, double>>();
    DisplayProfiler = false;
}

GameMisc::~GameMisc()
{
}

void GameMisc::Update()
{
    ProcessFreezeZones();
    DisplayPickups();
}

void GameMisc::Clear()
{
    GameProfiler.Finish();
    WeaponPickups.clear();
    FreezeZones.clear();
}

void GameMisc::Quit()
{
    WeaponPickups.clear();
}

void GameMisc::DisplayProfilerInfo()
{
    if (this->game->GameControls->IsControlPressed("debug2"))
        DisplayProfiler = !DisplayProfiler;

    std::map<std::string, double> times = GameProfiler.Finish();

    if (DisplayProfiler)
    {
        int lines = times.size();
        bool LagDetected = AverageDeltaTime > LastAverageDeltaTime && AverageDeltaTime >= LastAverageDeltaTime * 1.25f;

        if (LagDetected)
            lines += 1;

        DrawRectangle(50, 50, 400, lines * 15, ColorAlpha(BLACK,0.5f));
        DrawRectangle(500, 50, 400, lines * 15, ColorAlpha(BLACK,0.5f));

        if (LagDetected)
            DrawText("lag detected", 50, 50.0f + lines * 15, 15, RED);

        int i = 0;
        for (auto [name,val] : times)
        {
            DrawText((name + ", " + std::to_string(val * 1000.0f) + "ms").c_str(), 50.0f, 50.0f + i * 15, 15, RED);
            i++;
        }
    }

    if (GetTime() - LastStartedRecordingDelta < 1.0f)
    {
        RecordedDeltaTimes.push_back(GetFrameTime());
    }
    else
    {
        LastAverageDeltaTime = AverageDeltaTime;

        if (RecordedDeltaTimes.size() > 0)
        {
            AverageDeltaTime = 0.0f;
            for (float d : RecordedDeltaTimes)
                AverageDeltaTime += d;
            AverageDeltaTime /= (float)RecordedDeltaTimes.size();
        }

        RecordedDeltaTimes.clear();
        LastStartedRecordingDelta = GetTime();
    }

    if (abs(GetFrameTime() - LastDeltaTime) >= AverageDeltaTime * 2.15f && DisplayProfiler)
    {
        Stutters++;
        StutterCooldown = 2.0f;
    }

    if (StutterCooldown > 0.0f)
    {
        if (DisplayProfiler)
            DrawText(("stutter detected " + std::to_string(Stutters)).c_str(), 700, 700, 50, RED);
        StutterCooldown -= GetFrameTime();
    }
    else
    {
        Stutters = 0;
    }

    LastDeltaTime = GetFrameTime();
}

RayCastData GameMisc::RayCastPoint(Vector2 Origin, Vector2 Target, bool Debug)
{
    int q_check = game->GameTiles->GetTileAtWorldCoords(Origin);
    if (game->GameTiles->TileTypes[q_check] == WallTileType || game->GameTiles->TileTypes[q_check] == EnemyWallTileType)
    {
        return RayCastData{false, Origin, q_check};
    }
    Vector2 vRayStart = Vector2{Origin.x / game->GameTiles->TileSize, Origin.y / game->GameTiles->TileSize};
    Vector2 vRayTarget = Vector2{Target.x / game->GameTiles->TileSize, Target.y / game->GameTiles->TileSize};
    Vector2 vRayDir = Vector2Normalize(vRayTarget - vRayStart);

    Vector2 vRayUnitStepSize = {
        sqrt(1 + (vRayDir.y / vRayDir.x) * (vRayDir.y / vRayDir.x)),
        sqrt(1 + (vRayDir.x / vRayDir.y) * (vRayDir.x / vRayDir.y))
    };
    Vector2i vMapCheck = {(int)vRayStart.x, (int)vRayStart.y};
    Vector2 vRayLength1D = {0, 0};
    Vector2i vStep = {0, 0};

    if (vRayDir.x < 0)
    {
        vStep.x = -1;
        vRayLength1D.x = (vRayStart.x - float(vMapCheck.x)) * vRayUnitStepSize.x;
    }
    else
    {
        vStep.x = 1;
        vRayLength1D.x = (float(vMapCheck.x + 1) - vRayStart.x) * vRayUnitStepSize.x;
    }

    if (vRayDir.y < 0)
    {
        vStep.y = -1;
        vRayLength1D.y = (vRayStart.y - float(vMapCheck.y)) * vRayUnitStepSize.y;
    }
    else
    {
        vStep.y = 1;
        vRayLength1D.y = (float(vMapCheck.y + 1) - vRayStart.y) * vRayUnitStepSize.y;
    }

    int id = -1;

    // Perform "Walk" until collision or range check
    bool bTileFound = false;
    float fMaxDistance = Vector2Distance(vRayStart, vRayTarget);
    float fDistance = 0.0f;

    while (!bTileFound && fDistance < fMaxDistance)
    {
        // Walk along shortest path
        if (vRayLength1D.x < vRayLength1D.y)
        {
            vMapCheck.x += vStep.x;
            fDistance = vRayLength1D.x;
            vRayLength1D.x += vRayUnitStepSize.x;
        }
        else
        {
            vMapCheck.y += vStep.y;
            fDistance = vRayLength1D.y;
            vRayLength1D.y += vRayUnitStepSize.y;
        }

        if (fDistance >= fMaxDistance)
        {
            fDistance = fMaxDistance;
            break;
        }

        // Test tile at new test point
        if (vMapCheck.x >= 0 && vMapCheck.x < game->GameTiles->MapWidth && vMapCheck.y >= 0 && vMapCheck.y < game->
            GameTiles->MapHeight)
        {
            id = game->GameTiles->GetTileAt({(float)vMapCheck.x, (float)vMapCheck.y});
            int t_id = game->GameTiles->TileTypes[id];
            if (t_id == WallTileType || t_id == EnemyWallTileType)
            {
                bTileFound = true;
                break;
            }
        }
    }

    // Calculate intersection location
    Vector2 vIntersection = Origin - Vector2Normalize(Origin - Target) * fDistance * game->GameTiles->TileSize;

    if (game->DebugDraw)
        DrawLine(vIntersection.x, vIntersection.y, vRayStart.x * game->GameTiles->TileSize,
                 vRayStart.y * game->GameTiles->TileSize, RED);

    return RayCastData{!bTileFound, vIntersection, id};
}

bool GameMisc::RayCast(Vector2 Origin, Vector2 Target, bool Debug)
{
    return RayCastPoint(Origin, Target, Debug).HitAir;
}

void GameMisc::PlaceWeaponPickup(WeaponPickup Pickup)
{
    //game->
    Pickup.CreationTime = game->GetGameTime();
    WeaponPickups.push_back(Pickup);
}

void GameMisc::DisplayPickups()
{
    (void)std::erase_if(WeaponPickups, [&](WeaponPickup& pickup)
    {
        return pickup.PickedUp || game->GetGameTime() - pickup.CreationTime >= 45.0f || !game->GameResources->Weapons.count(pickup.Weapon);
    });
    for (WeaponPickup& pickup : WeaponPickups)
    {
        if (Vector2Distance(pickup.Position, game->MainPlayer->GetCenter()) >= GetRenderWidth())
            continue;
        // get floating offset
        float AnimationOffset = sin((game->GetGameTime() - pickup.CreationTime) * pickup.AnimationSpeed) * pickup.AnimationPower;

        Weapon& PickupWeapon = game->GameResources->Weapons.at(pickup.Weapon);
        std::string TexString = "placeholder";
        if (game->GameResources->Textures.count(PickupWeapon.texture))
            TexString = PickupWeapon.texture;

        DrawCircle(pickup.Position.x, pickup.Position.y, pickup.Radius / 1.125f, ColorAlpha(BLACK, 0.2f));

        float scale = max(game->GameResources->Textures[TexString].width,game->GameResources->Textures[TexString].height) / pickup.Radius;
        scale *= 0.5f;

        float sw = game->GameResources->Textures[TexString].width / scale * 1.1f;
        float sh = game->GameResources->Textures[TexString].height / scale * 1.1f;

        DrawTexturePro(game->GameResources->Textures[TexString], {0.0f,0.0f,
            (float)game->GameResources->Textures[TexString].width,(float)game->GameResources->Textures[TexString].height},
            {
                pickup.Position.x - sw/2,
                pickup.Position.y - sh/2 - AnimationOffset,
                sw,
                sh
            }, {
                0,0
            }, 0.0f, {255,0,0,255});

        sw /= 1.1f;
        sh /= 1.1f;

        DrawTexturePro(game->GameResources->Textures[TexString], {0.0f,0.0f,
            (float)game->GameResources->Textures[TexString].width,(float)game->GameResources->Textures[TexString].height},
            {
                pickup.Position.x - sw/2,
                pickup.Position.y - sh/2 - AnimationOffset,
                sw,
                sh
            }, {
                0,0
            }, 0.0f, WHITE);

        if (game->DebugDraw)
            DrawCircleV({
                            pickup.Position.x,
                            pickup.Position.y - AnimationOffset
                        }, pickup.Radius, ColorAlpha(RED, 0.5f));

        // get distance
        float DistanceToPickup = Vector2Distance(pickup.Position, {
                                                     game->MainPlayer->BoundingBox.x,
                                                     game->MainPlayer->BoundingBox.y
                                                 });

        // in range?
        if (DistanceToPickup <= pickup.Radius && pickup.LeftOwner)
        {
            pickup.PickedUp = game->MainPlayer->MainWeaponsSystem.GiveWeapon(pickup.Weapon, pickup.Ammo);
        }
        else if (DistanceToPickup > pickup.Radius && !pickup.LeftOwner)
            pickup.LeftOwner = true;
    }
}

void GameMisc::ProcessFreezeZones()
{
    (void)std::erase_if(FreezeZones, [this](std::pair<Rectangle, double> rec)
    {
        return game->GetGameTime() - rec.second >= 45.0f;
    });
    for (std::pair<Rectangle, double>& rec : FreezeZones)
    {
        float Alpha = 1.0f;
        if (game->GetGameTime() - rec.second <= 1.0f)
            Alpha = game->GetGameTime() - rec.second;
        if (game->GetGameTime() - rec.second >= 44.0f)
            Alpha = 45.0f - (game->GetGameTime() - rec.second);
        DrawRectangleRec(rec.first, ColorAlpha(BLUE, Alpha));
        DrawRectangleRec({rec.first.x + 4, rec.first.y + 4, rec.first.width - 8, rec.first.height - 8},
                         ColorAlpha(SKYBLUE, Alpha));
    }
}