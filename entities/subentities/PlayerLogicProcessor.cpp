//
// Created by lalit on 3/14/2026.
//

#include "PlayerLogicProcessor.h"

#include <iostream>
#include <raymath.h>
#include <nlohmann/json.hpp>

#include "../../game/Game.h"
#include "Player.h"
#include "Bullet.h"
#include "Turret.h"

PlayerLogicProcessor::PlayerLogicProcessor(std::weak_ptr<Player> Owner)
{
    this->Owner = Owner;
    DashCooldown = 0;
    PlayerDashLineThickness = 10;
    DashTimeStart = 0;
    DamageNotifs = std::vector<Vector3>();
    RankClassifications = std::vector<std::string>();
    DashedEnemies = std::vector<std::weak_ptr<Enemy>>();
    FightMusicLayer = 0;
    RankLevel = 0;
    FightMusicLayerGoal = 0;
    LayerSwitchCooldown = 0;
    PreviousFightTrack = "";

    RankClassifications.emplace_back("Bad");
    RankClassifications.emplace_back("Okay");
    RankClassifications.emplace_back("Decent");
    RankClassifications.emplace_back("Good");
    RankClassifications.emplace_back("Great");
    RankClassifications.emplace_back("BOUNCING!!!");
}

PlayerLogicProcessor::PlayerLogicProcessor()
{
}

PlayerLogicProcessor::~PlayerLogicProcessor()
{
}

void PlayerLogicProcessor::ProcessStress()
{
    auto MyPlayer = Owner.lock();
    MyPlayer->FrameStressLevel = 0.0f;
    if (MyPlayer->HealthConcern)
        MyPlayer->FrameStressLevel += 0.5f;

    std::vector<shared_ptr<Entity>> bulletArray = MyPlayer->game->GameEntities.Entities[BulletType];
    for (int i = 0; i < bulletArray.size(); i++)
        if (shared_ptr<Bullet> entity = dynamic_pointer_cast<Bullet>(bulletArray.at(i)); entity != nullptr and !entity->ShouldDelete)
            if (Vector2Distance({entity->BoundingBox.x, entity->BoundingBox.y},{MyPlayer->game->MainPlayer->BoundingBox.x,MyPlayer->game->MainPlayer->BoundingBox.y}) < 600)
                MyPlayer->FrameStressLevel += 0.01f;

    std::vector<shared_ptr<Entity>> turretArray = MyPlayer->game->GameEntities.Entities[TurretType];
    for (int i = 0; i < turretArray.size(); i++)
        if (shared_ptr<Turret> entity = dynamic_pointer_cast<Turret>(turretArray.at(i)); entity != nullptr and !entity->ShouldDelete)
            if (entity->CurrentState != LOOKING)
                MyPlayer->FrameStressLevel += 0.1f;

    MyPlayer->FrameStressLevel += MyPlayer->EnemiesDetected * 0.075f;

    MyPlayer->FrameStressLevel = min(max(MyPlayer->FrameStressLevel, 0.0f), 1.0f);

    if (MyPlayer->FrameStressLevel > MyPlayer->StressLevel)
        MyPlayer->StressLevel = Lerp(MyPlayer->StressLevel, MyPlayer->FrameStressLevel, 2.5f * MyPlayer->game->GetGameDeltaTime());
    else
        MyPlayer->StressLevel = Lerp(MyPlayer->StressLevel, MyPlayer->FrameStressLevel, 0.25f * MyPlayer->game->GetGameDeltaTime());

}

