cmake_minimum_required(VERSION 4.0)
project(BouncingPlus)

# Base flags
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FIREBASE_SOURCES "")

# --------------------------- GAME CONFIG FLAGS START ---------------------------
option(FIREBASE False)
option(FPS_BOOST True)
# --------------------------- GAME CONFIG FLAGS END ---------------------------

if (OPTIMIZED_REL)
    message(STATUS "Enabling FPS Boost automatically")
    set(FPS_BOOST True)
endif ()

# Adding in firebase
if (MSVC)
    if (FIREBASE)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

        set(FIREBASE_SOURCES src/data/Leaderboard.cpp src/data/Leaderboard.h src/data/keys/firebase_keys.h)

        # Set this to wherever you extracted the SDK. A relative path keeps it portable.
        set(FIREBASE_CPP_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/firebase_cpp_sdk")

        # Firebase's own CMake needs this hint on Windows
        set(FIREBASE_WINDOWS_BUILD TRUE)

        add_subdirectory(${FIREBASE_CPP_SDK_DIR} bin/firebase EXCLUDE_FROM_ALL)

        # Pick only the Firebase products you actually use, but firebase_app is ALWAYS required last.
        # Common targets: firebase_auth, firebase_database, firebase_firestore,
        #                 firebase_storage, firebase_functions, firebase_remote_config
        set(FIREBASE_LIBS
                firebase_auth       # remove/add as needed
                firebase_database   # remove/add as needed
                firebase_app        # must be last
        )
    else ()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endif ()

# Dependencies

# Raylib
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

# JSON Library
find_package(nlohmann_json CONFIG REQUIRED)

# Adding all .cpp + .h files
add_executable(BouncingPlus src/data/Progress.cpp
        src/data/Progress.h
        src/data/SaveConverter.cpp
        src/data/SaveConverter.h
        src/entities/Entity.cpp
        src/entities/Entity.h
        src/entities/EntityType.h
        src/entities/subentities/enemy/behaviors/BehaviorType.h
        src/entities/subentities/enemy/behaviors/CatchBehavior.cpp
        src/entities/subentities/enemy/behaviors/CatchBehavior.h
        src/entities/subentities/enemy/behaviors/DwellerBehavior.cpp
        src/entities/subentities/enemy/behaviors/DwellerBehavior.h
        src/entities/subentities/enemy/behaviors/EnemyBehavior.cpp
        src/entities/subentities/enemy/behaviors/EnemyBehavior.h
        src/entities/subentities/enemy/behaviors/WeaponBehavior.cpp
        src/entities/subentities/enemy/behaviors/WeaponBehavior.h
        src/entities/subentities/enemy/Enemy.cpp
        src/entities/subentities/enemy/Enemy.h
        src/entities/subentities/player/Player.cpp
        src/entities/subentities/player/Player.h
        src/entities/subentities/player/PlayerLogicProcessor.cpp
        src/entities/subentities/player/PlayerLogicProcessor.h
        src/entities/subentities/projectile/Bullet.cpp
        src/entities/subentities/projectile/Bullet.h
        src/entities/subentities/projectile/Throwable.cpp
        src/entities/subentities/projectile/Throwable.h
        src/entities/subentities/stationary/Spawner.cpp
        src/entities/subentities/stationary/Spawner.h
        src/entities/subentities/stationary/Turret.cpp
        src/entities/subentities/stationary/Turret.h
        src/entities/subentities/stationary/UpgradeStation.cpp
        src/entities/subentities/stationary/UpgradeStation.h
        src/entities/systems/Effects.cpp
        src/entities/systems/Effects.h
        src/entities/systems/Powerups.cpp
        src/entities/systems/Powerups.h
        src/entities/systems/Weapons.cpp
        src/entities/systems/Weapons.h
        src/game/core/BaseProfiler.cpp
        src/game/core/BaseProfiler.h
        src/game/core/Controls.cpp
        src/game/core/Controls.h
        src/game/core/GameMisc.cpp
        src/game/core/GameMisc.h
        src/game/core/SharedManager.cpp
        src/game/core/SharedManager.h
        src/game/Game.cpp
        src/game/Game.h
        src/game/managers/CameraManager.cpp
        src/game/managers/CameraManager.h
        src/game/managers/EntityManager.cpp
        src/game/managers/EntityManager.h
        src/game/managers/GameModeManager.cpp
        src/game/managers/GameModeManager.h
        src/game/managers/ParticleManager.cpp
        src/game/managers/ParticleManager.h
        src/game/managers/ResourceManager.cpp
        src/game/managers/ResourceManager.h
        src/game/managers/SoundManager.cpp
        src/game/managers/SoundManager.h
        src/game/ui/gameplay_ui/GameplayUI.cpp
        src/game/ui/gameplay_ui/GameplayUI.h
        src/game/ui/gameplay_ui/GameplayUIHeavy.cpp
        src/game/ui/gameplay_ui/GameplayUIMenus.cpp
        src/game/ui/gameplay_ui/GameplayUIMeters.cpp
        src/game/ui/Menu.cpp
        src/game/ui/Menu.h
        src/game/ui/UIUtils.cpp
        src/game/ui/UIUtils.h
        src/globals.h
        src/level/LevelLoader.cpp
        src/level/LevelLoader.h
        src/level/tiles/TileManager.cpp
        src/level/tiles/TileManager.h
        src/level/tiles/TileManagerFileRead.cpp
        src/level/tiles/TileManagerRendering.cpp
        src/level/tiles/TileType.h
        src/main.cpp
        ${FIREBASE_SOURCES}
)

if (MSVC)
    if (FIREBASE)
        target_compile_options(BouncingPlus PRIVATE -DFIREBASE)
        add_custom_command(TARGET BouncingPlus POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${FIREBASE_CPP_SDK_DIR}/libs/windows/VS2019/MD/x64/Release"
                $<TARGET_FILE_DIR:BouncingPlus>
                COMMENT "Copying Firebase DLLs..."
        )
    endif ()
endif ()

# Linking all of the libraries together
target_link_libraries(BouncingPlus PRIVATE raylib)
target_link_libraries(BouncingPlus PRIVATE nlohmann_json)
target_link_libraries(BouncingPlus PRIVATE ${FIREBASE_LIBS})

# Optional FPS Boost Flag
if (FPS_BOOST)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

if (ipo_supported)
    set_target_properties(BouncingPlus PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )
endif ()
endif ()

# MinGW support
if (MINGW)
    target_link_options(BouncingPlus PRIVATE
            -static
    )
endif ()

# Win32 / Game icon
if(WIN32)
    target_sources(BouncingPlus PRIVATE resources.rc)
endif()