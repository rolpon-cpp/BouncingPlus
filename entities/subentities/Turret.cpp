//
// Created by lalit on 3/19/2026.
//

#include "Turret.h"

#include <iostream>
#include <raymath.h>

#include "../systems/Weapons.h"
#include "../../game/Game.h"

Turret::Turret(Game& game, std::string Weapon, float X, float Y) : Entity(game.GameResources.Textures["turret"],
                                                      Rectangle{X - 24, Y - 24, 48, 48}, 0, game)
{
    this->Weapon = Weapon;
    this->Type = TurretType;
    this->Target = {BoundingBox.x, BoundingBox.y};
    this->CurrentState = LOOKING;
    this->Initialized = false;
    this->TurretRotation = GetRandomValue(0, 360);
    this->RotationSpeed = GetRandomValue(50, 100);
    this->LookingRotationGoal = GetRandomValue(0, 360);
    this->GoalSwitchCooldown = 0.5f;
    this->SuspiciousRotationLockCooldown = 2.5f;
    this->Range = GetRandomValue(600, 1200);
    this->AngleRange = GetRandomValue(35, 70);
}

void Turret::SetTarget()
{
    Target = Vector2Add(GetCenter(), Vector2{
        cos(TurretRotation * (2 * PI / 360)) * this->Range, sin(TurretRotation * (2 * PI / 360)) * this->Range
    });
}

void Turret::ChangeState(TurretState NewState)
{
    TurretState OldState = this->CurrentState;
    this->CurrentState = NewState;
    if (OldState == SUSPICIOUS)
    {
        this->SuspiciousRotationLockCooldown = 2.5f;
    } else if (OldState == LOOKING)
    {
        this->LookingRotationGoal = GetRandomValue(0, 360);
        this->GoalSwitchCooldown = 0.5f;
    } else if (OldState == DETECTED)
    {
        SetTarget();
    }
}

Turret::Turret()
{
}

Turret::~Turret()
{
}

bool Turret::PlayerIsVisible()
{
    float plrAngle = Vector2LineAngle(GetCenter(), game->MainPlayer->GetCenter()) * RAD2DEG;
    float targAngle = Vector2LineAngle(GetCenter(), Target) * RAD2DEG;
    return plrAngle >= targAngle - AngleRange/2 && plrAngle <= targAngle + AngleRange/2 && game->RayCast(GetCenter(), game->MainPlayer->GetCenter()) && Vector2Distance(GetCenter(), game->MainPlayer->GetCenter()) <= Range;
}

void Turret::Update()
{
    if (!Initialized)
    {
        MyWeaponsSystem = WeaponsSystem(shared_from_this(), *game);
        MyWeaponsSystem.GiveWeapon(this->Weapon);
        MyWeaponsSystem.Equip(0);
        Initialized = true;
    }

    float skip = 2.5f;
    for (int i = 0; i < AngleRange / skip; i++)
    {
        float Angle = TurretRotation - AngleRange/2 + (i * skip);
        float X = cos(Angle * (2 * PI / 360))*Range;
        float Y = sin(Angle * (2 * PI / 360))*Range;
        auto p = game->RayCastPoint(GetCenter(),Vector2Add(GetCenter(), Vector2{X,Y}));
        if (Vector2Distance(GetCenter(), game->MainPlayer->GetCenter()) < Range+GetRenderWidth()/2)
            DrawCircleSector(GetCenter(), Vector2Distance(GetCenter(),p.HitPosition), Angle - skip/2, Angle + skip/2, skip, ColorAlpha(RED,0.35f));
        if (Vector2Distance(GetCenter(), p.HitPosition) <= 200 && GoalSwitchCooldown <= 0)
        {
            this->LookingRotationGoal = TurretRotation + GetRandomValue(-180,180);
            GoalSwitchCooldown = 0.5f;
        }
    }
    switch (this->CurrentState)
    {
        case LOOKING:
            {
                if (abs(this->TurretRotation - this->LookingRotationGoal) < 5)
                {
                    TurretRotation = this->LookingRotationGoal;
                    GoalSwitchCooldown -= game->GetGameDeltaTime();
                    if (GoalSwitchCooldown <= 0)
                    {
                        this->LookingRotationGoal = TurretRotation + GetRandomValue(-180,180);
                        GoalSwitchCooldown = 0.5f;
                    }
                } else
                {
                    float direction = (this->TurretRotation - this->LookingRotationGoal) / abs(this->TurretRotation - this->LookingRotationGoal);
                    this->TurretRotation -= direction * this->RotationSpeed * game->GetGameDeltaTime();
                }
                SetTarget();

                if (PlayerIsVisible())
                    ChangeState(DETECTED);
                break;
            }
        case DETECTED:
            {
                this->TurretRotation = 180 - Vector2LineAngle(GetCenter(), Target) * RAD2DEG+180;
                Target = Vector2Lerp(Target, game->MainPlayer->GetCenter(), 6.5f * game->GetGameDeltaTime());
                MyWeaponsSystem.Attack(Target);
                if (MyWeaponsSystem.WeaponAmmo[MyWeaponsSystem.CurrentWeaponIndex] <= 0)
                    MyWeaponsSystem.Reload();
                if (!PlayerIsVisible())
                    ChangeState(SUSPICIOUS);
                break;
            }
    case SUSPICIOUS:
            {
                this->TurretRotation = 180 - Vector2LineAngle(GetCenter(), Target) * RAD2DEG+180;
                SuspiciousRotationLockCooldown -= game->GetGameDeltaTime();
                if (PlayerIsVisible())
                    ChangeState(DETECTED);
                else if (SuspiciousRotationLockCooldown <= 0)
                    ChangeState(LOOKING);
                break;
            }

    }

    Rotation=TurretRotation;

    Entity::Update();
    MyWeaponsSystem.Update();
}