void PlayerLogicProcessor::HandleFightMusic()
{
    auto MyPlayer = Owner.lock();

    if (MyPlayer->Health <= 0)
        return;
    if (MyPlayer->game->isReturning || MyPlayer->game->ShouldReturn)
        return;
    if (MyPlayer->game->CurrentLevelName.empty())
        return;
    if (!MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName].count("music"))
        return;
    if (MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["music"].empty())
        return;

    if (LayerSwitchCooldown <= 0)
    {
        if (MyPlayer->StressLevel <= 0.3f)
        {
            FightMusicLayerGoal = 1.0f;
        } else if (MyPlayer->StressLevel <= 0.5f)
        {
            FightMusicLayerGoal = 2.0f;
        } else if (MyPlayer->StressLevel <= 0.7)
        {
            FightMusicLayerGoal = 3.0f;
            LayerSwitchCooldown = 2.0f;
        } else if (MyPlayer->StressLevel <= 1.0f)
        {
            FightMusicLayerGoal = 4.0f;
            LayerSwitchCooldown = 10.0f;
        }
    } else
    {
        LayerSwitchCooldown -= MyPlayer->game->GetGameDeltaTime();
    }

    FightMusicLayer = Lerp(FightMusicLayer, FightMusicLayerGoal, 2.5f * MyPlayer->game->GetGameDeltaTime());

    int ChosenLayer = (int)round(FightMusicLayer);
    std::string FightTrack = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["music"].get<std::string>()+"_layer"+to_string(ChosenLayer);
    if (ChosenLayer == 4 && MyPlayer->StressLevel >= 0.4f)
    {
        LayerSwitchCooldown += 3.0f;
        LayerSwitchCooldown = min((float)LayerSwitchCooldown, 10.0f);
    }

    if (PreviousFightTrack != FightTrack)
    {
        if (!PreviousFightTrack.empty() && MyPlayer->game->GameSounds.IsGameMusicPlaying(PreviousFightTrack))
            MyPlayer->game->GameSounds.StopGameMusic(PreviousFightTrack, true);
        PreviousFightTrack = FightTrack;
    }

    if (!MyPlayer->game->GameSounds.IsGameMusicPlaying(FightTrack))
        MyPlayer->game->GameSounds.PlayGameMusic(FightTrack, true);

}

void PlayerLogicProcessor::Update()
{
    DashLogic();
    DisplayDamageNotifs();
    ProcessStress();
    RankLevelLogic();
    HandleFightMusic();
}

void PlayerLogicProcessor::PhysicsUpdate()
{
    DashAttacking();
}

void PlayerLogicProcessor::Destroy()
{
    DashCooldown = 0;
    RankLevel = 0;
    PlayerDashLineThickness = 10;
    DashTimeStart = 0;
    DamageNotifs.clear();
    DashedEnemies.clear();
}

void PlayerLogicProcessor::RankLevelLogic()
{
    auto MyPlayer = Owner.lock();

    RankLevel -= RankLevel * (1.0f - MyPlayer->StressLevel) * 0.12f * MyPlayer->game->GetGameDeltaTime();

    RankLevel = max(min(RankLevel, 1.0f), 0.0f);
    std::erase_if(MyPlayer->ScoreChanges, [MyPlayer](ScoreChange& e) {
        return MyPlayer->game->GetGameTime() - e.Time >= 10;
        });
}

void PlayerLogicProcessor::IncreaseScore(std::string Reason, float Points)
{
    auto MyPlayer = Owner.lock();

    Points *= MyPlayer->StressLevel;

    RankLevel += min(Points / 600.0f, 0.1f);

    MyPlayer->ScoreChanges.push_back({Reason, Points, MyPlayer->game->GetGameTime()});
    MyPlayer->game->GameScore += Points;
}

void PlayerLogicProcessor::DamageNotification(Vector2 From)
{
    auto MyPlayer = Owner.lock();
    this->DamageNotifs.push_back({From.x, From.y, (float) MyPlayer->game->GetGameTime()});
}

