#include "raylib.h"
#include "game/Game.h"
#include "game/ui/Menu.h"
#include "globals.h"
#include "game/core/SharedManager.h"
#include "level/LevelLoader.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

struct Data
{
    SharedManager& SharedManager;
    Game& MainGame;
    Menu& MainMenu;
    bool& InGame;
};

void loop(void* arg)
{

    Data* d = (Data*)arg;
    SharedManager& SharedMgr=d->SharedManager;
    Game& MainGame=d->MainGame;
    Menu& MainMenu=d->MainMenu;
    bool& InGame=d->InGame;

    #ifdef PLATFORM_WEB
        SetMouseScale((float)GetRenderWidth() / WINDOW_WIDTH, (float)GetRenderHeight() / WINDOW_HEIGHT);
    #endif

    BeginDrawing();

    SharedMgr.Update();

    ClearBackground(BLANK);

    if (InGame) {
        if (MainGame.ShouldReturn) {
            InGame = false;
            MainMenu.Reset();
            MainGame.ShouldReturn = false;
            MainGame.Clear();

#ifndef PLATFORM_WEB
            ShowCursor();
#endif
        } else
            MainGame.Update();
        // i am scared!!! i scare you!!!
    } else {
        MainMenu.Update();
        std::string map = MainMenu.LeaveMenu();
        if (!map.empty()) {
#ifndef PLATFORM_WEB
            HideCursor();
#endif
            InGame = true;
            MainGame.ShouldReturn = false;
            MainGame.Reload(map);
        }
    }
    DrawFPS(0,0);

#ifdef PLATFORM_WEB
    // This returns the current size of the WASM heap in bytes
    uint32_t heapSize = EM_ASM_INT({
        return HEAP8.length;
    });
    DrawText(TextFormat("WASM Heap: %u MB", heapSize / (1024 * 1024)), 0, 20, 20, GREEN);
#endif

    EndDrawing();
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BouncingPlus");
    InitAudioDevice();

    #ifndef PLATFORM_WEB
        Image t =LoadImage("assets/img/enemy.png");
        SetWindowIcon(t);
        UnloadImage(t);
    #endif

    SharedManager SharedMgr = SharedManager();

    Game MainGame = Game(SharedMgr);
    Menu MainMenu = Menu(SharedMgr);

    bool InGame = false;

    #ifdef PLATFORM_WEB
        SharedMgr.FrameRate = 60;
    #else
        SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - WINDOW_WIDTH/2, GetMonitorHeight(GetCurrentMonitor())/2 - WINDOW_HEIGHT/2);
    #endif

    SetExitKey(KEY_NULL);

    if (argc == 2 && string(argv[1]) == "test")
    {
        MainMenu.Reset();
        MainGame.Reload("debug");
        InGame = true;
    }

    // tip of advice: dont look into any other code file that isnt a manager... youre gonna find some... uhhh... extremely readable code!

    Data d = {
        SharedMgr,MainGame,MainMenu,InGame
    };

    #ifdef PLATFORM_WEB
        emscripten_set_main_loop_arg(loop, &d, 0, 1);
    #else
        while (!WindowShouldClose())
            loop(&d);
    #endif

    MainMenu.Quit();
    MainGame.Quit();
    SharedMgr.Quit();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
