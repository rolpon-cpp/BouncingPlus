//
// Created by lalit on 5/23/2026.
//

#ifndef BOUNCINGPLUS_DWELLERBEHAVIOR_H
#define BOUNCINGPLUS_DWELLERBEHAVIOR_H
#include "EnemyBehavior.h"
#include "raylib.h"

class DwellerBehavior : public EnemyBehavior
{
public:
    bool HasMovedIntoPlace;
    Vector2 PlaceSpot;
    float TravelDistance;
    double TravelStartTime;

    DwellerBehavior();
    DwellerBehavior(Enemy& Owner, Game& game);
    ~DwellerBehavior();
    void Update();
    void ChooseNewMovementSpot();
};


#endif //BOUNCINGPLUS_DWELLERBEHAVIOR_H