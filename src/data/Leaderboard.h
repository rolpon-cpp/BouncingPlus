//
// Created by lalit on 6/11/2026.
//

#ifndef BOUNCINGPLUS_LEADERBOARD_H
#define BOUNCINGPLUS_LEADERBOARD_H

#include "firebase/app.h"

class Leaderboard
{
    public:
    firebase::App* app;
    Leaderboard();
    ~Leaderboard();
};


#endif //BOUNCINGPLUS_LEADERBOARD_H