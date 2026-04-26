//
// Created by lalit on 8/27/2025.
//

#include "Player.h"
#include <nlohmann/json.hpp>
#include "../../game/Game.h"
#include "raylib.h"
#include <raymath.h>

using namespace std;

// the player's name is Jones
// little bouncing ballz boy!
Player::Player(float X, float Y, float Speed, Texture2D &PlayerTexture, Game &game) : Entity(PlayerTexture,
                                                                   Rectangle{X - 18, Y - 18, 36, 36}, Speed, game) {
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
}

Player::Player() {
}

Player::~Player() {

}

void Player::ToggleInvincibility() {
    this->isInvincible = !this->isInvincible;
}

void Player::PhysicsUpdate(float DeltaTime, double Time) {
    float MovementX = 0;
    float MovementY = 0;
    if (PlayerFrozenTimer <= 0) {
        if (game->GameControls->IsControlDown("up")) {
            MovementY -= 1;
        }
        if (game->GameControls->IsControlDown("down")) {
            MovementY += 1;
        }
        if (game->GameControls->IsControlDown("left")) {
            MovementX -= 1;
        }
        if (game->GameControls->IsControlDown("right")) {
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
    if (Vector2Distance(LastPos, {BoundingBox.x,BoundingBox.y}) > 0 && Vector2Distance({0,0}, Movement) > 0) {
        LastMovedTime = Time;
        ExtraSpeed += 10 * DeltaTime;
    }
    LastPos = {BoundingBox.x,BoundingBox.y};
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
    if ((Health/MaxHealth) > 2.0f)
        Speed *= (1.0f - min(((Health/MaxHealth)-3.0f) / 2.0f, 0.5f));
    LogicProcessor.PhysicsUpdate();
    Entity::PhysicsUpdate(DeltaTime, Time);
}

void Player::OnWallVelocityBump(float Power)
{
    if (Power >= 400)
    {
        float PreviousH = Health;
        Entity::OnWallVelocityBump(Power);
        game->GameSounds.PlayGameSound("dash_wall_hit");
        int ParticleAmount = round(Power / 500.0f);
        bool PURPLE_OR_BLUE = GetRandomValue(1,2)==1;
        if (ParticleAmount > 0)
            game->GameParticles.ParticleEffect({
                GetCenter(),
                Power / 1.5f,
                PURPLE_OR_BLUE ? PURPLE : BLUE,
                100,
                Power / 400.0f,
                1.0f,
                PURPLE_OR_BLUE ? BLUE : PURPLE
            }, (180 - Vector2LineAngle(VelocityMovement, {0,0})*RAD2DEG), 15, ParticleAmount);
        float Damage = Power / 390.0f;
        if (Health - Damage >= 20)
        {
            Health -= Damage;
        } else if (PreviousH >= 20)
        {
            Health = 20;
        }
    }
}


void Player::Update()
{
    // is the weapon system not initialized?? init it now!!!
    if (!this->SystemsInitialized) {
        this->LogicProcessor = PlayerLogicProcessor(dynamic_pointer_cast<Player>(shared_from_this()));
        this->MainWeaponsSystem = WeaponsSystem(shared_from_this(), *game);
        this->MainPowerupSystem = PowerupSystem(dynamic_pointer_cast<Player>(shared_from_this()), *game);
        auto f = game->LevelData[game->CurrentLevelName]["player"]["inventory"];
        for (int i = 0; i < (int)min((float)f.size(),3.0f); i++) {
            this->MainWeaponsSystem.Weapons[i] = f[i];
            this->MainWeaponsSystem.WeaponAmmo[i] = game->GameResources.Weapons[f[i]].Ammo;
        }
        if (game->GameResources.Powerups.count(game->LevelData[game->CurrentLevelName]["player"]["powerup"]))
        {
            MainPowerupSystem.SetPowerup(game->GameResources.Powerups[game->LevelData[game->CurrentLevelName]["player"]["powerup"]]);
        }
        this->MainWeaponsSystem.Equip(0);
        this->SystemsInitialized = true;
    }

    if (Health > 1000)
        Health = 1000;

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
    {
        DrawTexturePro(game->GameResources.Textures["warning"], {0,0,33,34},{BoundingBox.x + BoundingBox.width/2 + 12,BoundingBox.y - 24 - 10,24,24},{0,0},0,WHITE);
    }

    // player transparency processing
    EntityColor = ColorAlpha(WHITE, Alpha);
    Alpha = Lerp(Alpha, (InvincibilityResetTimer > 0 ? 0.5f : 1.0f), 5.5f*game->GetGameDeltaTime());

    if (InvincibilityResetTimer > 0)
        InvincibilityResetTimer -= game->GetGameDeltaTime();
    if (InvincibilityResetTimer <= 0)
    {
        isInvincible = false;
        Dodging=false;
    }

    if (PlayerFrozenTimer <= 0) {
        // powerup logic
        if (game->GameControls->IsControlDown("powerup"))
        {
            MainPowerupSystem.Activate();
        }

        // firing logic
        if (IsMouseButtonDown(0) && MainWeaponsSystem.CurrentWeapon != nullptr &&
            MainWeaponsSystem.CurrentWeapon->Ammo > 0 && MainWeaponsSystem.WeaponAmmo[MainWeaponsSystem.CurrentWeaponIndex] <=0 &&
            MainWeaponsSystem.AttackCooldowns[MainWeaponsSystem.CurrentWeaponIndex] >= MainWeaponsSystem.CurrentWeapon->Cooldown)
        {
            MainWeaponsSystem.Reload();
        }
        //Vector2 WorldMousePos = Vector2{0, 0};
        if ((IsMouseButtonDown(0) || game->GameControls->IsControlDown("attack_other")) && !IsPreparingForDash)
        {
            MainWeaponsSystem.Attack(GetScreenToWorld2D(GetMousePosition(), game->GameCamera.RaylibCamera));
        }

        // reload logic
        if (game->GameControls->IsControlPressed("reload") && MainWeaponsSystem.TimeStartedReloading == -1)
            MainWeaponsSystem.Reload();

        //dropping stuff
        if (game->GameControls->IsControlPressed("drop") && MainWeaponsSystem.CurrentWeaponIndex != -1)
            MainWeaponsSystem.DropWeapon(MainWeaponsSystem.Weapons[MainWeaponsSystem.CurrentWeaponIndex]);

        // inventory input logic
        if (game->GameControls->IsControlPressed("item1")) {
            if (MainWeaponsSystem.CurrentWeaponIndex != 0) {
                MainWeaponsSystem.Equip(0);
            } else if (MainWeaponsSystem.CurrentWeaponIndex == 0) {
                MainWeaponsSystem.Unequip();
            }
        }
        if (game->GameControls->IsControlPressed("item2")) {
            if (MainWeaponsSystem.CurrentWeaponIndex != 1) {
                MainWeaponsSystem.Equip(1);
            } else if (MainWeaponsSystem.CurrentWeaponIndex == 1) {
                MainWeaponsSystem.Unequip();
            }
        }
        if (game->GameControls->IsControlPressed("item3")) {
            if (MainWeaponsSystem.CurrentWeaponIndex != 2) {
                MainWeaponsSystem.Equip(2);
            } else if (MainWeaponsSystem.CurrentWeaponIndex == 2) {
                MainWeaponsSystem.Unequip();
            }
        }
    } else {
        PlayerFrozenTimer -= game->GetGameDeltaTime();
    }

    Vector2 c = GetCenter();
    DrawCircleGradient(c.x, c.y, BoundingBox.width / 1.25f, ColorAlpha(PURPLE, 0.5), BLANK);

    // update entity
    Entity::Update();
    MainWeaponsSystem.Update();
    MainPowerupSystem.Update();
    LogicProcessor.Update();

    // did we get a kill? play kill sound game!
    if (Kills != LastKills) {
        game->GameSounds.PlayGameSound("death");
        ExtraSpeed += 14;

        float ComboTime = min(max(4.0f - 3.0f * (static_cast<float>(EnemiesDetected) / 10.0f), 1.0f), 4.0f);

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
            KillPoints *= 1.0f + (1.0f - static_cast<float>(game->GetGameTime() - LastKilledAnEnemy) / ComboTime) / 2.0f;
            KillName += ", Combo x"+to_string(EnemyCombo);
        } else
            EnemyCombo = 0;

        KillPoints *= 1.0f + static_cast<float>(EnemyCombo) / 10.0f;
        LogicProcessor.IncreaseScore(KillName, KillPoints);

        LastKilledAnEnemy = game->GetGameTime();
    }
    LastKills = Kills;
    EnemiesDetected = 0;
}

void Player::OnDeath()
{
    if (!game->CurrentLevelName.empty() && !game->LevelData[game->CurrentLevelName]["music"].get<string>().empty())
    {
        for (int i = 1; i < 5; i++)
        {
            std::string FightTrack = game->LevelData[game->CurrentLevelName]["music"].get<string>()+"_layer"+to_string(i);
            game->GameSounds.StopGameMusic(FightTrack, true);
        }
    }
    Entity::OnDeath();
}

void Player::OnDelete()
{
    ScoreChanges.clear();
    MainWeaponsSystem.Unequip();
    LogicProcessor.Destroy();
    Entity::OnDelete();
}
