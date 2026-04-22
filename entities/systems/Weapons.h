//
// Created by lalit on 9/13/2025.
//

#ifndef BOUNCINGPLUS_WEAPON_H
#define BOUNCINGPLUS_WEAPON_H

#include <vector>
#include <string>
#include "raylib.h"

class Player;
class Entity;
class Turret;
using namespace std;

struct Weapon {
    bool isMelee = false;
    bool Throwable = false;
    bool ShakeScreen = false;
    bool SlowdownOverTime = false;
    float ChargeSpeed = 0;
    float PushbackForce = 0;
    float WeaponSize = 1;
    int SpreadRange[2] = {0, 0};
    int Ammo = -1;
    double ReloadTime = 0;
    float Speed = 0;
    float WeaponWeightSpeedMultiplier = 1;
    Vector2 Size = {1,1};
    float Damage = 0;
    float HealthGain = -1;
    float BulletLifetime = -1;
    float Cooldown = 0;
    float AngleRange = 0;
    float Range = 0;
    int Bullets = -1;
    float Intensity = 0;
    std::string BulletTexture = "";
    std::string texture = "";
    std::vector<string> sound;
};

struct WeaponPickup
{
    Vector2 Position;
    Color PickupColor;
    float Radius;
    int Ammo = -1;
    std::string Weapon = "Default Gun";
    double AnimationSpeed = 2.5f;
    float AnimationPower = 50.0f;
    bool PickedUp = false;
    bool LeftOwner = false;
    double CreationTime = 0;
};

class WeaponsSystem {
    void DisplayGunTexture();
    void MeleeAttack(std::shared_ptr<Entity> entity, float Angle);
    void ResetMeleeAnim();
    void DisplayWeaponCone();
    void DisplayWeaponReflectance();
    void DisplayMeleeAnim();
    void GunAttack(float TargetAngle, float cX, float cY);

    Game* game;
    weak_ptr<Entity> OwnerPtr;
    bool MeleeAnim;
    Texture* MeleeAnimTexture;
    float MeleeAnimAngle;
    float MeleeAnimRange;
    float MeleeAnimPercent;
    float MeleeAnimAlpha;
    Vector2 MeleeDisplayRenderTarget;

    public:
        std::string Weapons[3];
        Weapon* CurrentWeapon = nullptr;
        double TimeStartedReloading;
        int CurrentWeaponIndex = 0;
        float AttackCooldowns[3];
        int WeaponAmmo[3];
        float ChargingProgress = 0.0f;
        bool TriedChargingThisFrame = false;
        Vector2 ChargeTarget = {0,0};

        WeaponsSystem(shared_ptr<Entity> Owner, Game &game);
        WeaponsSystem();
        virtual ~WeaponsSystem();
        virtual void Equip(int Index);
        virtual void Reload();
        virtual bool GiveWeapon(std::string WeaponName, int Ammo = -1);
        virtual bool DropWeapon(std::string WeaponName);
        virtual void Unequip();
        virtual void Update();
        virtual void Attack(Vector2 Target);
        virtual void Charge(Vector2 Target);
        void ShootWeaponOut();
};

#endif //BOUNCINGPLUS_WEAPON_H