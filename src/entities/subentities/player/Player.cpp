//
// Created by Rolpon on 8/27/2025.
//

#include "Player.h"
#include <nlohmann/json.hpp>
#include "../../../game/Game.h"
#include "raylib.h"
#include <raymath.h>

#include "../../../game/managers/CameraManager.h"
#include "../../../game/managers/ParticleManager.h"
#include "../../../game/managers/SoundManager.h"
#include "../../../game/managers/ResourceManager.h"
#include "../../../game/managers/GameModeManager.h"
#include "../../../game/core/Controls.h"
#include "../../../game/core/SharedManager.h"

using namespace std;

// the player's name is Jones
// little bouncing ballz boy!
Player::Player(float X, float Y, float Speed, Texture2D& PlayerTexture, Game& game) : Entity(PlayerTexture,
    Rectangle{X - 18, Y - 18, 36, 36}, Speed, game)
{
    this->Type = PlayerType;
    this->Kills = 0;
    this->isInvincible = false;
    this->LastKills = 0;
    this->OrigSpeed = Speed;
    this->ExtraSpeed = 0;
    this->IntervalHealth = Health;
    this->LastInterval = game.GetGameTime();
    this->LastWarningSign = game.GetGameTime();
    this->HealthConcern = false;
    this->WarningSign = false;
    this->SpeedBuff = 0;
    this->LastTanked = 0;
    this->EnemyCombo = 0;
    this->LastKilledAnEnemy = game.GetGameTime();
    this->LastPos = Vector2{0, 0};
    this->InvincibilityResetTimer = 0;
    this->StressLevel = 0;
    this->FrameStressLevel = 0;
    this->EnemiesDetected = 0;

    this->EntityColor = Color{
        game.GameShared->Progress.Data.PlayerColor[0], game.GameShared->Progress.Data.PlayerColor[1],
        game.GameShared->Progress.Data.PlayerColor[2],
        game.GameShared->Progress.Data.PlayerColor[3]
    };
}

Player::Player()
{
}

Player::~Player()
{
}

void Player::ToggleInvincibility()
{
    this->isInvincible = !this->isInvincible;
}

void Player::PhysicsUpdate(float DeltaTime, double Time)
{
    float MovementX = 0;
    float MovementY = 0;
    if (PlayerFrozenTimer <= 0)
    {
        if (game->GameControls->IsControlDown("up"))
        {
            MovementY -= 1;
        }
        if (game->GameControls->IsControlDown("down"))
        {
            MovementY += 1;
        }
        if (game->GameControls->IsControlDown("left"))
        {
            MovementX -= 1;
        }
        if (game->GameControls->IsControlDown("right"))
        {
            MovementX += 1;
        }
    }
    if (Health >= 200 && Time - LastTanked >= 2)
    {
        Health -= 2;
        LastTanked = Time;
    }
    if (VelocityPower < 50)
    {
        VelocityPower = 0;
    }
    Movement = Vector2Normalize(Vector2{MovementX, MovementY});
    if (Vector2Distance(LastPos, {BoundingBox.x, BoundingBox.y}) > 0 && Vector2Distance({0, 0}, Movement) > 0)
    {
        LastMovedTime = Time;
        ExtraSpeed += 10 * DeltaTime;
    }
    LastPos = {BoundingBox.x, BoundingBox.y};
    if (Time - LastMovedTime > 1)
        ExtraSpeed = 0;
    if (ReduceSpeedBuff)
    {
        SpeedBuff -= 100 * DeltaTime;
        if (SpeedBuff <= 0)
        {
            SpeedBuff = 0;
            ReduceSpeedBuff = false;
        }
    }
    ExtraSpeed = min(ExtraSpeed, 400.0f);
    Speed = (OrigSpeed + ExtraSpeed + SpeedBuff);
    if ((Health / MaxHealth) > 2.0f)
        Speed *= (1.0f - min(((Health / MaxHealth) - 3.0f) / 2.0f, 0.5f));
    LogicProcessor.PhysicsUpdate();
    Entity::PhysicsUpdate(DeltaTime, Time);
}

void Player::OnWallVelocityBump(float Power)
{
    if (Power >= 400)
    {
        if (VelocityPower > 150 && !Dodging)
            game->GameSounds->PlayGameSound("dash_wall_hit", 0.25f);
        VelocityPower *= 0.45f;
        Entity::OnWallVelocityBump(Power);
        int ParticleAmount = round(Power / 600.0f);
        bool PURPLE_OR_BLUE = GetRandomValue(1, 2) == 1;
        if (ParticleAmount > 0)
            game->GameParticles->ParticleEffect({
                                                    GetCenter(),
                                                    Power / 1.5f,
                                                    PURPLE_OR_BLUE ? PURPLE : BLUE,
                                                    100,
                                                    Power / 400.0f,
                                                    1.0f,
                                                    PURPLE_OR_BLUE ? BLUE : PURPLE
                                                }, (180 - Vector2LineAngle(VelocityMovement, {0, 0}) * RAD2DEG), 15,
                                                ParticleAmount);
        if (Dodging)
            VelocityPower *= 0.5f;
        if (!isInvincible)
        {
            float PreviousH = Health;
            float Damage = Power / 500.0f;
            if (Health > 20)
                Health -= Damage;
            if (Health <= 20.0f && PreviousH > 20)
                Health = 20.0f;
        }
    }
}

