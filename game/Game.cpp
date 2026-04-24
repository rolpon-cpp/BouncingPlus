//
// Created by lalit on 8/26/2025.
//

#include "Game.h"
#include <iostream>
#include <raymath.h>
#include "raylib.h"
#include "../entities/systems/Weapons.h"
#include "ui/UIManager.h"
#include <filesystem>
#include "../level/LevelLoader.h"

namespace fs = std::filesystem;

using namespace std;

void HideCursorCrossPlatform() {
#ifdef PLATFORM_WEB
#else
    HideCursor();
#endif
}

void ShowCursorCrossPlatform() {
#ifdef PLATFORM_WEB
#else
    ShowCursor();
#endif
}

bool IsCursorHiddenCrossPlatform() {
#ifdef PLATFORM_WEB
    return false;
#else
    return IsCursorHidden();
#endif
}

Game::Game(SharedManager& Shared)
{
    this->GameShared = &Shared;
    GameControls = &GameShared->Controls;
    LevelData = this->GameShared->LevelData;

    // init game services
    GameUI = UIManager(*this);
    GameTiles = TileManager(*this);
    GameParticles = ParticleManager(*this);
    GameCamera = CameraManager(*this);
    GameEntities = EntityManager(*this);
    GameSounds = SoundManager(*this);
    GameMode = GameModeManager(*this);
    GameResources = ResourceManager(*this);

    // profiler
    GameProfiler = Profiler(*this);

    // game speed & timing
    GameSpeed = 1.0f;
    SlowdownTime = 0;
    GameTime = 0.0f;
    AverageDeltaTime = 0.0f;
    LastAverageDeltaTime = 0.0f;
    LastDeltaTime = 0.0f;
    StutterCooldown = 0.0f;
    Stutters= 0;
    MaxSlowdownTime = 0;
    LastStartedRecordingDelta = GetTime();
    GameScore = 0;

    Paused = false;

    BannedWeaponDrops = std::vector<std::string>();
    EnemyRoleWeapons = unordered_map<std::string, std::string>();
    WeaponPickups = std::vector<WeaponPickup>();
    FreezeZones = std::vector<std::pair<Rectangle, double>>();

    // extra stuff
    MainPlayer = nullptr;
    CurrentLevelName = "";
    DebugDraw = false;
    ShouldReturn = false;
    isReturning = false;
    DisplayProfiler = false;

    SetGameData();
}

void Game::SetGameData() {
    GameResources.Load();

    uOutlineSize = GetShaderLocation(GameResources.Shaders["weps_outline"], "outlineSize");
    uOutlineColor = GetShaderLocation(GameResources.Shaders["weps_outline"], "outlineColor");
    uTextureSize = GetShaderLocation(GameResources.Shaders["weps_outline"], "textureSize");
    uThreshold = GetShaderLocation(GameResources.Shaders["weps_outline"], "threshold");

    WeaponPickupTex = LoadRenderTexture(150, 150);

    BannedWeaponDrops.emplace_back("Default Gun");
    BannedWeaponDrops.emplace_back("Player Gun");
}

float Game::GetGameDeltaTime()
{
    return GetFrameTime() * GameSpeed;
}

double Game::GetGameTime()
{
    return GameTime;
}

void Game::Slowdown(float Time) {
    SlowdownTime = Time;
    MaxSlowdownTime = Time;
}

void Game::Slowdown(float Time, float CrashIntensity) {
    SlowdownTime = Time;
    MaxSlowdownTime = Time;
    SlowdownShakeIntensity = CrashIntensity;
}

void Game::ProcessSlowdownAnimation() {
    if (SlowdownTime > 0 && MaxSlowdownTime > 0) {
        GameSpeed = 0.0f;
        float Percent = SlowdownTime / MaxSlowdownTime;
        if (Percent >= 0.1)
        {
            if (Percent < 0.5)
                GameSpeed = Lerp(1.0f, 0.1f, Percent / 0.5f);
            else
                GameSpeed = Lerp(0.1f, 1.0f, (Percent-0.5f) / 0.5f);
        }
        if (SlowdownShakeIntensity > 0 && Percent < 0.5f) {
            GameCamera.ShakeCamera(SlowdownShakeIntensity);
            GameSounds.PlayGameSound("dash_hit", min(max(SlowdownShakeIntensity, 0.0f), 1.0f));
            SlowdownShakeIntensity = 0;
        }
        SlowdownTime -= GetFrameTime();
    } else {
        SlowdownTime = 0;
        MaxSlowdownTime = 0;
        SlowdownShakeIntensity = 0;
        GameSpeed = 1.0f;
    }
}

