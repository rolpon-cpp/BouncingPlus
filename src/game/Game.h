//
// Created by lalit on 8/26/2025.
//

#ifndef BOUNCINGPLUS_GAME_H
#define BOUNCINGPLUS_GAME_H

#include <raylib.h>
#include "core/BaseProfiler.h"
#include "managers/GameModeManager.h"
#include "managers/CameraManager.h"
#include "../entities/Entity.h"
#include "managers/EntityManager.h"
#include "managers/ParticleManager.h"
#include "../entities/subentities/player/Player.h"
#include "managers/SoundManager.h"
#include "../level/tiles/TileManager.h"
#include "ui/gameplay_ui/GameplayUI.h"
#include "../entities/systems/Weapons.h"
#include "managers/ResourceManager.h"
#include "core/SharedManager.h"
#include "core/GameMisc.h"

class Game {

    float FreezeTime;
    float MaxFreezeTime;

    GameplayUI GameUI;
    void SetGameData();
    void ProcessSlowdownAnimation();

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
        float GameScore;
        float FinalLevelCompletionScore;

        // Managers
        TileManager GameTiles;
        EntityManager GameEntities;
        CameraManager GameCamera;
        ParticleManager GameParticles;
        SoundManager GameSounds;
        GameModeManager GameMode;
        ResourceManager GameResources;
        GameMisc GameMiscTools;

        shared_ptr<Player> MainPlayer;

        // Extra Assets
        std::unordered_map<std::string, std::string> EnemyRoleWeapons;
        std::vector<std::string> BannedWeaponDrops;

        Game(SharedManager& Shared);

        // Base game functions
        void Freeze(float Time);
        void Reload(std::string Filename);
        void Update();
        void Clear();
        void Quit();
        float GetGameDeltaTime();
        double GetGameTime();
};


#endif //BOUNCINGPLUS_GAME_H