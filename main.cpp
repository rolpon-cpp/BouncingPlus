#include "raylib.h"
#include "game/Game.h"
#include "game/ui/Menu.h"
#include "game/core/SharedManager.h"
#include "level/LevelLoader.h"

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#define WINDOW_WIDTH 1480.0f
#define WINDOW_HEIGHT 920.0f

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
    SharedManager& SharedManager=d->SharedManager;
    Game& MainGame=d->MainGame;
    Menu& MainMenu=d->MainMenu;
    bool& InGame=d->InGame;

    #ifdef PLATFORM_WEB
        SetMouseScale((float)GetRenderWidth() / WINDOW_WIDTH, (float)GetRenderHeight() / WINDOW_HEIGHT);
    #endif

    BeginDrawing();

    SharedManager.Update();

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

    Image t =LoadImage("assets/img/player.png");
    SetWindowIcon(t);
    UnloadImage(t);

    LevelLoader level_loader = LevelLoader();
    std::map<std::string,json> level_data = level_loader.GetLevelsData();

    SharedManager SharedManager{};
    SharedManager.LevelData = level_data;
    SharedManager.UIAssets = UIAssets();
    SharedManager.UIAssets.Load();
    SharedManager.Controls.SetDefaultBindings();

    Game MainGame = Game(SharedManager);
    Menu MainMenu = Menu(SharedManager);

    bool InGame = false;

    #ifdef PLATFORM_WEB
        SharedManager.FrameRate = 60;
    #else
        SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        SetWindowSize(GetMonitorWidth(GetCurrentMonitor()) / 1.2f, GetMonitorHeight(GetCurrentMonitor()) / 1.2f);
        SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - GetRenderWidth()/2, GetMonitorHeight(GetCurrentMonitor())/2 - GetRenderHeight()/2);
        SharedManager.FrameRate = max(min(GetMonitorRefreshRate(GetCurrentMonitor()) + 60,240),0);
    #endif

    SetExitKey(KEY_NULL);

    // tip of advice: dont look into any other code file that isnt a manager... youre gonna find some... uhhh... extremely readable code!

    Data d = {
        SharedManager,MainGame,MainMenu,InGame
    };

    #ifdef PLATFORM_WEB
        emscripten_set_main_loop_arg(loop, &d, 0, 1);
    #else
        while (!WindowShouldClose())
            loop(&d);
    #endif


    MainMenu.Quit();
    MainGame.Quit();
    SharedManager.Quit();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
