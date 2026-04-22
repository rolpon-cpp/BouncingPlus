//
// Created by lalit on 3/19/2026.
//

#ifndef BOUNCINGPLUS_TURRET_H
#define BOUNCINGPLUS_TURRET_H

#include "raylib.h"
#include "string"
#include "../Entity.h"
#include "../systems/Weapons.h"

class Game;

enum TurretState
{
    LOOKING,
    DETECTED,
    SUSPICIOUS
};

class Turret : public Entity
{
    bool Initialized;

    float TurretRotation;
    float RotationSpeed;

    float LookingRotationGoal;
    double GoalSwitchCooldown;

    double SuspiciousRotationLockCooldown;

    float Range;
    float AngleRange;

    void ChangeState(TurretState NewState);
    void SetTarget();
    bool PlayerIsVisible();

    public:
    TurretState CurrentState;
    Vector2 Target;
    WeaponsSystem MyWeaponsSystem;
    std::string Weapon;
    Turret(Game &game, std::string Weapon, float X, float Y);
    Turret();
    ~Turret()override;
    void Update()override;

};


#endif //BOUNCINGPLUS_TURRET_H