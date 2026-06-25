//
// Created by Rolpon on 6/11/2026.
//

#ifndef BOUNCINGPLUS_ENGINETOOLS_H
#define BOUNCINGPLUS_ENGINETOOLS_H

#include <vector>

#include "BaseProfiler.h"
#include "raylib.h"

struct WeaponPickup;
class Game;

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

class GameMisc
{
public:
    // Profiler
    BaseProfiler GameProfiler;
    std::vector<float> RecordedDeltaTimes;
    float AverageDeltaTime;
    float LastAverageDeltaTime;
    double LastStartedRecordingDelta;
    float LastDeltaTime;
    double StutterCooldown;
    int Stutters;
    bool DisplayProfiler;

    Game* game;
    GameMisc();
    GameMisc(Game* game);
    ~GameMisc();

    void SetGameData();

    void Update();
    void Clear();
    void Quit();

    std::vector<WeaponPickup> WeaponPickups;
    std::vector<std::pair<Rectangle, double>> FreezeZones;

    void PlaceWeaponPickup(WeaponPickup Pickup);
    void ProcessFreezeZones();
    void DisplayPickups();
    void DisplayProfilerInfo();

    RayCastData RayCastPoint(Vector2 Origin, Vector2 Target, bool Debug = false);
    bool RayCast(Vector2 Origin, Vector2 Target, bool Debug = false);
};


#endif //BOUNCINGPLUS_ENGINETOOLS_H