#include <iostream>
#include <thread>

#include "raylib.h"
#include "game/Game.h"
#include "game/ui/Menu.h"
#include "globals.h"
#include "game/core/SharedManager.h"
#include "game/managers/ResourceManager.h"
#include "level/LevelLoader.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#elif FIREBASE
#include "data/Leaderboard.h"
#endif

#ifndef PLATFORM_WEB
#include "GLFW/glfw3.h"
#endif

struct Data
{
    SharedManager& SharedManager;
    Game* MainGame;
    Menu* MainMenu;
    bool& InGame;
};

void load_game(Game*& g, Menu*& m, SharedManager& s, GLFWwindow*& window, LoadingStage* ld, bool* IsDone)
{
    glfwMakeContextCurrent(window);
    s.Load();
    ld->stage = -1;
    g = new Game(s,ld);
    m = new Menu(s);
    glfwMakeContextCurrent(nullptr);
    *IsDone = true;
}

void loop(void* arg)
{
    Data* d = (Data*)arg;
    SharedManager& SharedMgr = d->SharedManager;
    Game& MainGame = *d->MainGame;
    Menu& MainMenu = *d->MainMenu;
    bool& InGame = d->InGame;

#ifdef PLATFORM_WEB
    SetMouseScale((float)GetRenderWidth() / WINDOW_WIDTH, (float)GetRenderHeight() / WINDOW_HEIGHT);
#endif

    BeginDrawing();

    SharedMgr.Update();

    ClearBackground(BLANK);

    if (InGame)
    {
        if (MainGame.ShouldReturn)
        {
            InGame = false;
            MainMenu.Reset();
            MainGame.ShouldReturn = false;
            MainGame.Clear();

#ifndef PLATFORM_WEB
            ShowCursor();
#endif
        }
        else
            MainGame.Update();
        // i am scared!!! i scare you!!!
    }
    else
    {
        MainMenu.Update();
        std::string map = MainMenu.LeaveMenu();
        if (!map.empty())
        {
#ifndef PLATFORM_WEB
            HideCursor();
#endif
            InGame = true;
            MainGame.ShouldReturn = false;
            MainGame.Reload(map);
        }
    }
    DrawFPS(0, 0);

#ifdef PLATFORM_WEB
    // This returns the current size of the WASM heap in bytes
    uint32_t heapSize = EM_ASM_INT({
            return HEAP8.length;

    });
    DrawText(TextFormat("WASM Heap: %u MB", heapSize / (1024 * 1024)), 0, 20, 20, GREEN);
#endif

    EndDrawing();
}

int main(int argc, char* argv[])
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BouncingPlus");
    InitAudioDevice();

#ifndef PLATFORM_WEB
    Image t = LoadImage("assets/img/enemy.png");
    SetWindowIcon(t);
    UnloadImage(t);
#ifdef FIREBASE
    Leaderboard MainLeaderboard = Leaderboard();
    double LastUpdatedLeaderboard = 0.0f;
#endif
#endif

    // test commit
    SharedManager SharedMgr = SharedManager();

    Game* MainGame;
    Menu* MainMenu;

#ifdef PLATFORM_WEB
    SharedMgr.FrameRate = 60;
#else
    SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - WINDOW_WIDTH / 2,
                      GetMonitorHeight(GetCurrentMonitor()) / 2 - WINDOW_HEIGHT / 2);
#endif

#ifdef PLATFORM_WEB
    SharedMgr.Load();
    MainGame = new Game(SharedMgr);
    MainMenu = new Menu(SharedMgr);
#else
    GLFWwindow* mainCtx = glfwGetCurrentContext();

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* loaderCtx = glfwCreateWindow(1, 1, "", nullptr, mainCtx);

    bool IsDoneLoading = false;
    LoadingStage ld = {0};
    std::thread cool_thread(load_game, std::ref(MainGame), std::ref(MainMenu), std::ref(SharedMgr), std::ref(loaderCtx), &ld, &IsDoneLoading);

    float anim_state = 0.0f;
    int dots = 0;

    float LoadingTransparency = 1.0f;

    while (!WindowShouldClose() && LoadingTransparency > 0.0f)
    {
        if (IsDoneLoading)
            LoadingTransparency -= 2.0f * GetFrameTime();
        BeginDrawing();
        ClearBackground(BLACK);
        anim_state += GetFrameTime();
        if (anim_state >= 0.4)
        {
            dots += 1;
            if (dots >= 4)
                dots = 0;
            anim_state = 0;
        }

        std::string s = "LOADING";
        for (int i = 0; i < dots; i++)
            s += ".";

        std::string s2 = "";
        std::string s3 = "";

        switch (ld.stage)
        {
        case -1:
            {
                s2 = "Loading UI and save file...";
                break;
            }
        case 0:
            {
                s2 = "Initializing systems...";
                break;
            }
        case 1:
            {
                s2 = "Loading assets from disk...";
                s3 = std::to_string(ld.assets_loaded) + " assets loaded\ntime taken: "
                + std::to_string((int)round(GetTime()- ld.start_time)) + "s";
                break;
            }
        case 2:
            {
                s2 = "Finishing up...";
            }
        }

        DrawText(s.c_str(),GetRenderWidth()/2.0f - MeasureText(s.c_str(),50)/2.0f,
            GetRenderHeight()/2.0f-25.0f,50,ColorAlpha(WHITE,LoadingTransparency));

        DrawText(s2.c_str(),GetRenderWidth()/2.0f - MeasureText(s2.c_str(),20)/2.0f,
            GetRenderHeight()/2.0f+40,20,ColorAlpha(WHITE,LoadingTransparency));

        if (!s3.empty())
        {
            DrawText(s3.c_str(),GetRenderWidth()/2.0f - MeasureText(s3.c_str(),20)/2.0f,
            GetRenderHeight()/2.0f+60,20,ColorAlpha(WHITE,LoadingTransparency));
        }

        EndDrawing();
    }
    cool_thread.join();
    glfwDestroyWindow(loaderCtx);
    MainMenu->BlackTransparency = 1.0f;
#endif

    bool InGame = false;

    SetExitKey(KEY_NULL);

    if (argc == 2 && std::string(argv[1]) == "test")
    {
        MainMenu->Reset();
        MainGame->Reload("debug");
        InGame = true;
    }

    // tip of advice: dont look into any other code file that isnt a manager... youre gonna find some... uhhh... extremely readable code!

    Data d = {
        SharedMgr, MainGame, MainMenu, InGame
    };

#ifdef PLATFORM_WEB
    emscripten_set_main_loop_arg(loop, &d, 0, 1);
#else
    while (!WindowShouldClose())
    {
        if (!d.SharedManager.QuitGame)
        {
            loop(&d);
#ifdef FIREBASE
            if (GetTime() - LastUpdatedLeaderboard >= 120.0f)
            {
                MainLeaderboard.UpdateData();
                LastUpdatedLeaderboard = GetTime();
            }
#endif
        }
        else
        {
            break;
        }
    }
#ifdef FIREBASE
    MainLeaderboard.Quit();
#endif
#endif

    MainMenu->Quit();
    MainGame->Quit();
    SharedMgr.Quit();
    CloseAudioDevice();
    CloseWindow();

    delete MainGame;
    delete MainMenu;

    return 0;
}
