//
// Created by lalit on 6/11/2026.
//

#ifndef BOUNCINGPLUS_LEADERBOARD_H
#define BOUNCINGPLUS_LEADERBOARD_H

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include "firebase/app.h"
#include "firebase/database.h"

class Leaderboard
{
    void ExtractLeaderboardData();
    public:

    std::vector<std::pair<std::string, int>> LeaderboardScores;

    bool AwaitingResponse;
    firebase::Future<firebase::database::DataSnapshot> FirebaseFuture;

    firebase::App* FirebaseApp;
    firebase::database::Database* FirebaseDatabase;
    Leaderboard();
    ~Leaderboard();

    void UpdateData();
    void Quit();
};


#endif //BOUNCINGPLUS_LEADERBOARD_H