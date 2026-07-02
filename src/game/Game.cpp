#include "Game.h"
#include "ui/gameplay_ui/GameplayUI.h"
#include <filesystem>
#include "../level/LevelLoader.h"
#include "core/BaseProfiler.h"
#include "managers/GameModeManager.h"
#include "managers/CameraManager.h"
#include "../entities/Entity.h"
#include "managers/EntityManager.h"
#include "managers/ParticleManager.h"
#include "../entities/subentities/player/Player.h"
#include "managers/SoundManager.h"
#include "../level/tiles/TileManager.h"
#include "managers/ResourceManager.h"
#include "core/SharedManager.h"
#include "core/GameMisc.h"

namespace fs = std::filesystem;

using namespace std;

Game::~Game() = default;

Game::Game(SharedManager& Shared, LoadingStage* LoadingStagePtr)
{
    this->GameShared = &Shared;
    GameControls = &GameShared->Controls;

    CurrentLoadingStage.stage = 0;
    *LoadingStagePtr = CurrentLoadingStage;

    // init game services
    GameUI = make_unique<GameplayUI>(this);
    GameTiles = make_unique<TileManager>(this);
    GameParticles = make_unique<ParticleManager>(this);
    GameCamera = make_unique<CameraManager>(this);
    GameEntities = make_unique<EntityManager>(this);
    GameMode = make_unique<GameModeManager>(this);
    GameMiscTools = make_unique<GameMisc>(this);
    GameResources = make_unique<ResourceManager>(this);

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

    // Loading phase
    CurrentLoadingStage.stage = 1;
    CurrentLoadingStage.start_time = GetTime();
    *LoadingStagePtr = CurrentLoadingStage;

    // Loading asset managers
    GameResources->Load(LoadingStagePtr);
    GameSounds = make_unique<SoundManager>(this, LoadingStagePtr);

    // Final loading phase
    CurrentLoadingStage.stage = 2;
    *LoadingStagePtr = CurrentLoadingStage;
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

void Game::Freeze(float Time)
{
    FreezeTime = Time;
    MaxFreezeTime = Time;
}

void Game::ProcessSlowdownAnimation()
{
    if (FreezeTime > 0 && MaxFreezeTime > 0)
    {
        GameSpeed = 0.0f;
        FreezeTime -= GetFrameTime();
    }
    else
    {
        FreezeTime = 0;
        MaxFreezeTime = 0;
        GameSpeed = GameMode->LevelGameSpeed;
    }
}

void Game::Update()
{
    if (this->GameControls->IsControlPressed("pause"))
        Paused = !Paused;
    if (!IsWindowFocused())
        Paused = true;

    if (!Paused)
    {
#ifndef PLATFORM_WEB
        if (!IsCursorHidden())
            HideCursor();
#endif

        if (this->GameControls->IsControlPressed("debug") && GameShared->DevMode)
            DebugDraw = !DebugDraw;
        if (!GameShared->DevMode)
            DebugDraw = false;

        GameTime += GetGameDeltaTime();

        if (this->GameControls->IsControlPressed("level_restart_or_finish"))
        {
            if (GameMode->WonLevel)
            {
                isReturning = true;
                if (!GameMode->GetCurrentLevelName().empty() && !this->GameShared->LevelData[GameMode->
                    GetCurrentLevelName()]["music"].get<string>().empty())
                {
                    for (int i = 1; i < 5; i++)
                    {
                        std::string FightTrack = this->GameShared->LevelData[GameMode->GetCurrentLevelName()]["music"].
                            get<string>() + "_layer" + to_string(i);
                        GameSounds->StopGameMusic(FightTrack, true);
                    }
                }
            }
            else if ((MainPlayer->Health <= 0 || MainPlayer->ShouldDelete) && !GameMode->GetCurrentLevelName().empty())
            {
                Reload(GameMode->GetCurrentLevelName());
                GameUI->StartingBlackScreenTrans = 0.0f;
                GameUI->EndBlackScreenTrans = 0.0f;
            }
        }

        GameCamera->Begin();

        GameMiscTools->GameProfiler.ProfilerLog("misc");
        ProcessSlowdownAnimation();
        GameMiscTools->Update();

        GameMiscTools->GameProfiler.ProfilerLog("tiles");
        GameTiles->Update();

        GameMiscTools->GameProfiler.ProfilerLog("particles");
        GameParticles->Update();

        GameMiscTools->GameProfiler.ProfilerLog("entities");
        GameEntities->Update();

        GameMiscTools->GameProfiler.ProfilerLog("sound");
        GameSounds->Update();

        GameMiscTools->GameProfiler.ProfilerLog("gamemode");
        GameMode->Update();

        GameMiscTools->GameProfiler.StopLog();

        GameCamera->End();
    }
#ifndef PLATFORM_WEB
    else if (IsCursorHidden())
        ShowCursor();
#endif

    GameCamera->Display(GameSpeed == 0.0f);

    GameUI->GameUI();

    GameMiscTools->DisplayProfilerInfo();

    if (isReturning)
    {
        GameUI->StartingBlackScreenTrans = 0;
        GameUI->EndBlackScreenTrans += 0.65f * GetFrameTime();
        GameTiles->Lines.clear();
        GameTiles->PrevFileName.clear();
        if (GameUI->EndBlackScreenTrans >= 0.9f)
            ShouldReturn = true;
    }
    else
        GameUI->EndBlackScreenTrans = 0;
    if (ShouldReturn)
    {
        GameUI->StartingBlackScreenTrans = 1.0f;
        GameUI->EndBlackScreenTrans = 0.0f;
    }
}

void Game::Clear()
{
    Paused = false;
    ShouldReturn = false;
    isReturning = false;

    GameTime = 0;
    FinalLevelCompletionScore = 0;
    GameScore = 0;

    GameEntities->Clear();
    BannedWeaponDrops.clear();
    EnemyRoleWeapons.clear();
    GameTiles->Clear();
    GameMiscTools->Clear();
    GameParticles->Clear();
    GameSounds->Clear();
    GameCamera->Clear();
    GameMode->Clear();
    GameUI->Clear();
    MainPlayer.reset();
    GameSounds->ClearCache();
}

void Game::ReloadInfinite()
{
    Clear();

    std::string MapName = "Infinite";

    GameMode->PrepareForInfiniteMode();

    for (std::string s : this->GameShared->LevelData[MapName]["game"]["banned_spawn_weapons"])
        BannedWeaponDrops.emplace_back(s);
    EnemyRoleWeapons = this->GameShared->LevelData[MapName]["enemy_weapons"].get<unordered_map<
        std::string, std::string>>();

    GameTiles->PrepareAsInfiniteMode();

    MainPlayer = make_shared<Player>(GameTiles->PlayerSpawnPosition.x,
                                     GameTiles->PlayerSpawnPosition.y,
                                     this->GameShared->LevelData[MapName]["player"]["starting_speed"],
                                     GameResources->Textures["player"], *this);
    MainPlayer->MaxHealth = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    MainPlayer->Health = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    GameCamera->CameraPosition = Vector2Add(MainPlayer->GetCenter(), {
                                                (float)GetRandomValue(-100, 100),
                                                (float)GetRandomValue(-100, 100),
                                            });
    GameEntities->AddEntity(PlayerEntityType, MainPlayer);
}

void Game::Reload(std::string MapName)
{
    Clear();

    GameMode->PrepareGameMode(this->GameShared->LevelData[MapName], MapName);

    for (std::string s : this->GameShared->LevelData[MapName]["game"]["banned_spawn_weapons"])
        BannedWeaponDrops.emplace_back(s);
    EnemyRoleWeapons = this->GameShared->LevelData[MapName]["enemy_weapons"].get<unordered_map<
        std::string, std::string>>();

    GameTiles->ReadMapDataFile("assets/maps/" + GameMode->GetCurrentLevelName() + "/map_data.csv");
    if (fs::exists(("assets/maps/" + GameMode->GetCurrentLevelName() + "/entities.csv").c_str()))
        GameTiles->ReadEntitiesFile("assets/maps/" + GameMode->GetCurrentLevelName() + "/entities.csv");

    MainPlayer = make_shared<Player>(GameTiles->PlayerSpawnPosition.x,
                                     GameTiles->PlayerSpawnPosition.y,
                                     this->GameShared->LevelData[MapName]["player"]["starting_speed"],
                                     GameResources->Textures["player"], *this);
    MainPlayer->MaxHealth = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    MainPlayer->Health = this->GameShared->LevelData[MapName]["player"]["starting_health"];
    GameCamera->CameraPosition = Vector2Add(MainPlayer->GetCenter(), {
                                                (float)GetRandomValue(-100, 100),
                                                (float)GetRandomValue(-100, 100),
                                            });
    GameEntities->AddEntity(PlayerEntityType, MainPlayer);
}

void Game::Quit()
{
    Clear();

    EnemyRoleWeapons.clear();
    GameResources->Weapons.clear();
    BannedWeaponDrops.clear();

    GameEntities->Quit();
    GameTiles->Quit();
    GameUI->Quit();
    GameParticles->Quit();
    GameCamera->Quit();
    GameSounds->Quit();
    GameResources->Quit();
    GameMode->Quit();
    GameMiscTools->Quit();

    GameEntities.reset();
    GameTiles.reset();
    GameUI.reset();
    GameParticles.reset();
    GameCamera.reset();
    GameSounds.reset();
    GameResources.reset();
    GameMode.reset();
    GameMiscTools.reset();
}