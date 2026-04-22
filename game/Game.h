//
// Created by lalit on 8/26/2025.
//

#ifndef BOUNCINGPLUS_GAME_H
#define BOUNCINGPLUS_GAME_H

#include <raylib.h>
#include <nlohmann/json_fwd.hpp>

#include "core/Profiler.h"
#include "managers/GameModeManager.h"
#include "managers/CameraManager.h"
#include "../entities/Entity.h"
#include "managers/EntityManager.h"
#include "managers/ParticleManager.h"
#include "../entities/subentities/Player.h"
#include "managers/SoundManager.h"
#include "../level/TileManager.h"
#include "ui/UIManager.h"
#include "../entities/systems/Weapons.h"
#include "managers/ResourceManager.h"
#include "core/SharedManager.h"

using namespace std;

struct Vector2i
{
    int x;
    int y;
};

struct RayCastData
{
    bool HitAir;
    Vector2 HitPosition;
    int HitTile;
};

class Game {

    float SlowdownTime;
    float MaxSlowdownTime;
    float SlowdownShakeIntensity;

    UIManager GameUI;
    Profiler GameProfiler;
    void SetGameData();
    void ProcessSlowdownAnimation();
    void DisplayPickups();
    void DisplayProfilerInfo();

    RenderTexture2D WeaponPickupTex;
    int uTextureSize;
    int uOutlineSize;
    int uOutlineColor;
    int uThreshold;

    std::vector<float> RecordedDeltaTimes;
    float AverageDeltaTime;
    float LastAverageDeltaTime;
    double LastStartedRecordingDelta;
    float LastDeltaTime;

    double StutterCooldown;
    int Stutters;

    public:
        SharedManager* GameShared;
        Controls* GameControls;

        // Timing, Speed, and Menu Management
        double GameTime;
        float GameSpeed;
        bool Paused;
        bool isReturning;
        bool ShouldReturn;
        bool DebugDraw;
        bool DisplayProfiler;
        float GameScore;

        // Level Information
        std::string CurrentLevelName;
        std::map<std::string, json> LevelData;

        // Managers
        TileManager GameTiles;
        EntityManager GameEntities;
        CameraManager GameCamera;
        ParticleManager GameParticles;
        SoundManager GameSounds;
        GameModeManager GameMode;
        ResourceManager GameResources;

        shared_ptr<Player> MainPlayer;
        std::vector<WeaponPickup> WeaponPickups;
        std::vector<std::pair<Rectangle, double>> FreezeZones;

        // Extra Assets
        unordered_map<std::string, std::string> EnemyRoleWeapons;
        std::vector<std::string> BannedWeaponDrops;

        Game(SharedManager& Shared);
        void PlaceWeaponPickup(WeaponPickup Pickup);
        void ProcessFreezeZones();
        void Slowdown(float Time);
        void Slowdown(float Time, float ShakeIntensity);
        void Reload(std::string Filename);
        void Update();
        void Clear();
        void Quit();
        void UnloadAssets();
        float GetGameDeltaTime();
        double GetGameTime();

        RayCastData RayCastPoint(Vector2 Origin, Vector2 Target, bool Debug = false);
        bool RayCast(Vector2 Origin, Vector2 Target, bool Debug = false);

};


#endif //BOUNCINGPLUS_GAME_H