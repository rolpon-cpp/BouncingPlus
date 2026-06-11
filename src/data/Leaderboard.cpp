//
// Created by lalit on 6/11/2026.
//

#include "Leaderboard.h"
#include "../firebase_keys.h"

Leaderboard::Leaderboard()
{
    firebase::AppOptions options;
    options.set_api_key(FIREBASE_API_KEY);
    options.set_app_id(FIREBASE_APP_ID);
    options.set_project_id(FIREBASE_PROJECT_ID);

    app = firebase::App::Create(options);
}

Leaderboard::~Leaderboard()
{
}
