//
// Created by Rolpon on 4/3/2026.
//

#include "Throwable.h"

#include <raymath.h>
#include "../../../game/managers/ResourceManager.h"
#include "../../../game/Game.h"

Throwable::Throwable(Vector2 From, Vector2 Direction, std::string Texture, float EntityHitWidth,
                     float CollisionHitWidth, float Height, Game& game) :
    Entity(game.GameResources->Textures[Texture], {0, 0, 10, 10}, 2000, game)
{
    Type = ThrowableEntityType;
    Priority = LargeAreaNearbyPlayerPriorityType;
    SpawnTime = game.GetGameTime();
    Movement = Vector2Normalize(Direction);
    //BoundingBox.x = From + ()
}

Throwable::Throwable()
{
}

Throwable::~Throwable()
{
}

void Throwable::Update()
{
    Entity::Update();
}

void Throwable::PhysicsUpdate(float DeltaTime, double time)
{
    Entity::PhysicsUpdate(DeltaTime, time);
}