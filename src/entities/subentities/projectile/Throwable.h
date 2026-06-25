//
// Created by Rolpon on 4/3/2026.
//

#ifndef BOUNCINGPLUS_THROWABLE_H
#define BOUNCINGPLUS_THROWABLE_H

#include "../../Entity.h"
#include <raylib.h>

class Throwable : public Entity
{
public:
    double SpawnTime;
    Throwable(Vector2 From, Vector2 Direction, std::string Texture, float EntityHitWidth, float CollisionHitWidth,
              float Height, Game& game);
    Throwable();
    ~Throwable() override;
    void Update() override;
    void PhysicsUpdate(float DeltaTime, double time) override;
};


#endif //BOUNCINGPLUS_THROWABLE_H