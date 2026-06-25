#ifndef BOUNCINGPLUS_GAME_H
#define BOUNCINGPLUS_GAME_H

class Player;
class GameplayUI;
class SharedManager;
class Controls;
class TileManager;
class EntityManager;
class CameraManager;
class ParticleManager;
class SoundManager;
class GameModeManager;
class ResourceManager;
class GameMisc;

#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

class Game
{
    float FreezeTime;
    float MaxFreezeTime;

    std::unique_ptr<GameplayUI> GameUI;
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
    std::unique_ptr<TileManager> GameTiles;
    std::unique_ptr<EntityManager> GameEntities;
    std::unique_ptr<CameraManager> GameCamera;
    std::unique_ptr<ParticleManager> GameParticles;
    std::unique_ptr<SoundManager> GameSounds;
    std::unique_ptr<GameModeManager> GameMode;
    std::unique_ptr<ResourceManager> GameResources;
    std::unique_ptr<GameMisc> GameMiscTools;

    std::shared_ptr<Player> MainPlayer;

    // Extra Assets
    std::unordered_map<std::string, std::string> EnemyRoleWeapons;
    std::vector<std::string> BannedWeaponDrops;

    Game(SharedManager& Shared);
    ~Game();

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