void Player::PlayerControls()
{
    if (PlayerFrozenTimer <= 0)
    {
        // powerup logic
        if (game->GameControls->IsControlDown("powerup"))
        {
            MainPowerupSystem.Activate();
        }

        // firing logic
        if (IsMouseButtonDown(0) && MainWeaponsSystem.CurrentWeapon != nullptr &&
            MainWeaponsSystem.CurrentWeapon->Ammo > 0 && MainWeaponsSystem.WeaponAmmo[MainWeaponsSystem.
                CurrentWeaponIndex] <= 0 &&
            MainWeaponsSystem.AttackCooldowns[MainWeaponsSystem.CurrentWeaponIndex] >= MainWeaponsSystem.CurrentWeapon->
            Cooldown)
        {
            MainWeaponsSystem.Reload();
        }
        //Vector2 WorldMousePos = Vector2{0, 0};
        if ((IsMouseButtonDown(0) || game->GameControls->IsControlDown("attack_other")) && !IsPreparingForDash)
        {
            MainWeaponsSystem.Attack(GetScreenToWorld2D(GetMousePosition(), game->GameCamera->RaylibCamera));
        }

        // reload logic
        if (game->GameControls->IsControlPressed("reload") && MainWeaponsSystem.TimeStartedReloading == -1)
            MainWeaponsSystem.Reload();

        //dropping stuff
        if (game->GameControls->IsControlPressed("drop") && MainWeaponsSystem.CurrentWeaponIndex != -1)
            MainWeaponsSystem.DropWeapon(MainWeaponsSystem.Weapons[MainWeaponsSystem.CurrentWeaponIndex]);

        // inventory/item switch logic
        int LastWeaponIdx = MainWeaponsSystem.CurrentWeaponIndex;

        // inventory scrolling logic
        ScrollWheel -= GetMouseWheelMoveV().y;
        if (abs(ScrollWheel) > 1.85f)
        {
            if (MainWeaponsSystem.CurrentWeaponIndex == -1)
            {
                if (ScrollWheel < 0)
                    MainWeaponsSystem.Equip(2);
                if (ScrollWheel > 0)
                    MainWeaponsSystem.Equip(0);
            }
            else
            {
                int Idx = -1;
                if (ScrollWheel < 0)
                {
                    Idx = MainWeaponsSystem.CurrentWeaponIndex - 1;
                    if (Idx < 0)
                        Idx = 2;
                }
                if (ScrollWheel > 0)
                {
                    Idx = MainWeaponsSystem.CurrentWeaponIndex + 1;
                    if (Idx > 2)
                        Idx = 0;
                }
                if (Idx != -1)
                    MainWeaponsSystem.Equip(Idx);
            }
            ScrollWheel = 0;
        }

        // inventory input logic
        if (game->GameControls->IsControlPressed("item1"))
        {
            if (MainWeaponsSystem.CurrentWeaponIndex != 0)
            {
                MainWeaponsSystem.Equip(0);
            }
            else if (MainWeaponsSystem.CurrentWeaponIndex == 0)
            {
                MainWeaponsSystem.Unequip();
            }
        }
        if (game->GameControls->IsControlPressed("item2"))
        {
            if (MainWeaponsSystem.CurrentWeaponIndex != 1)
            {
                MainWeaponsSystem.Equip(1);
            }
            else if (MainWeaponsSystem.CurrentWeaponIndex == 1)
            {
                MainWeaponsSystem.Unequip();
            }
        }
        if (game->GameControls->IsControlPressed("item3"))
        {
            if (MainWeaponsSystem.CurrentWeaponIndex != 2)
            {
                MainWeaponsSystem.Equip(2);
            }
            else if (MainWeaponsSystem.CurrentWeaponIndex == 2)
            {
                MainWeaponsSystem.Unequip();
            }
        }

        if (MainWeaponsSystem.CurrentWeaponIndex != LastWeaponIdx)
            LastSwappedItem = game->GetGameTime();
    }
    else
    {
        PlayerFrozenTimer -= game->GetGameDeltaTime();
    }
}

void Player::SystemsInitCheck()
{
    // is the weapon system not initialized?? init it now!!!
    if (!this->SystemsInitialized)
    {
        this->LogicProcessor = PlayerLogicProcessor(dynamic_pointer_cast < Player > (shared_from_this()));
        this->MainWeaponsSystem = WeaponsSystem(shared_from_this(), *game);
        this->MainEffectsSystem = Effects(shared_from_this(), *game);
        this->MainPowerupSystem = PowerupSystem(dynamic_pointer_cast < Player > (shared_from_this()), *game);

        auto f = game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()]["player"]["inventory"];
        for (int i = 0; i < (int)min((float)f.size(), 3.0f); i++)
        {
            this->MainWeaponsSystem.Weapons[i] = f[i];
            this->MainWeaponsSystem.WeaponAmmo[i] = game->GameResources->Weapons[f[i]].Ammo;
        }
        if (game->GameResources->Powerups.count(
            game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()]["player"]["powerup"]))
        {
            MainPowerupSystem.SetPowerup(
                game->GameResources->Powerups[game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()][
                    "player"]["powerup"]]);
        }
        this->MainWeaponsSystem.Equip(0);
        this->SystemsInitialized = true;
    }
}

