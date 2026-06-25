//
// Created by Rolpon on 6/11/2026.
//

#include "Leaderboard.h"

#include <algorithm>

#include "keys/firebase_keys.h"

Leaderboard::Leaderboard()
{
    firebase::AppOptions options;
    options.set_api_key(FIREBASE_API_KEY);
    options.set_app_id(FIREBASE_APP_ID);
    options.set_project_id(FIREBASE_PROJECT_ID);

    FirebaseApp = firebase::App::Create(options);
    FirebaseDatabase = firebase::database::Database::GetInstance(FirebaseApp);

    AwaitingResponse = false;
}

Leaderboard::~Leaderboard()
{
}

void Leaderboard::UpdateData()
{
    if (!AwaitingResponse)
    {
        FirebaseFuture = FirebaseDatabase->GetReference("leaderboard").GetValue();
        AwaitingResponse = true;
    }

    if (FirebaseFuture.status() == firebase::kFutureStatusComplete)
    {
        if (FirebaseFuture.error() == firebase::database::kErrorNone) {
            ExtractLeaderboardData();
        } else {
            printf("Firebase DB error %d: %s\n", FirebaseFuture.error(), FirebaseFuture.error_message());
        }
        AwaitingResponse = false;
    }
}

void Leaderboard::ExtractLeaderboardData()
{
    auto RawData = FirebaseFuture.result();

    std::map<std::string, int> Scores;

    if (RawData->HasChild("scores"))
    {
        for (const firebase::database::DataSnapshot& child : RawData->Child("scores").children())
        {
            std::string name;
            int score = 0;

            if (child.HasChild("name"))
                name = child.Child("name").value().string_value();
            if (child.HasChild("score"))
                score = child.Child("score").value().int64_value();

            Scores[name] = score;
        }
    }

    std::vector<std::pair<std::string, int>> sorted(Scores.begin(), Scores.end());

    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    LeaderboardScores = sorted;
}

void Leaderboard::Quit()
{
    LeaderboardScores.clear();
    delete FirebaseApp;
}
