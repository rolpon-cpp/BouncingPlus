//
// Created by Rolpon on 8/27/2025.
//

#include "TileManager.h"
#include <string>
#include <nlohmann/json.hpp>
#include "../../game/Game.h"
#include "../../game/managers/CameraManager.h"

TileManager::TileManager()
{
}

TileManager::TileManager(Game* game)
{
    this->game = game;
    PrevFileName = "";
    Lines = std::vector<std::string>();
    Distortions = std::vector<Distortion>();
    TileMapTex = LoadRenderTexture(1, 1);
    TileTypes[0] = NothingTileType; // air
    TileTypes[-1] = NothingTileType; // air
    TileTypes[1] = WallTileType; // bouncy wall
    TileTypes[2] = WallTileType; // delete wall
    TileTypes[3] = EnemyTileType; // enemy
    TileTypes[4] = EnemyTileType; // sword
    TileTypes[5] = EnemyTileType; // shotgun
    TileTypes[6] = PlayerSpawnTileType; // player spawn
    TileTypes[7] = SpawnerTileType; // enemy spawner tile type
    TileTypes[8] = EnemySpawnTileType; // enemy spawn tile type
    TileTypes[9] = UpgradeStationTileType; // upgrade station tile type
    TileTypes[10] = BossTileType; // boss
    TileTypes[11] = TurretTileType; // turrets
    TileTypes[12] = EnemyWallTileType; // enemy walls
    TileTypes[13] = CatchEnemyTileType; // catch enemy
    TileTypes[14] = DwellerEnemyTileType; // dweller enemy
    Clear();
}

int TileManager::GetTileAt(int x, int y)
{
    int idx = y * MapWidth + x;
    if (x < 0 || y < 0 || x >= MapWidth || y >= MapHeight)
        return -1;
    if (Map.size() <= idx)
        return -1;
    return Map[idx];
}

int TileManager::GetTileAt(Vector2 coord)
{
    return GetTileAt(coord.x, coord.y);
}

void TileManager::SetTileAt(int x, int y, int id)
{
    int idx = y * MapWidth + x;
    if (x < 0 || y < 0 || x >= MapWidth || y >= MapHeight)
        return;
    if (Map.size() <= idx)
        return;
    Map[idx] = id;
}

int TileManager::GetTileAtWorldCoords(Vector2 coord)
{
    int x = (int)(coord.x / TileSize);
    int y = (int)(coord.y / TileSize);
    return GetTileAt(x, y);
}

void TileManager::SetTileAt(Vector2 coord, int id)
{
    SetTileAt(coord.x, coord.y, id);
}

void TileManager::Clear()
{
    Map.clear();
    Distortions.clear();
    MapWidth = 0;
    MapHeight = 0;
    PlayerSpawnPosition = {0, 0};
    BossSpawnPosition = {0, 0};
    EnemySpawnLocations = std::vector<Vector2>();
    FXLifetime = 0.75f;
    TileSize = 72;
    if (IsRenderTextureValid(TileMapTex))
        UnloadRenderTexture(TileMapTex);
    TileMapTex = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
}

void TileManager::Quit()
{
    Clear();
    if (IsRenderTextureValid(TileMapTex))
        UnloadRenderTexture(TileMapTex);
}