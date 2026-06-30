//
// Created by Rolpon on 8/28/2025.
//

#ifndef BOUNCINGPLUS_ENEMY_H
#define BOUNCINGPLUS_ENEMY_H
#include "../../Entity.h"
#include "../../systems/Effects.h"
#include "../../systems/Weapons.h"
#include "behaviors/EnemyBehavior.h"


class Enemy : public Entity
{
    float AnimatedHealth;
    float TotalHealth = -1;

    Vector2 WanderPos;
    double LastSetWanderPos = 0;
    double WanderingCooldown;

    bool WeaponsSystemInit = false;

    bool LastFreezingState = false;

    Vector2 WallMovement;

    float EnemyTransparency = 0;

    void Init(float Health, float Speed, float Armor, std::string Weapon, std::unique_ptr<EnemyBehavior> EnemyBehavior,
              Game& game);

public:
    bool RenderHealthBar = true;
    bool WanderingEnabled = true;

    std::string MyWeapon;
    std::unique_ptr<EnemyBehavior> Behavior = nullptr;

    WeaponsSystem MainWeaponsSystem;
    Effects MainEffectsSystem;

    float Armor;
    float HealthRegenRate;

    float AngeredRangeBypassTimerMax;
    float RemainingHealthOfOriginalHealth = 0;
    float AngeredRangeBypassTimer;

    Enemy(float X, float Y, float Health, float Speed, float Armor, std::string Weapon, Texture2D& EnemyTexture,
          Game& game);
    Enemy(float X, float Y, float Health, float Speed, float Armor, std::string Weapon,
          std::unique_ptr<EnemyBehavior> EnemyBehavior, Texture2D& EnemyTexture, Game& game);
    Enemy();
    ~Enemy() override;
    void Update() override;
    void MoveAwayFromWalls();
    void Wander();
    void OnDelete() override;
    void OnDeath() override;
};


#endif