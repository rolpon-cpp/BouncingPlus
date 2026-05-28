echo this script was made by Claude AI!!!
cd ..
call C:\Emscripten\emsdk\emsdk_env.bat
if not exist build-web mkdir build-web
em++ ^
    entities/Entity.cpp ^
    entities/subentities/enemy/behaviors/CatchBehavior.cpp ^
    entities/subentities/enemy/behaviors/DwellerBehavior.cpp ^
    entities/subentities/enemy/behaviors/EnemyBehavior.cpp ^
    entities/subentities/enemy/behaviors/WeaponBehavior.cpp ^
    entities/subentities/enemy/Enemy.cpp ^
    entities/subentities/player/Player.cpp ^
    entities/subentities/player/PlayerLogicProcessor.cpp ^
    entities/subentities/projectile/Bullet.cpp ^
    entities/subentities/projectile/Throwable.cpp ^
    entities/subentities/stationary/Spawner.cpp ^
    entities/subentities/stationary/Turret.cpp ^
    entities/subentities/stationary/UpgradeStation.cpp ^
    entities/systems/Effects.cpp ^
    entities/systems/Powerups.cpp ^
    entities/systems/Weapons.cpp ^
    game/core/Controls.cpp ^
    game/core/Profiler.cpp ^
    game/core/SharedManager.cpp ^
    game/Game.cpp ^
    game/managers/CameraManager.cpp ^
    game/managers/EntityManager.cpp ^
    game/managers/GameModeManager.cpp ^
    game/managers/ParticleManager.cpp ^
    game/managers/ResourceManager.cpp ^
    game/managers/SoundManager.cpp ^
    game/ui/Menu.cpp ^
    game/ui/UIManager.cpp ^
    game/ui/UIUtils.cpp ^
    level/LevelLoader.cpp ^
    level/tiles/TileManager.cpp ^
    level/tiles/TileManagerFileRead.cpp ^
    level/tiles/TileManagerRendering.cpp ^
    main.cpp ^
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
    -sASYNCIFY ^
	-sSTACK_SIZE=7242880 ^
	-sEXPORTED_RUNTIME_METHODS=HEAPF32 ^
	-std=c++20 ^
    -sALLOW_MEMORY_GROWTH=1 ^
    --preload-file assets ^
    --shell-file C:\Emscripten\raylib\src\minshell.html