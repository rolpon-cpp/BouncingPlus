//
// Created by lalit on 3/14/2026.
//

#ifndef BOUNCINGPLUS_PLAYERUTILS_H
#define BOUNCINGPLUS_PLAYERUTILS_H
#include <memory>
#include <raylib.h>
#include <vector>
#include "Enemy.h"

struct ScoreChange;
class Player;

class PlayerLogicProcessor
{
public:

    std::weak_ptr<Player> Owner;

    int uTime = -1;
    int uWidth = -1;
    int uHeight = -1;

    float RankLevel = 0;
    std::vector<std::string> RankClassifications;

    double LayerSwitchCooldown = 0;
    float FightMusicLayerGoal = 0;
    float FightMusicLayer = 0;
    std::string PreviousFightTrack;

    std::vector<Vector3> DamageNotifs;

    float DashCooldown = 0;
    double DashTimeStart = 0;
    std::vector<std::weak_ptr<Enemy>> DashedEnemies;
    float PlayerDashLineThickness = 10;

    PlayerLogicProcessor();
    PlayerLogicProcessor(std::weak_ptr<Player> Owner);
    ~PlayerLogicProcessor();
    void Update();
    void IncreaseScore(std::string Reason, float Points);
    void DamageNotification(Vector2 From);
    void AttackDashedEnemy(std::shared_ptr<Enemy> entity, bool already_attacked);
    void PhysicsUpdate();
    void DashAttacking();
    void HandleFightMusic();
    void DashLogic();
    void RankLevelLogic();
    void ProcessStress();
    void DisplayDamageNotifs();
    void Destroy();
};


#endif //BOUNCINGPLUS_PLAYERUTILS_H