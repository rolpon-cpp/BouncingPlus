//
// Created by lalit on 6/11/2026.
//

#include "GameMisc.h"
#include "../Game.h"
#include "raymath.h"

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

void GameMisc::SetGameData()
{
    uOutlineSize = GetShaderLocation(game->GameResources.Shaders["weps_outline"], "outlineSize");
    uOutlineColor = GetShaderLocation(game->GameResources.Shaders["weps_outline"], "outlineColor");
    uTextureSize = GetShaderLocation(game->GameResources.Shaders["weps_outline"], "GOODtextureSize");
    uThreshold = GetShaderLocation(game->GameResources.Shaders["weps_outline"], "threshold");

    WeaponPickupTex = LoadRenderTexture(150, 150);
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
    UnloadRenderTexture(WeaponPickupTex);
}

void GameMisc::DisplayProfilerInfo()
{
    if (this->game->GameControls->IsControlPressed("debug2"))
        DisplayProfiler = !DisplayProfiler;

    std::map<std::string, double> times = GameProfiler.Finish();

    if (DisplayProfiler)
    {
        int i = 0;
        for (auto [name,val] : times)
        {
            DrawText((name + ", " + to_string(val * 1000.0f) + "ms").c_str(), 250, 150 + i * 50, 50, RED);
            i++;
        }
        if (AverageDeltaTime > LastAverageDeltaTime && AverageDeltaTime >= LastAverageDeltaTime * 1.25f)
            DrawText("lag detected", 600, 600, 50, RED);

    }

    if (GetTime() - LastStartedRecordingDelta < 1.0f)
    {
        RecordedDeltaTimes.push_back(GetFrameTime());
    } else
    {
        LastAverageDeltaTime = AverageDeltaTime;

        if (RecordedDeltaTimes.size() > 0)
        {
            AverageDeltaTime = 0.0f;
            for (float d : RecordedDeltaTimes)
                AverageDeltaTime += d;
            AverageDeltaTime /= (float) RecordedDeltaTimes.size();
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
            DrawText(("stutter detected " + to_string(Stutters)).c_str(), 700, 700, 50, RED);
        StutterCooldown -= GetFrameTime();
    } else
    {
        Stutters = 0;
    }

    LastDeltaTime = GetFrameTime();
}

RayCastData GameMisc::RayCastPoint(Vector2 Origin, Vector2 Target, bool Debug)
{
    int q_check = game->GameTiles.GetTileAtWorldCoords(Origin);
    if (game->GameTiles.TileTypes[q_check] == WallTileType || game->GameTiles.TileTypes[q_check] == EnemyWallTileType)
    {
        return RayCastData{false, Origin, q_check};
    }
    Vector2 vRayStart = Vector2{Origin.x / game->GameTiles.TileSize, Origin.y / game->GameTiles.TileSize};
    Vector2 vRayTarget = Vector2{Target.x / game->GameTiles.TileSize, Target.y / game->GameTiles.TileSize};
    Vector2 vRayDir = Vector2Normalize(vRayTarget - vRayStart);

    Vector2 vRayUnitStepSize = { sqrt(1 + (vRayDir.y / vRayDir.x) * (vRayDir.y / vRayDir.x)), sqrt(1 + (vRayDir.x / vRayDir.y) * (vRayDir.x / vRayDir.y)) };
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
        if (vMapCheck.x >= 0 && vMapCheck.x < game->GameTiles.MapWidth && vMapCheck.y >= 0 && vMapCheck.y < game->GameTiles.MapHeight)
        {
            id = game->GameTiles.GetTileAt({(float)vMapCheck.x,(float)vMapCheck.y});
            int t_id = game->GameTiles.TileTypes[id];
            if (t_id == WallTileType || t_id == EnemyWallTileType)
            {
                bTileFound = true;
                break;
            }
        }
    }

    // Calculate intersection location
    Vector2 vIntersection = Origin - Vector2Normalize(Origin - Target) * fDistance * game->GameTiles.TileSize;

    if (game->DebugDraw)
        DrawLine(vIntersection.x, vIntersection.y,vRayStart.x*game->GameTiles.TileSize,vRayStart.y*game->GameTiles.TileSize,RED);

    return RayCastData{!bTileFound, vIntersection, id};
}

bool GameMisc::RayCast(Vector2 Origin, Vector2 Target, bool Debug) {
    return RayCastPoint(Origin,Target,Debug).HitAir;
}

void GameMisc::PlaceWeaponPickup(WeaponPickup Pickup) {
    //game->
    Pickup.CreationTime = game->GetGameTime();
    WeaponPickups.push_back(Pickup);
}