void PlayerLogicProcessor::AttackDashedEnemy(std::shared_ptr<Enemy> entity, bool already_attacked)
{
    auto MyPlayer = Owner.lock();
    // check if we're colliding with them. if so, attack!
    float extra_size = 64;
    Rectangle myRect = Rectangle{MyPlayer->BoundingBox.x - extra_size / 2, MyPlayer->BoundingBox.y - extra_size / 2, MyPlayer->BoundingBox.width + extra_size, MyPlayer->BoundingBox.height + extra_size};
    if (MyPlayer->game->DebugDraw)
        DrawRectangleRec(myRect, ColorAlpha(RED, 0.5f));
    if (CheckCollisionRecs(myRect, entity->BoundingBox) && !already_attacked) {
        // calculate damage & attack
        float Damage = min(MyPlayer->VelocityPower / 18.5f / max(1.0f, DashedEnemies.size() * 0.85f), 100.0f);

        float EnemyConcentration = 0.8f + (MyPlayer->FrameStressLevel*1.2f);

        EnemyConcentration = max(min(EnemyConcentration, 2.0f), 1.0f);
        EnemyConcentration *= MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_concentration_boost"].get<float>();

        Damage *= EnemyConcentration;
        Damage *= min(max((MyPlayer->Health / MyPlayer->MaxHealth) - 2.0f, 1.0f), 5.5f);
        if (entity->Armor <= 0)
            entity->Health -= Damage;
        else
            entity->Armor -= Damage;

        float reward = Damage / 12.5f;
        MyPlayer->Health += reward;

        float amount = 1500.0f;

        // did we kill them? if so, give health & kills
        if (entity->Health <= 0) {
            MyPlayer->Health += Damage * 0.1f;
            amount = 950;
            MyPlayer->Kills+=1;
            this->IncreaseScore("Dash Kill", 45 * ((DashedEnemies.size() + 1.0f) * 1.25f));
        }

        MyPlayer->game->GameParticles.ParticleEffect({{
            MyPlayer->BoundingBox.x + MyPlayer->BoundingBox.width/2, MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.height/2},
                700,
                PURPLE,
                600,
                MyPlayer->VelocityPower / 500.0f,
                1.3f,
                PINK
        }, 180-Vector2LineAngle({0,0}, MyPlayer->VelocityMovement)*RAD2DEG, 30, 35);

        this->IncreaseScore("Dash", Damage / 1.5f);
        MyPlayer->game->GameCamera.CameraPosition += Vector2Normalize({(float)GetRandomValue(-25, 25), (float)GetRandomValue(-25, 25)}) * (MyPlayer->VelocityPower / 150);
        MyPlayer->game->GameCamera.ShakeCamera(MyPlayer->VelocityPower / (amount - 50) / 1.5f);
        MyPlayer->game->GameCamera.QuickZoom(0.95f, 0.05f, false);
        MyPlayer->game->GameSounds.PlayGameSound("dash_hit",min(max(MyPlayer->VelocityPower/amount, 0.0f), 0.8f));

        // give them pushback force
        entity->VelocityMovement = MyPlayer->VelocityMovement;
        entity->VelocityPower = -MyPlayer->VelocityPower/2;

        // increase velocity and mark enemy as attacked
        MyPlayer->VelocityPower += MyPlayer->VelocityPower / (amount/200);
        DashedEnemies.push_back(std::weak_ptr(entity));
    }
}

void PlayerLogicProcessor::DashAttacking()
{
    auto MyPlayer = Owner.lock();
    if (MyPlayer->VelocityPower > 0 && !MyPlayer->Dodging) {
        // get enemies list
        std::vector<shared_ptr<Entity>>* array = &MyPlayer->game->GameEntities.Entities[EnemyType];
        for (int i = 0; i < array->size(); i++) {
            if (shared_ptr<Enemy> entity = dynamic_pointer_cast<Enemy>(array->at(i)); entity != nullptr and !entity->ShouldDelete) {
                // have we already attacked them? if so, ignore this!!!
                bool already_attacked = false;
                for (auto e : DashedEnemies) {
                    if (!e.owner_before(entity) && !entity.owner_before(e)) {
                        already_attacked = true;
                        break;
                    }
                }

                // attack them
                AttackDashedEnemy(entity, already_attacked);
            }
        }
    } else {
        DashedEnemies.clear();
    }
}

