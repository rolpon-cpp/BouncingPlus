cmake_minimum_required(VERSION 4.0)
project(BouncingPlus)
set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

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
add_executable(BouncingPlus src/entities/Entity.cpp src/entities/Entity.h src/entities/EntityType.h src/entities/subentities/enemy/behaviors/BehaviorType.h src/entities/subentities/enemy/behaviors/CatchBehavior.cpp src/entities/subentities/enemy/behaviors/CatchBehavior.h src/entities/subentities/enemy/behaviors/DwellerBehavior.cpp src/entities/subentities/enemy/behaviors/DwellerBehavior.h src/entities/subentities/enemy/behaviors/EnemyBehavior.cpp src/entities/subentities/enemy/behaviors/EnemyBehavior.h src/entities/subentities/enemy/behaviors/WeaponBehavior.cpp src/entities/subentities/enemy/behaviors/WeaponBehavior.h src/entities/subentities/enemy/Enemy.cpp src/entities/subentities/enemy/Enemy.h src/entities/subentities/player/Player.cpp src/entities/subentities/player/Player.h src/entities/subentities/player/PlayerLogicProcessor.cpp src/entities/subentities/player/PlayerLogicProcessor.h src/entities/subentities/projectile/Bullet.cpp src/entities/subentities/projectile/Bullet.h src/entities/subentities/projectile/Throwable.cpp src/entities/subentities/projectile/Throwable.h src/entities/subentities/stationary/Spawner.cpp src/entities/subentities/stationary/Spawner.h src/entities/subentities/stationary/Turret.cpp src/entities/subentities/stationary/Turret.h src/entities/subentities/stationary/UpgradeStation.cpp src/entities/subentities/stationary/UpgradeStation.h src/entities/systems/Effects.cpp src/entities/systems/Effects.h src/entities/systems/Powerups.cpp src/entities/systems/Powerups.h src/entities/systems/Weapons.cpp src/entities/systems/Weapons.h src/game/core/Controls.cpp src/game/core/Controls.h src/game/core/Profiler.cpp src/game/core/Profiler.h src/game/core/SharedManager.cpp src/game/core/SharedManager.h src/game/Game.cpp src/game/Game.h src/game/managers/CameraManager.cpp src/game/managers/CameraManager.h src/game/managers/EntityManager.cpp src/game/managers/EntityManager.h src/game/managers/GameModeManager.cpp src/game/managers/GameModeManager.h src/game/managers/ParticleManager.cpp src/game/managers/ParticleManager.h src/game/managers/ResourceManager.cpp src/game/managers/ResourceManager.h src/game/managers/SoundManager.cpp src/game/managers/SoundManager.h src/game/ui/Menu.cpp src/game/ui/Menu.h src/game/ui/UIManager.cpp src/game/ui/UIManager.h src/game/ui/UIUtils.cpp src/game/ui/UIUtils.h src/level/LevelLoader.cpp src/level/LevelLoader.h src/level/tiles/TileManager.cpp src/level/tiles/TileManager.h src/level/tiles/TileManagerFileRead.cpp src/level/tiles/TileManagerRendering.cpp src/level/tiles/TileType.h src/main.cpp
        src/data/Progress.cpp
        src/data/Progress.h)
target_link_libraries(BouncingPlus PRIVATE raylib)
target_link_libraries(BouncingPlus PRIVATE nlohmann_json)

if (MINGW)
    target_link_options(BouncingPlus PRIVATE
            -static
    )
endif ()

if(WIN32)
    target_sources(BouncingPlus PRIVATE resources.rc)
endif()