void GameMisc::DisplayPickups()
{
    std::erase_if(WeaponPickups, [&](WeaponPickup& pickup) {
            return pickup.PickedUp || game->GetGameTime() - pickup.CreationTime >= 45 || !game->GameResources.Weapons.count(pickup.Weapon);
    });
    for (WeaponPickup& pickup : WeaponPickups)
    {
        if (Vector2Distance(pickup.Position, game->MainPlayer->GetCenter()) >= GetRenderWidth())
            continue;
        // get floating offset
        float AnimationOffset = sin((game->GetGameTime() - pickup.CreationTime) * pickup.AnimationSpeed) * pickup.AnimationPower;
        Weapon& PickupWeapon = game->GameResources.Weapons.at(pickup.Weapon);
        std::string TexString = "placeholder";
        if (game->GameResources.Textures.count(PickupWeapon.texture))
            TexString=PickupWeapon.texture;

        Vector2 siz = {(float)game->GameResources.Textures[TexString].width, (float)game->GameResources.Textures[TexString].height};
        siz = Vector2Normalize(siz);
        siz = Vector2Multiply(siz, {pickup.Radius*2.5f, pickup.Radius*2.5f});

        float outlineSize = 3.0f;
        float threshold = 0.5f;
        Color outlineColor = pickup.PickupColor;

        DrawCircle(pickup.Position.x, pickup.Position.y, pickup.Radius / 1.125f, ColorAlpha(BLACK, 0.2f));

        game->GameCamera.BeginRenderTexture(WeaponPickupTex);
        ClearBackground(BLANK);

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

        DrawTexturePro(game->GameResources.Textures[TexString], {
            0,
            0,
            (float)game->GameResources.Textures[TexString].width,
            (float)game->GameResources.Textures[TexString].height
        }, {
            WeaponPickupTex.texture.width/2.0f,WeaponPickupTex.texture.height/2.0f,
            siz.x,
            siz.y

        }, {siz.x / 2, siz.y / 2}, 0, WHITE);
        EndBlendMode();
        game->GameCamera.EndRenderTexture();

        Vector2 texSize = {(float)WeaponPickupTex.
            texture.width,
            (float)WeaponPickupTex.texture.height};
        float outlineColorRGB[4] = {
            (float)outlineColor.r / 255.0f,
                (float)outlineColor.g / 255.0f,
                (float)outlineColor.b / 255.0f,
                    (float)outlineColor.a / 255.0f
        };

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        BeginShaderMode(game->GameResources.Shaders["weps_outline"]);
        SetShaderValue(game->GameResources.Shaders["weps_outline"], uTextureSize, &texSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(game->GameResources.Shaders["weps_outline"], uThreshold, &threshold, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->GameResources.Shaders["weps_outline"], uOutlineSize, &outlineSize, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->GameResources.Shaders["weps_outline"], uOutlineColor, &outlineColorRGB, SHADER_UNIFORM_VEC4);
        DrawTexturePro(WeaponPickupTex.texture, {0,0,(float)WeaponPickupTex.texture.width,
            (float)-WeaponPickupTex.texture.height
        }, {pickup.Position.x,
            pickup.Position.y - AnimationOffset,
            (float)WeaponPickupTex.texture.width,
                (float)WeaponPickupTex.texture.height},
            {WeaponPickupTex.texture.width/2.0f,WeaponPickupTex.texture.height/2.0f}, 0, WHITE);
        EndBlendMode();
        EndShaderMode();

        if (game->DebugDraw)
            DrawCircleV({pickup.Position.x,
            pickup.Position.y - AnimationOffset}, pickup.Radius, ColorAlpha(RED, 0.5f));

        // get distance
        float DistanceToPickup = Vector2Distance(pickup.Position, {
            game->MainPlayer->BoundingBox.x,
            game->MainPlayer->BoundingBox.y
        });

        // in range?
        if (DistanceToPickup <= pickup.Radius && pickup.LeftOwner)
        {
            pickup.PickedUp = game->MainPlayer->MainWeaponsSystem.GiveWeapon(pickup.Weapon, pickup.Ammo);
        } else if (DistanceToPickup > pickup.Radius && !pickup.LeftOwner)
            pickup.LeftOwner = true;
    }
}

void GameMisc::ProcessFreezeZones()
{
    std::erase_if(FreezeZones, [this](std::pair<Rectangle, double> rec) {
        return game->GetGameTime() - rec.second >= 45;
        });
    for (std::pair<Rectangle, double> &rec : FreezeZones)
    {
        float Alpha = 1.0f;
        if (game->GetGameTime() - rec.second <= 1.0f)
            Alpha = game->GetGameTime() - rec.second;
        if (game->GetGameTime() - rec.second >= 44.0f)
            Alpha = 45.0f - (game->GetGameTime() - rec.second);
        DrawRectangleRec(rec.first, ColorAlpha(BLUE, Alpha));
        DrawRectangleRec({rec.first.x + 4, rec.first.y + 4, rec.first.width - 8, rec.first.height - 8}, ColorAlpha(SKYBLUE, Alpha));
    }
}