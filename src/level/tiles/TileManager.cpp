//
// Created by Rolpon on 8/27/2025.
//

#include "TileManager.h"
#include <string>
#include <nlohmann/json.hpp>
#include "../../game/Game.h"
#include "../../game/managers/CameraManager.h"
#include "../../game/managers/GameModeManager.h"

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
    TileTypes[-1] = NothingTileType; // air
    TileTypes[0] = NothingTileType; // air
    TileTypes[1] = WallTileType; // bouncy wall
    TileTypes[2] = WallTileType; // delete wall
    TileTypes[3] = EnemyTileType; // enemy
    TileTypes[4] = EnemyTileType; // sword
    TileTypes[5] = EnemyTileType; // shotgun
    TileTypes[6] = PlayerSpawnTileType; // player spawn
    TileTypes[7] = SpawnerTileType; // enemy spawner tile type
    TileTypes[8] = EnemySpawnTileType; // enemy spawn tile type
    TileTypes[9] = NothingTileType; // upgrade station tile type
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

Vector2 TileManager::GetWorldCenter()
{
    return {MapWidth * TileSize * .5f, MapHeight * TileSize * .5f};
}

void TileManager::PrepareAsInfiniteMode()
{
    Clear();
    MapWidth = 1000;
    MapHeight = 1000;
    for (int i = 0; i < MapWidth * MapHeight; i++)
        Map[i] = 0;
}

void TileManager::Update()
{
    if (game->GameMode->InfiniteMode)
    {
        // TODO: implement entity spawning logic & tile placement
    }
    Display();
}

void TileManager::Clear()
{
    Map.clear();
    InfiniteChunksGenerated.clear();
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

CollisionData TileManager::IsColliding(Rectangle BoundingBox, std::vector<TileType> tile_types, std::vector<Vector2> exclusions)
{
    int tile_x = static_cast<int>(BoundingBox.x / game->GameTiles->TileSize);
    int tile_y = static_cast<int>(BoundingBox.y / game->GameTiles->TileSize);

    int spaces_occupied_x = ceil(BoundingBox.width / game->GameTiles->TileSize);
    int spaces_occupied_y = ceil(BoundingBox.height / game->GameTiles->TileSize);

    for (int y = 0; y < (spaces_occupied_y + 2); y++)
    {
        for (int x = 0; x < (spaces_occupied_x + 2); x++)
        {
            int curr_tile_x = tile_x + x - spaces_occupied_x;
            int curr_tile_y = tile_y + y - spaces_occupied_y;
            int tile_id = game->GameTiles->GetTileAt(curr_tile_x,curr_tile_y);
            Rectangle bbox = Rectangle{curr_tile_x * game->GameTiles->TileSize, curr_tile_y * game->GameTiles->TileSize,
                game->GameTiles->TileSize, game->GameTiles->TileSize};

            bool found = false;

            for (Vector2 exclusion : exclusions)
            {
                if (exclusion.x == curr_tile_x && exclusion.y == curr_tile_y)
                {
                    found = true;
                    break;
                }
            }

            bool found_tile_type = false;

            if (game->GameTiles->TileTypes[tile_id] == WallTileType)
                found_tile_type = true;
            else if (!tile_types.empty())
            {
                for (TileType type : tile_types)
                {
                    if (game->GameTiles->TileTypes[tile_id] == type)
                    {
                        found_tile_type = true;
                        break;
                    }
                }
            }

            if (found_tile_type && CheckCollisionRecs(BoundingBox, bbox) && !found)
            {
                Vector2 Normal = {0, 0};

                Vector2 BBoxCenter ={BoundingBox.x + BoundingBox.width/2.0f, BoundingBox.y + BoundingBox.height/2.0f};
                Vector2 TileCenter = {bbox.x + bbox.width/2.0f, bbox.y + bbox.height/2.0f};

                if (BBoxCenter.y < TileCenter.y)
                    Normal.y = -1.0f;
                if (BBoxCenter.y > TileCenter.y)
                    Normal.y = 1.0f;
                if (BBoxCenter.x < TileCenter.x)
                    Normal.x = -1.0f;
                if (BBoxCenter.x > TileCenter.x)
                    Normal.x = 1.0f;

                return CollisionData{
                    true,
                    tile_id,
                    Normal,
                    {static_cast<float>(curr_tile_x),static_cast<float>(curr_tile_y)}
                };
            }
        }
    }
    return CollisionData{};
}

void TileManager::Quit()
{
    Clear();
    if (IsRenderTextureValid(TileMapTex))
        UnloadRenderTexture(TileMapTex);
}
