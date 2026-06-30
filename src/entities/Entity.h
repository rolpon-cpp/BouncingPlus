//
// Created by Rolpon on 8/26/2025.
//

#ifndef BOUNCINGPLUS_ENTITY_H
#define BOUNCINGPLUS_ENTITY_H
#include <complex.h>
#include <memory>
#include "EntityType.h"
#include <raylib.h>

class Game;

class Entity : public std::enable_shared_from_this<Entity>
{
    void Initialize(Texture2D& Texture, Rectangle BoundingBox, float Speed);

public:
    EntityType Type;
    EntityPriorityType Priority;

    Game* game;
    Rectangle BoundingBox;
    Texture2D* Texture;
    Vector2 Movement;

    Color EntityColor;

    Vector2 VelocityMovement;
    float VelocityPower;
    Vector2 LastVelBounceCoord;

    float WeaponWeightSpeedMultiplier;
    float FrameStackSpeed;
    float Rotation;
    float Speed;
    float Health;
    float MaxHealth;
    bool ShouldDelete;
    bool CollisionsEnabled;
    Entity();
    virtual ~Entity();

    Entity(Texture2D& Texture, Rectangle BoundingBox, float Speed, Game& game);
    virtual void PhysicsUpdate(float DeltaTime, double time);
    bool IsVisible(Rectangle bbox);
    bool IsVisible();
    Vector2 GetCenter();
    float GetSpeed();
    void DamageOther(std::shared_ptr<Entity> other, float Damage, std::shared_ptr<Entity> owner = nullptr,
                     float HealthGain = -1);
    virtual void OnWallVelocityBump(float Power);
    virtual void Update();
    virtual void OnDelete();
    virtual void OnDeath();
};


#endif //BOUNCINGPLUS_ENTITY_H