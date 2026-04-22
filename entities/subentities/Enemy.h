//
// Created by lalit on 8/28/2025.
//

#ifndef BOUNCINGPLUS_ENEMY_H
#define BOUNCINGPLUS_ENEMY_H
#include "../Entity.h"
#include "../systems/Weapons.h"
#include "behaviors/EnemyBehavior.h"



class Enemy : public Entity {
    float AnimatedHealth;
    std::string MyWeapon;
    Vector2 WanderPos;
    float TotalHealth = -1;
double LastSetWanderPos = 0;
    double WanderingCooldown;
    bool weaponsSystemInit = false;
    bool isActive = false;
    double ActivationTimer;
    Vector2 WallMovement;
    float Alpha = 0;
    void Init(float X, float Y, float Health, float Speed, float Armor, std::string Weapon, std::unique_ptr<EnemyBehavior> EnemyBehavior, Texture2D& EnemyTexture, Game &game);
public:
    std::unique_ptr<EnemyBehavior> Behavior = nullptr;
    WeaponsSystem weaponsSystem;
    float AngeredRangeBypassTimerMax;
    float RemainingHealthOfOriginalHealth = 0;
    float AngeredRangeBypassTimer;
    float Armor;
    float HealthRegenRate;
    bool WanderingEnabled;
    Enemy(float X, float Y, float Health, float Speed, float Armor, std::string Weapon, Texture2D& EnemyTexture, Game &game);
    Enemy(float X, float Y, float Health, float Speed, float Armor, std::string Weapon, std::unique_ptr<EnemyBehavior> EnemyBehavior, Texture2D& EnemyTexture, Game &game);
    Enemy();
    ~Enemy()override;
    void Update() override;
    void MoveAwayFromWalls();
    void Wander();
    void OnDelete() override;
    void OnDeath()override;
};


#endif