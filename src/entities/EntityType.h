//
// Created by Rolpon on 10/24/2025.
//

#ifndef ENTITYTYPE_H
#define ENTITYTYPE_H

enum EntityType
{
    DefaultEntityType,
    EnemyEntityType,
    PlayerEntityType,
    SpawnerEntityType,
    BulletEntityType,
    BossEntityType,
    TurretEntityType,
    ThrowableEntityType,
    EndOfEntityType
};

enum EntityPriorityType
{
    AlwaysPriorityType,
    CloseToPlayerPriorityType,
    NearbyPlayerPriorityType,
    LargeAreaNearbyPlayerPriorityType,
    NeverPriorityType,
};

#endif //ENTITYTYPE_H