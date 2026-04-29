//
// Created by lalit on 1/23/2026.
//

#include "Powerups.h"

#include <iostream>

#include "../../game/Game.h"
#include "../subentities/Bullet.h"
#include "../subentities/Player.h"

#include <raylib.h>
#include <raymath.h>


void Powerup::Complete(std::shared_ptr<Player> Owner)
{
}

void Powerup::Undo(std::shared_ptr<Player> Owner)
{
}

void SpeedPowerup::Complete(std::shared_ptr<Player> Owner)
{
    Powerup::Complete(Owner);
    Owner->ReduceSpeedBuff = false;
    Owner->SpeedBuff += (250.0f - (40.0f * Owner->MainPowerupSystem.CurrentLength)) * Owner->game->GetGameDeltaTime();
}

void SpeedPowerup::Undo(std::shared_ptr<Player> Owner)
{
    Powerup::Undo(Owner);
    Owner->ReduceSpeedBuff = true;
    Owner->SpeedBuff -= 100;
}

SpeedPowerup::SpeedPowerup()
{
    Cooldown = 15;
    Length = 5;
    Name = "Speed";
}

ShieldPowerup::ShieldPowerup()
{
    Cooldown = 30;
    Length = 15;
    Name = "B-Shield";
}

void ShieldPowerup::Complete(std::shared_ptr<Player> Owner)
{
    Powerup::Complete(Owner);

    TransBuff = Lerp(TransBuff, 0.0f, Owner->game->GetGameDeltaTime() * LerpSpeed);
    float circle_transparency = min(Length - Owner->MainPowerupSystem.CurrentLength, DefaultTrans);
    if (Owner->MainPowerupSystem.CurrentLength <= DefaultTrans)
        circle_transparency = Owner->MainPowerupSystem.CurrentLength;
    else
        circle_transparency += TransBuff;

    std::vector<shared_ptr<Entity>>* array = &Owner->game->GameEntities.Entities[BulletType];
    for (int i = 0; i < array->size(); i++) {
        if (shared_ptr<Bullet> entity = dynamic_pointer_cast<Bullet>(array->at(i)); entity != nullptr && !entity->ShouldDelete) {
            float dist = Vector2Distance({Owner->BoundingBox.x + Owner->BoundingBox.width / 2, Owner->BoundingBox.y + Owner->BoundingBox.height / 2},
                    {entity->BoundingBox.x + entity->BoundingBox.width / 2, entity->BoundingBox.y + entity->BoundingBox.height / 2});
            auto p = entity->OwnerPtr.lock();
            if (dist <= FieldSize && p != Owner)
            {
                entity->RotGoal = -(180 - (Vector2LineAngle({Owner->BoundingBox.x + Owner->BoundingBox.width / 2, Owner->BoundingBox.y + Owner->BoundingBox.height / 2},
                    {entity->BoundingBox.x + entity->BoundingBox.width / 2, entity->BoundingBox.y + entity->BoundingBox.height / 2}) * RAD2DEG));

                float X = cos(entity->RotGoal * (2 * PI / 360))*100;
                float Y = sin(entity->RotGoal * (2 * PI / 360))*100;

                entity->Movement = Vector2{X, Y};

                TransBuff = 1.0f - DefaultTrans;
            }
        }
    }

    displayFieldSize = Lerp(displayFieldSize,FieldSize,Owner->game->GetGameDeltaTime() * LerpSpeed);
    for (int i = 0; i < ShieldThickness; i++)
    {
        DrawCircleLines(round(Owner->BoundingBox.x + Owner->BoundingBox.width / 2), round(Owner->BoundingBox.y + Owner->BoundingBox.height / 2), displayFieldSize/2 - i, ColorAlpha(YELLOW, circle_transparency));
    }
}

void ShieldPowerup::Undo(std::shared_ptr<Player> Owner)
{
    Powerup::Undo(Owner);
    displayFieldSize = 0;
    TransBuff = 0;
}

FreezePowerup::FreezePowerup()
{
    Cooldown = 30;
    Length = 0;
    Name = "Ice Trap";
}

void FreezePowerup::Complete(std::shared_ptr<Player> Owner)
{
    auto r = Owner->game->RayCastPoint(Owner->GetCenter(), GetScreenToWorld2D(GetMousePosition(), Owner->game->GameCamera.RaylibCamera));
    float w = GetRandomValue(250, 300);
    float h = GetRandomValue(250, 300);
    Rectangle rec = {r.HitPosition.x - w/2, r.HitPosition.y - h/2, w, h};
    Owner->game->FreezeZones.push_back(std::pair(rec, Owner->game->GetGameTime()));

    Powerup::Complete(Owner);
}

void FreezePowerup::Undo(std::shared_ptr<Player> Owner)
{
    Powerup::Undo(Owner);
}

TankyPowerup::TankyPowerup()
{
    Cooldown = 30;
    Length = 20;
    Name = "Tank";
}

void TankyPowerup::Complete(std::shared_ptr<Player> Owner)
{
    Owner->ExtraSpeed = -Owner->Speed * 0.75f;
    Owner->game->GameCamera.QuickZoom(1.5f, 0.1f);
    if (Owner->MainWeaponsSystem.CurrentWeaponIndex != -1)
        Owner->MainWeaponsSystem.AttackCooldowns[Owner->MainWeaponsSystem.CurrentWeaponIndex] -= 0.5f * Owner->game->GetGameDeltaTime();
    if (!Owner->isInvincible)
        Owner->ToggleInvincibility();
    Powerup::Complete(Owner);
}

void TankyPowerup::Undo(std::shared_ptr<Player> Owner)
{
    Owner->ExtraSpeed = 0;
    Owner->SpeedBuff = 250;
    if (Owner->isInvincible)
        Owner->ToggleInvincibility();
    Powerup::Undo(Owner);
}

void PowerupSystem::Activate()
{
    auto Player = Owner.lock();
    if (Player == nullptr)
        return;
    if (CurrentPowerup != nullptr && CurrentCooldown <= 0 && CurrentLength <= 0)
    {
        if (Player->Type == PlayerType)
            Player->LogicProcessor.IncreaseScore("Powerup Use, " + CurrentPowerup->Name, 25.0f, YELLOW);
        CurrentCooldown = CurrentPowerup->Cooldown;
        CurrentLength = CurrentPowerup->Length;
        PowerupIsActive = true;
    }
}

void PowerupSystem::SetPowerup(Powerup* Powerup)
{
    auto Player = Owner.lock();
    if (Player == nullptr)
        return;
    if (this->CurrentPowerup != nullptr)
        this->CurrentPowerup->Undo(Player);
    this->CurrentPowerup = Powerup;
    PowerupIsActive = false;
    CurrentCooldown = 0;
    CurrentLength = 0;
}

void PowerupSystem::Update()
{
    auto Player = Owner.lock();
    if (Player == nullptr)
        return;
    if (CurrentPowerup != nullptr)
    {
        if (PowerupIsActive)
        {
            CurrentPowerup->Complete(Player);
            CurrentLength -= game->GetGameDeltaTime();
            if (CurrentLength <= 0)
            {
                CurrentPowerup->Undo(Player);
                PowerupIsActive = false;
                CurrentCooldown = CurrentPowerup->Cooldown;
            }
        } else
        {
            CurrentCooldown -= game->GetGameDeltaTime();
        }
    }
}

PowerupSystem::PowerupSystem(std::shared_ptr<Player> Owner, Game& game)
{
    this->game = &game;
    this->Owner = Owner;
}

PowerupSystem::PowerupSystem()
{
}

PowerupSystem::~PowerupSystem()
{
}