void PlayerLogicProcessor::DashLogic()
{
    auto MyPlayer = Owner.lock();
    // update dash cooldownm
    if (DashCooldown > 0)
        DashCooldown -= MyPlayer->game->GetGameDeltaTime();

    if (DashCooldown <= 0 && MyPlayer->game->GameControls->IsControlDown("dash") && !MyPlayer->isInvincible) {
        if (!MyPlayer->IsPreparingForDash) {
            DashTimeStart = MyPlayer->game->GetGameTime();
            PlayerDashLineThickness = 10;
        }
        MyPlayer->IsPreparingForDash = true;
    }
    if (
        (MyPlayer->IsPreparingForDash && !MyPlayer->game->GameControls->IsControlDown("dash")) || MyPlayer->MainWeaponsSystem.ChargingProgress > 0.0f
        ) {
        MyPlayer->IsPreparingForDash = false;
    }
    if (MyPlayer->game->MainPlayer->IsPreparingForDash || MyPlayer->game->MainPlayer->MainWeaponsSystem.TimeStartedReloading != -1) {
        MyPlayer->game->GameCamera.QuickZoom(1.25f, 0.1f);
        if (MyPlayer->game->MainPlayer->IsPreparingForDash)
        {
            if (static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart) / 1.1f > 0.8f)
                PlayerDashLineThickness = Lerp(PlayerDashLineThickness, 20, 2 * MyPlayer->game->GetGameDeltaTime());
            float alpha = static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart) / 1.1f;
            Vector2 Target = GetScreenToWorld2D(GetMousePosition(), MyPlayer->game->GameCamera.RaylibCamera);
            float cx = MyPlayer->BoundingBox.x + MyPlayer->BoundingBox.width / 2;
            float cy = MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.height / 2;
            float FinalAngle = (atan2(cy - Target.y, cx - Target.x) * RAD2DEG);
            Texture2D& anim_tex = MyPlayer->game->GameResources.Textures["arrow"];
            float width = anim_tex.width;
            float height = anim_tex.height;
            BeginShaderMode(MyPlayer->game->GameResources.Shaders["dash_arrow"]);
            float t = static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart);

            if (uTime == -1)
            {
                uTime = GetShaderLocation(MyPlayer->game->GameResources.Shaders["dash_arrow"], "time");
                uWidth = GetShaderLocation(MyPlayer->game->GameResources.Shaders["dash_arrow"], "renderWidth");
                uHeight = GetShaderLocation(MyPlayer->game->GameResources.Shaders["dash_arrow"], "renderHeight");
            }

            int w = GetRenderWidth();
            int h = GetRenderHeight();

            SetShaderValue(MyPlayer->game->GameResources.Shaders["dash_arrow"],
                uTime,
                &t,
                SHADER_UNIFORM_FLOAT);

            SetShaderValue(MyPlayer->game->GameResources.Shaders["dash_arrow"],
                uWidth,
                &w,
                SHADER_UNIFORM_INT);

            SetShaderValue(MyPlayer->game->GameResources.Shaders["dash_arrow"],
                uHeight,
                &h,
                SHADER_UNIFORM_INT);

            float size = 0.375f;
            DrawTexturePro(anim_tex, Rectangle{0, 0, width, height *6},
                           Rectangle{MyPlayer->BoundingBox.x + MyPlayer->BoundingBox.width/2 - cosf((FinalAngle) * DEG2RAD)*10,
                               MyPlayer->BoundingBox.y +MyPlayer->BoundingBox.height/2 - sinf((FinalAngle) * DEG2RAD)*10, width*size,
                                     height * 6 * size
        }, Vector2{width * size * 0.5f, height*6*size}, FinalAngle-90, ColorAlpha(WHITE, alpha));
            EndShaderMode();
        }
    }
    if ((IsMouseButtonDown(1) || IsMouseButtonDown(0)) && MyPlayer->IsPreparingForDash && MyPlayer->Health > 0 && MyPlayer->game->GetGameTime() - DashTimeStart >= 0.35f) {
        DashCooldown = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_base_cooldown"].get<float>() + (2.2f - min(static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart), 1.1f) * 2);
        DashedEnemies.clear();
        MyPlayer->VelocityMovement = Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), MyPlayer->game->GameCamera.RaylibCamera), {MyPlayer->BoundingBox.x+MyPlayer->BoundingBox.width/2, MyPlayer->BoundingBox.y+MyPlayer->BoundingBox.height/2});
        MyPlayer->VelocityPower = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_base_power"].get<float>() * max(min(static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart), 1.1f), 0.45f);
        MyPlayer->VelocityPower /= min(max((MyPlayer->Health / MyPlayer->MaxHealth)-2.0f, 1.0f), 1.25f);
        MyPlayer->VelocityPower *= MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_power_multiplier"].get<float>();
        MyPlayer->game->GameSounds.PlayGameSound("dash");
        MyPlayer->PlayerFrozenTimer = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_frozen_multiplier"].get<float>() *
            min(max((MyPlayer->VelocityPower / MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_base_power"].get<float>()), 0.35f), 1.1f);
        if (!MyPlayer->isInvincible)
        {
            MyPlayer->ToggleInvincibility();
            MyPlayer->InvincibilityResetTimer = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dash_iframe_time"].get<float>();
        }
        if (IsMouseButtonDown(1)) {
            MyPlayer->VelocityPower *= 1.25f;
            MyPlayer->Dodging = true;
            MyPlayer->InvincibilityResetTimer = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dodge_iframe_time"].get<float>();
            DashCooldown = MyPlayer->game->LevelData[MyPlayer->game->CurrentLevelName]["player"]["dodge_cooldown"].get<float>();
        }
        MyPlayer->IsPreparingForDash = false;
    }

    // display dashing bar
    int w = 56;
    int h = 15;
    float a = 0;
    if (MyPlayer->IsPreparingForDash)
        a = Lerp(0, 0.6f, min( (float)(MyPlayer->game->GetGameTime() - DashTimeStart) / 0.2f, 1.0f ));

    DrawRectangle((int)(MyPlayer->BoundingBox.x + (MyPlayer->BoundingBox.width / 2) - (w/2)),
        (int)(MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.width + 10),
        w, h, ColorAlpha(BLACK, a));

    DrawRectangle(
        (int)(MyPlayer->BoundingBox.x + (MyPlayer->BoundingBox.width / 2) - (w/2))+5,
        (int)(MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.width + 10)+5,
        (!MyPlayer->IsPreparingForDash ? 0 : min(static_cast<float>(MyPlayer->game->GetGameTime() - DashTimeStart) / 1.1f, 1.0f))*(w-10),
        h-10,
        ColorAlpha(WHITE, a*1.25f));

    // Health bar
    DrawRectangle((int)(MyPlayer->BoundingBox.x-20-h),
       (int)(MyPlayer->BoundingBox.y + (MyPlayer->BoundingBox.height / 2) - (w/2)),
       h, w,
       ColorAlpha(BLACK, 0.4f));

    DrawRectangle((int)(MyPlayer->BoundingBox.x-20-h)+5,
       (int)(MyPlayer->BoundingBox.y + (MyPlayer->BoundingBox.height / 2) - (w/2))+5,
       h - 10, min(MyPlayer->Health / 100, 1.0f) * (w - 10.0f),
       ColorAlpha(GREEN, (MyPlayer->HealthConcern ? (MyPlayer->WarningSign ? 0 : 0.8f) : 0.8f) ));

    // Ammo bar
    if (MyPlayer->MainWeaponsSystem.CurrentWeapon != nullptr && MyPlayer->MainWeaponsSystem.CurrentWeapon->Ammo > 0)
    {
        DrawRectangle((int)(MyPlayer->BoundingBox.x+MyPlayer->BoundingBox.height+20),
       (int)(MyPlayer->BoundingBox.y + (MyPlayer->BoundingBox.height / 2) - (w/2)),
       h, w,
       ColorAlpha(BLACK, 0.4f));

        DrawRectangle((int)(MyPlayer->BoundingBox.x+MyPlayer->BoundingBox.height+20)+5,
           (int)(MyPlayer->BoundingBox.y + (MyPlayer->BoundingBox.height / 2) - (w/2))+5,
           h - 10, ((float)MyPlayer->MainWeaponsSystem.WeaponAmmo[MyPlayer->MainWeaponsSystem.CurrentWeaponIndex] / MyPlayer->MainWeaponsSystem.CurrentWeapon->Ammo) * (w - 10.0f),
           ColorAlpha(YELLOW, 0.8f ));
    }
}

void PlayerLogicProcessor::DisplayDamageNotifs()
{
    auto MyPlayer = Owner.lock();
    std::erase_if(DamageNotifs, [this, MyPlayer](Vector3 e)
    {
        return (float)MyPlayer->game->GetGameTime() - e.z >= 1.5f;
    });
    for (Vector3 notif : DamageNotifs)
    {
        float time = (float)MyPlayer->game->GetGameTime() - notif.z;
        Vector2 m = Vector2Multiply(Vector2Normalize(Vector2Subtract({MyPlayer->BoundingBox.x + MyPlayer->BoundingBox.width/2, MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.height/2}, {notif.x, notif.y})),
            {200, 200});
        float angle = 180 - Vector2LineAngle({0,0},m) * RAD2DEG;
        float trans = 1.0f;
        if (time <= 0.5f)
            trans = time / 0.5f;
        if (time >= 1.0f)
            trans = (time - 1.0f) / 0.5f;
        DrawCircleSector(Vector2Subtract({MyPlayer->BoundingBox.x + MyPlayer->BoundingBox.width/2, MyPlayer->BoundingBox.y + MyPlayer->BoundingBox.height/2}, m), 45, (180+angle)-50, (180+angle)+50, 10, ColorAlpha(WHITE, trans/2));
    }
}
