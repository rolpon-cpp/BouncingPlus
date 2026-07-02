cd ..
call C:\Emscripten\emsdk\emsdk_env.bat
if not exist build-web mkdir build-web
em++ ^
    src/data/Progress.cpp ^
    src/data/SaveConverter.cpp ^
    src/entities/Entity.cpp ^
    src/entities/subentities/enemy/behaviors/CatchBehavior.cpp ^
    src/entities/subentities/enemy/behaviors/DwellerBehavior.cpp ^
    src/entities/subentities/enemy/behaviors/EnemyBehavior.cpp ^
    src/entities/subentities/enemy/behaviors/WeaponBehavior.cpp ^
    src/entities/subentities/enemy/Enemy.cpp ^
    src/entities/subentities/player/Player.cpp ^
    src/entities/subentities/player/PlayerLogicProcessor.cpp ^
    src/entities/subentities/projectile/Bullet.cpp ^
    src/entities/subentities/projectile/Throwable.cpp ^
    src/entities/subentities/stationary/Spawner.cpp ^
    src/entities/subentities/stationary/Turret.cpp ^
    src/entities/systems/Effects.cpp ^
    src/entities/systems/Powerups.cpp ^
    src/entities/systems/Weapons.cpp ^
    src/game/core/BaseProfiler.cpp ^
    src/game/core/Controls.cpp ^
    src/game/core/GameMisc.cpp ^
    src/game/core/SharedManager.cpp ^
    src/game/Game.cpp ^
    src/game/managers/CameraManager.cpp ^
    src/game/managers/EntityManager.cpp ^
    src/game/managers/GameModeManager.cpp ^
    src/game/managers/ParticleManager.cpp ^
    src/game/managers/ResourceManager.cpp ^
    src/game/managers/SoundManager.cpp ^
    src/game/ui/gameplay_ui/GameplayUI.cpp ^
    src/game/ui/gameplay_ui/GameplayUIHeavy.cpp ^
    src/game/ui/gameplay_ui/GameplayUIMenus.cpp ^
    src/game/ui/gameplay_ui/GameplayUIMeters.cpp ^
    src/game/ui/Menu.cpp ^
    src/game/ui/UIUtils.cpp ^
    src/level/LevelLoader.cpp ^
    src/level/tiles/TileManager.cpp ^
    src/level/tiles/TileManagerFileRead.cpp ^
    src/level/tiles/TileManagerRendering.cpp ^
    src/main.cpp ^
    -o build-web/index.html ^
    -I . ^
    -I C:\Emscripten\raylib\src ^
    -I C:\Emscripten\json ^
    -L C:\Emscripten\raylib\build-web\raylib ^
    -lraylib ^
    -sUSE_GLFW=3 ^
	-sUSE_WEBGL2=1 ^
	-DPLATFORM_WEB ^
	-DGRAPHICS_API_OPENGL_ES3 ^
	-sFULL_ES3 ^
	-sMIN_WEBGL_VERSION=2 ^
	-sEXCEPTION_CATCHING_ALLOWED=["__cxa_throw"] ^
    -sWASM=1 ^
    -O3 ^
    -sASYNCIFY ^
	-sSTACK_SIZE=7242880 ^
	-sEXPORTED_RUNTIME_METHODS=HEAPF32 ^
	-std=c++23 ^
    -sALLOW_MEMORY_GROWTH=1 ^
    --preload-file assets ^
    --memoryprofiler ^
    --shell-file C:\Emscripten\raylib\src\minshell.html