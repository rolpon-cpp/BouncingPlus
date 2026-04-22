//
// Created by lalit on 8/29/2025.
//

#ifndef BOUNCINGPLUS_BULLET_H
#define BOUNCINGPLUS_BULLET_H
#include <string>
#include <vector>

#include "../Entity.h"

using namespace std;

class Bullet : public Entity {
    float ExistenceTimer;
    float Damage;
    bool DD=false;

    std::pair<bool,vector<Vector2>> BulletCollision();
public:
    float RotGoal;
    std::weak_ptr<Entity> OwnerPtr;
    float Lifetime;
    bool SlowdownOverTime;
    float HealthGain;
    Vector2 FirePoint;
    vector<Vector2> LastBouncedCoordinates;
    Bullet(float X, float Y, float Angle, Vector2 Size, float Speed, float Damage, float Lifetime, Texture2D& BulletTexture, std::shared_ptr<Entity> Owner, Game &game);
    Bullet();
    void Attack(std::shared_ptr<Entity> entity);
    void PhysicsUpdate(float dt, double time)override;
    void Bounce(Vector2 Normal);
    ~Bullet()override;
    void Update()override;
};


#endif