void Player::Update()
{
    SystemsInitCheck();

    if (Health > 1000)
        Health = 1000;

    ProcessWarningSign();

    // player transparency processing
    EntityColor = ColorAlpha({
                                 game->GameShared->Progress.Data.PlayerColor[0],
                                 game->GameShared->Progress.Data.PlayerColor[1],
                                 game->GameShared->Progress.Data.PlayerColor[2],
                                 game->GameShared->Progress.Data.PlayerColor[3]
                             }, Alpha);
    Alpha = Lerp(Alpha, (InvincibilityResetTimer > 0 ? 0.5f : 1.0f), 5.5f * game->GetGameDeltaTime());

    if (InvincibilityResetTimer > 0)
        InvincibilityResetTimer -= game->GetGameDeltaTime();
    if (InvincibilityResetTimer <= 0)
    {
        isInvincible = false;
        Dodging = false;
    }

    PlayerControls();

    DrawCircleGradient(GetCenter().x, GetCenter().y, BoundingBox.width / 1.25f, ColorAlpha(PURPLE, 0.5), BLANK);

    // update entity
    Entity::Update();
    MainWeaponsSystem.Update();
    MainPowerupSystem.Update();
    MainEffectsSystem.Update();
    LogicProcessor.Update();

    ProcessKills();
    EnemiesDetected = 0;
}

void Player::ProcessWarningSign()
{
    // warning sign interval
    if (game->GetGameTime() - LastInterval >= 1.5f)
    {
        HealthConcern = (IntervalHealth - Health) >= 75;
        LastInterval = game->GetGameTime();
    }
    if (Health < 50)
        HealthConcern = true;

    if (game->GetGameTime() - LastWarningSign >= 0.1f)
    {
        WarningSign = !WarningSign;
        LastWarningSign = game->GetGameTime();
    }
    if (Health > 0 && HealthConcern && WarningSign)
        DrawTexturePro(game->GameResources->Textures["warning"], {0, 0, 33, 34}, {
                           BoundingBox.x + BoundingBox.width / 2 + 12, BoundingBox.y - 24 - 10, 24, 24
                       }, {0, 0}, 0, WHITE);
}

void Player::ProcessKills()
{
    ComboTime = min(max(3.0f - 2.0f * (static_cast<float>(EnemiesDetected) / 10.0f), 1.15f), 3.0f);
    // did we get a kill?
    if (Kills != LastKills)
    {
        game->GameShared->Progress.Data.Money += 15.0f;
        game->GameSounds->PlayGameSound("death");
        ExtraSpeed += 14;

        std::string KillName = "Kill";
        float KillPoints = 100.0f;
        if (StressLevel >= 0.55f)
        {
            KillPoints += 25.0f;
            KillName += ", Stress";
        }

        if (game->GetGameTime() - LastKilledAnEnemy <= ComboTime)
        {
            EnemyCombo++;
            KillPoints *= 1.0f + (1.0f - static_cast<float>(game->GetGameTime() - LastKilledAnEnemy) / ComboTime) /
                2.0f;
            KillName += ", Combo x" + to_string(EnemyCombo);
        }
        else
            EnemyCombo = 0;

        StressLevel += 0.1f;
        if (StressLevel > 1.0f)
            StressLevel = 1.0f;

        KillPoints *= 1.0f + static_cast<float>(EnemyCombo) / 10.0f;
        LogicProcessor.IncreaseScore(KillName, KillPoints, ColorBrightness(RED, GetRandomValue(-300, 300) / 1000.0f));

        LastKilledAnEnemy = game->GetGameTime();
    }
    if (game->GetGameTime() - LastKilledAnEnemy > ComboTime)
        EnemyCombo = 0;
    LastKills = Kills;
}

void Player::OnDeath()
{
    if (!game->GameMode->GetCurrentLevelName().empty() && !game->GameShared->LevelData[game->GameMode->
        GetCurrentLevelName()]["music"].get<string>().empty())
    {
        for (int i = 1; i < 5; i++)
        {
            std::string FightTrack = game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()]["music"].get<
                string>() + "_layer" + to_string(i);
            game->GameSounds->StopGameMusic(FightTrack, true);
        }
    }
    Entity::OnDeath();
}

void Player::OnDelete()
{
    ScoreChanges.clear();
    MainWeaponsSystem.Unequip();
    LogicProcessor.Destroy();
    MainEffectsSystem.Cleanup();
    Entity::OnDelete();
}