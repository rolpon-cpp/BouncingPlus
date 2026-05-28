cmake_minimum_required(VERSION 4.0)
project(BouncingPlus)
set(CMAKE_CXX_STANDARD 20)
# Dependencies
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif()
endif()
find_package(nlohmann_json CONFIG REQUIRED)
add_executable(BouncingPlus entities/Entity.cpp entities/subentities/enemy/behaviors/CatchBehavior.cpp entities/subentities/enemy/behaviors/EnemyBehavior.cpp entities/subentities/enemy/behaviors/WeaponBehavior.cpp entities/subentities/projectile/Bullet.cpp entities/subentities/enemy/Enemy.cpp entities/subentities/player/Player.cpp entities/subentities/player/PlayerLogicProcessor.cpp entities/subentities/stationary/Spawner.cpp entities/subentities/projectile/Throwable.cpp entities/subentities/stationary/Turret.cpp entities/subentities/stationary/UpgradeStation.cpp entities/systems/Effects.cpp entities/systems/Powerups.cpp entities/systems/Weapons.cpp game/core/Controls.cpp game/core/Profiler.cpp game/core/SharedManager.cpp game/Game.cpp game/managers/CameraManager.cpp game/managers/EntityManager.cpp game/managers/GameModeManager.cpp game/managers/ParticleManager.cpp game/managers/ResourceManager.cpp game/managers/SoundManager.cpp game/ui/Menu.cpp game/ui/UIManager.cpp game/ui/UIUtils.cpp level/LevelLoader.cpp level/tiles/TileManager.cpp main.cpp
        entities/subentities/enemy/behaviors/DwellerBehavior.cpp
        entities/subentities/enemy/behaviors/DwellerBehavior.h
        level/tiles/TileManagerFileRead.cpp
        level/tiles/TileManagerRendering.cpp)
target_link_libraries(BouncingPlus PRIVATE raylib)
target_link_libraries(BouncingPlus PRIVATE nlohmann_json)