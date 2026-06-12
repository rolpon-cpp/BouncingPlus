//
// Created by lalit on 8/26/2025.
//

#include "Game.h"
#include <iostream>
#include <raymath.h>
#include "raylib.h"
#include "../entities/systems/Weapons.h"
#include "ui/gameplay_ui/GameplayUI.h"
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

    // init game services
    GameUI = GameplayUI(this);
    GameTiles = TileManager(this);
    GameParticles = ParticleManager(this);
    GameCamera = CameraManager(this);
    GameEntities = EntityManager(this);
    GameSounds = SoundManager(this);
    GameMode = GameModeManager(this);
    GameResources = ResourceManager(this);
    GameMiscTools = GameMisc(this);

    // game speed & timing
    GameSpeed = 1.0f;
    FreezeTime = 0;
    GameTime = 0.0f;
    MaxFreezeTime = 0;
    GameScore = 0;
    Paused = false;
    BannedWeaponDrops = std::vector<std::string>();
    EnemyRoleWeapons = unordered_map<std::string, std::string>();

    // extra stuff
    MainPlayer = nullptr;
    DebugDraw = false;
    ShouldReturn = false;
    isReturning = false;

    SetGameData();
}

void Game::SetGameData() {
    GameResources.Load();
    GameMiscTools.SetGameData();
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

void Game::Freeze(float Time) {
    FreezeTime = Time;
    MaxFreezeTime = Time;
}

void Game::ProcessSlowdownAnimation() {
    if (FreezeTime > 0 && MaxFreezeTime > 0) {
        GameSpeed = 0.0f;
        FreezeTime -= GetFrameTime();
    } else {
        FreezeTime = 0;
        MaxFreezeTime = 0;
        GameSpeed = GameMode.LevelGameSpeed;
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

        if (this->GameControls->IsControlPressed("level_restart_or_finish"))
        {
            if (GameMode.WonLevel)
            {
                isReturning=true;
                if (!GameMode.GetCurrentLevelName().empty() && !this->GameShared->LevelData[GameMode.GetCurrentLevelName()]["music"].get<string>().empty())
                {
                    for (int i = 1; i < 5; i++)
                    {
                        std::string FightTrack = this->GameShared->LevelData[GameMode.GetCurrentLevelName()]["music"].get<string>()+"_layer"+to_string(i);
                        GameSounds.StopGameMusic(FightTrack, true);
                    }
                }
            }
            else if ((MainPlayer->Health <= 0 || MainPlayer->ShouldDelete) && !GameMode.GetCurrentLevelName().empty())
            {
                Reload(GameMode.GetCurrentLevelName());
                GameUI.StartingBlackScreenTrans = 0.0f;
                GameUI.EndBlackScreenTrans = 0.0f;
            }
        }

        GameCamera.Begin();

        GameMiscTools.GameProfiler.ProfilerLog("misc");
        ProcessSlowdownAnimation();
        GameMiscTools.Update();

        GameMiscTools.GameProfiler.ProfilerLog("tiles");
        GameTiles.Update();

        GameMiscTools.GameProfiler.ProfilerLog("particles");
        GameParticles.Update();

        GameMiscTools.GameProfiler.ProfilerLog("entities");
        GameEntities.Update();

        GameMiscTools.GameProfiler.ProfilerLog("sound");
        GameSounds.Update();

        GameMiscTools.GameProfiler.ProfilerLog("gamemode");
        GameMode.Update();

        GameMiscTools.GameProfiler.StopLog();

        GameCamera.End();

    } else if (IsCursorHiddenCrossPlatform())
        ShowCursorCrossPlatform();
    GameCamera.Display(GameSpeed == 0.0f);

    GameUI.GameUI();

    GameMiscTools.DisplayProfilerInfo();

    if (isReturning)
    {
        GameUI.StartingBlackScreenTrans = 0;
        GameUI.EndBlackScreenTrans += 0.65f * GetFrameTime();
        GameTiles.Lines.clear();
        GameTiles.PrevFileName.clear();
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

void Game::Clear() {
    Paused = false;
    ShouldReturn = false;
    isReturning = false;

    GameTime = 0;
    FinalLevelCompletionScore = 0;
    GameScore = 0;

    GameEntities.Clear();
    BannedWeaponDrops.clear();
    EnemyRoleWeapons.clear();
    GameTiles.Clear();
    GameMiscTools.Clear();
    GameParticles.Clear();
    GameSounds.Clear();
    GameCamera.Clear();
    GameMode.Clear();
    GameUI.Clear();
    MainPlayer.reset();
    GameSounds.ClearCache();
}

void Game::Reload(std::string MapName) {
    Clear();

    GameMode.PrepareGameMode(this->GameShared->LevelData[MapName], MapName);

    for (std::string s : this->GameShared->LevelData[MapName]["game"]["banned_spawn_weapons"])
        BannedWeaponDrops.emplace_back(s);
    EnemyRoleWeapons= this->GameShared->LevelData[MapName]["enemy_weapons"].get<unordered_map<std::string, std::string>>();

    GameTiles.ReadMapDataFile("assets/maps/" + GameMode.GetCurrentLevelName() + "/map_data.csv");
    if (fs::exists(("assets/maps/" + GameMode.GetCurrentLevelName() + "/entities.csv").c_str()))
        GameTiles.ReadEntitiesFile("assets/maps/" + GameMode.GetCurrentLevelName() + "/entities.csv");

    MainPlayer = make_shared<Player>(GameTiles.PlayerSpawnPosition.x,
                                     GameTiles.PlayerSpawnPosition.y, this->GameShared->LevelData[MapName]["player"]["starting_speed"],
                                     GameResources.Textures["player"], *this);
    MainPlayer->MaxHealth = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    MainPlayer->Health = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    GameCamera.CameraPosition = Vector2Add(MainPlayer->GetCenter(), {
        (float)GetRandomValue(-100, 100),
        (float)GetRandomValue(-100, 100),
    });
    GameEntities.AddEntity(PlayerType, MainPlayer);
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
    EnemyRoleWeapons.clear();
    GameResources.Weapons.clear();
    BannedWeaponDrops.clear();
    GameMiscTools.Quit();
}