void Game::PlaceWeaponPickup(WeaponPickup Pickup) {
    Pickup.CreationTime = GetGameTime();
    WeaponPickups.push_back(Pickup);
}

void Game::DisplayPickups()
{
    std::erase_if(WeaponPickups, [&](WeaponPickup& pickup) {
            return pickup.PickedUp || GetGameTime() - pickup.CreationTime >= 45 || !GameResources.Weapons.count(pickup.Weapon);
    });
    for (WeaponPickup& pickup : WeaponPickups)
    {
        if (Vector2Distance(pickup.Position, MainPlayer->GetCenter()) >= GetRenderWidth())
            continue;
        // get floating offset
        float AnimationOffset = sin((GetGameTime() - pickup.CreationTime) * pickup.AnimationSpeed) * pickup.AnimationPower;
        Weapon& PickupWeapon = GameResources.Weapons.at(pickup.Weapon);
        std::string TexString = "placeholder";
        if (GameResources.Textures.count(PickupWeapon.texture))
            TexString=PickupWeapon.texture;

        Vector2 siz = {(float)GameResources.Textures[TexString].width, (float)GameResources.Textures[TexString].height};
        siz = Vector2Normalize(siz);
        siz = Vector2Multiply(siz, {pickup.Radius*2.5f, pickup.Radius*2.5f});

        float outlineSize = 3.0f;
        float threshold = 0.5f;
        Color outlineColor = pickup.PickupColor;

        DrawCircle(pickup.Position.x, pickup.Position.y, pickup.Radius / 1.125f, ColorAlpha(BLACK, 0.2f));

        GameCamera.BeginRenderTexture(WeaponPickupTex);
        ClearBackground(BLANK);

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

        DrawTexturePro(GameResources.Textures[TexString], {
            0,
            0,
            (float)GameResources.Textures[TexString].width,
            (float)GameResources.Textures[TexString].height
        }, {
            WeaponPickupTex.texture.width/2.0f,WeaponPickupTex.texture.height/2.0f,
            siz.x,
            siz.y

        }, {siz.x / 2, siz.y / 2}, 0, WHITE);
        EndBlendMode();
        GameCamera.EndRenderTexture();

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
        BeginShaderMode(GameResources.Shaders["weps_outline"]);
        SetShaderValue(GameResources.Shaders["weps_outline"], uTextureSize, &texSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(GameResources.Shaders["weps_outline"], uThreshold, &threshold, SHADER_UNIFORM_FLOAT);
        SetShaderValue(GameResources.Shaders["weps_outline"], uOutlineSize, &outlineSize, SHADER_UNIFORM_FLOAT);
        SetShaderValue(GameResources.Shaders["weps_outline"], uOutlineColor, &outlineColorRGB, SHADER_UNIFORM_VEC4);
        DrawTexturePro(WeaponPickupTex.texture, {0,0,(float)WeaponPickupTex.texture.width,
            (float)-WeaponPickupTex.texture.height
        }, {pickup.Position.x,
            pickup.Position.y - AnimationOffset,
            (float)WeaponPickupTex.texture.width,
                (float)WeaponPickupTex.texture.height},
            {WeaponPickupTex.texture.width/2.0f,WeaponPickupTex.texture.height/2.0f}, 0, WHITE);
        EndBlendMode();
        EndShaderMode();

        if (DebugDraw)
            DrawCircleV({pickup.Position.x,
            pickup.Position.y - AnimationOffset}, pickup.Radius, ColorAlpha(RED, 0.5f));

        // get distance
        float DistanceToPickup = Vector2Distance(pickup.Position, {
            MainPlayer->BoundingBox.x,
            MainPlayer->BoundingBox.y
        });

        // in range?
        if (DistanceToPickup <= pickup.Radius && pickup.LeftOwner)
        {
            pickup.PickedUp = MainPlayer->MainWeaponsSystem.GiveWeapon(pickup.Weapon, pickup.Ammo);
        } else if (DistanceToPickup > pickup.Radius && !pickup.LeftOwner)
            pickup.LeftOwner = true;
    }
}

void Game::ProcessFreezeZones()
{
    std::erase_if(FreezeZones, [this](std::pair<Rectangle, double> rec) {
        return GetGameTime() - rec.second >= 45;
        });
    for (std::pair<Rectangle, double> &rec : FreezeZones)
    {
        float Alpha = 1.0f;
        if (GetGameTime() - rec.second <= 1.0f)
            Alpha = GetGameTime() - rec.second;
        if (GetGameTime() - rec.second >= 44.0f)
            Alpha = 45.0f - (GetGameTime() - rec.second);
        DrawRectangleRec(rec.first, ColorAlpha(BLUE, Alpha));
        DrawRectangleRec({rec.first.x + 4, rec.first.y + 4, rec.first.width - 8, rec.first.height - 8}, ColorAlpha(SKYBLUE, Alpha));
    }
}

void Game::Update() {

    if (this->GameControls->IsControlPressed("pause"))
        Paused = !Paused;
    if (!IsWindowFocused())
        Paused=true;

    if (!Paused) {
        if (!IsCursorHiddenCrossPlatform())
            HideCursorCrossPlatform();

        if (this->GameControls->IsControlPressed("debug") && GameShared->DevMode)
            DebugDraw = !DebugDraw;
        if (!GameShared->DevMode)
            DebugDraw = false;

        GameTime += GetGameDeltaTime();

        // shader stuff
        if (!DebugDraw)
            GameCamera.ShaderDraw = false;
        if (this->GameControls->IsControlPressed("debug2") && DebugDraw)
            GameCamera.ShaderDraw = !GameCamera.ShaderDraw;
        if (this->GameControls->IsControlDown("debug3") && GameCamera.ShaderDraw)
            GameCamera.ShaderPixelPower += 10 * GetGameDeltaTime();
        if (this->GameControls->IsControlDown("debug4") && GameCamera.ShaderDraw)
            GameCamera.ShaderPixelPower -= 10 * GetGameDeltaTime();

        if (this->GameControls->IsControlPressed("level_restart_or_finish"))
        {
            if (GameMode.WonLevel)
            {
                isReturning=true;
                if (!CurrentLevelName.empty() && !LevelData[CurrentLevelName]["music"].get<string>().empty())
                {
                    for (int i = 1; i < 5; i++)
                    {
                        std::string FightTrack = LevelData[CurrentLevelName]["music"].get<string>()+"_layer"+to_string(i);
                        GameSounds.StopGameMusic(FightTrack, true);
                    }
                }
            }
            else if ((MainPlayer->Health <= 0 || MainPlayer->ShouldDelete) && !CurrentLevelName.empty())
            {
                Reload(CurrentLevelName);
                GameUI.StartingBlackScreenTrans = 0.0f;
                GameUI.EndBlackScreenTrans = 0.0f;
            }
        }

        GameCamera.Begin();

        GameProfiler.ProfilerLog("misc");
        ProcessSlowdownAnimation();
        ProcessFreezeZones();
        DisplayPickups();

        GameProfiler.ProfilerLog("tiles");
        GameTiles.Update();

        GameProfiler.ProfilerLog("particles");
        GameParticles.Update();

        GameProfiler.ProfilerLog("entities");
        GameEntities.Update();

        GameProfiler.ProfilerLog("sound");
        GameSounds.Update();

        GameProfiler.ProfilerLog("gamemode");
        GameMode.Update();

        GameProfiler.StopLog();

        GameCamera.End();

    } else if (IsCursorHiddenCrossPlatform())
        ShowCursorCrossPlatform();


    GameCamera.Display();
    GameUI.GameUI();

    DisplayProfilerInfo();

    if (isReturning)
    {
        GameUI.StartingBlackScreenTrans = 0;
        GameUI.EndBlackScreenTrans += 0.65f * GetFrameTime();
        if (GameUI.EndBlackScreenTrans >= 0.9f)
            ShouldReturn = true;
    } else
        GameUI.EndBlackScreenTrans = 0;
    if (ShouldReturn)
    {
        GameUI.StartingBlackScreenTrans = 1.0f;
        GameUI.EndBlackScreenTrans = 0.0f;
    }
}

void Game::DisplayProfilerInfo()
{
    if (this->GameControls->IsControlPressed("debug2"))
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

RayCastData Game::RayCastPoint(Vector2 Origin, Vector2 Target, bool Debug)
{
    Vector2 vRayStart = Vector2{Origin.x / GameTiles.TileSize, Origin.y / GameTiles.TileSize};
    Vector2 vRayTarget = Vector2{Target.x / GameTiles.TileSize, Target.y / GameTiles.TileSize};
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
        if (vMapCheck.x >= 0 && vMapCheck.x < GameTiles.MapWidth && vMapCheck.y >= 0 && vMapCheck.y < GameTiles.MapHeight)
        {
            id = GameTiles.GetTileAt({(float)vMapCheck.x,(float)vMapCheck.y});
            int t_id = GameTiles.TileTypes[id];
            if (t_id == WallTileType || t_id == EnemyWallTileType)
            {
                bTileFound = true;
                break;
            }
        }
    }

    // Calculate intersection location
    Vector2 vIntersection = Origin - Vector2Normalize(Origin - Target) * fDistance * GameTiles.TileSize;

    if (DebugDraw)
        DrawLine(vIntersection.x, vIntersection.y,vRayStart.x*GameTiles.TileSize,vRayStart.y*GameTiles.TileSize,RED);

    return RayCastData{!bTileFound, vIntersection, id};
}

bool Game::RayCast(Vector2 Origin, Vector2 Target, bool Debug) {
    return RayCastPoint(Origin,Target,Debug).HitAir;
}

void Game::Clear() {
    Paused = false;
    ShouldReturn = false;
    isReturning = false;
    GameProfiler.Finish();
    GameTime = 0;
    GameScore = 0;
    GameEntities.Clear();
    BannedWeaponDrops.clear();
    EnemyRoleWeapons.clear();
    CurrentLevelName.clear();
    WeaponPickups.clear();
    GameTiles.Clear();
    FreezeZones.clear();
    GameParticles.Clear();
    GameSounds.Clear();
    GameCamera.Clear();
    GameMode.Clear();
    GameUI.Clear();
    MainPlayer.reset();

    #ifdef PLATFORM_WEB
    GameSounds.ClearCache();
    #endif
}

void Game::Reload(std::string MapName) {
    Clear();

    CurrentLevelName = MapName;

    for (std::string s : LevelData[MapName]["game"]["banned_spawn_weapons"])
        BannedWeaponDrops.emplace_back(s);
    EnemyRoleWeapons= LevelData[MapName]["enemy_weapons"].get<unordered_map<std::string, std::string>>();

    GameTiles.ReadMapDataFile("assets/maps/" + CurrentLevelName + "/map_data.csv");
    if (fs::exists(("assets/maps/" + CurrentLevelName + "/entities.csv").c_str()))
        GameTiles.ReadEntitiesFile("assets/maps/" + CurrentLevelName + "/entities.csv");


    GameMode.PrepareGameMode(LevelData[MapName]);

    MainPlayer = make_shared<Player>(GameTiles.PlayerSpawnPosition.x,
                                     GameTiles.PlayerSpawnPosition.y, LevelData[MapName]["player"]["starting_speed"],
                                     GameResources.Textures["player"], *this);
    MainPlayer->MaxHealth = LevelData[MapName]["player"]["starting_health"];
    MainPlayer->Health = LevelData[MapName]["player"]["starting_health"];
    GameCamera.CameraPosition = Vector2Add(MainPlayer->GetCenter(), {
        (float)GetRandomValue(-100, 100),
        (float)GetRandomValue(-100, 100),
    });
    GameEntities.AddEntity(PlayerType, MainPlayer);
}

void Game::UnloadAssets() {
    EnemyRoleWeapons.clear();
    WeaponPickups.clear();
    GameResources.Weapons.clear();
    BannedWeaponDrops.clear();
    UnloadRenderTexture(WeaponPickupTex);
}

void Game::Quit() {
    Clear();
    GameEntities.Quit();
    GameTiles.Quit();
    GameUI.Quit();
    GameParticles.Quit();
    GameCamera.Quit();
    GameSounds.Quit();
    GameResources.Quit();
    GameMode.Quit();
    UnloadAssets();
}