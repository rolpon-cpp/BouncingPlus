//
// Created by Rolpon on 5/27/2026.
//

#include "TileManager.h"
#include "../../game/managers/EntityManager.h"
#include "../../game/managers/ResourceManager.h"
#include "../../game/managers/GameModeManager.h"
#include "../../game/core/SharedManager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "../../entities/subentities/enemy/Enemy.h"
#include "../../game/Game.h"
#include "../../entities/subentities/stationary/Spawner.h"
#include "../../entities/subentities/stationary/Turret.h"
#include "../../entities/subentities/stationary/UpgradeStation.h"
#include "../../entities/subentities/enemy/behaviors/CatchBehavior.h"
#include "../../entities/subentities/enemy/behaviors/DwellerBehavior.h"

void TileManager::ProcessTile(std::string cell, int x, int y, bool* PlayerSpawnFound)
{
    int tile_id = 0;
    if (std::isdigit(cell[0]))
    {
        try
        {
            tile_id = std::stoi(cell) + 1;
        } catch (std::invalid_argument& e)
        {
            cout << e.what() << "\n";
        }
    }

    if (tile_id < 0 || tile_id >= 15) {
        tile_id = 0;
    }

    Map.push_back(tile_id < 3 || tile_id == 12 ? tile_id : -1);

    float bbox_x = (static_cast<float>(x) * TileSize) + TileSize / 2.0f;
    float bbox_y = (static_cast<float>(y) * TileSize) + TileSize / 2.0f;

    switch (TileTypes[tile_id]) {
    case EnemyTileType:
        AddEnemy(bbox_x, bbox_y, tile_id);
        break;
    case EnemySpawnTileType:
        EnemySpawnLocations.push_back({bbox_x,bbox_y});
        break;
    case PlayerSpawnTileType:
        *PlayerSpawnFound = true;
        PlayerSpawnPosition = Vector2{bbox_x, bbox_y};
        break;
    case SpawnerTileType: {
            std::shared_ptr<Spawner> spawner = std::make_shared<Spawner>(*game, bbox_x, bbox_y);
            game->GameEntities->AddEntity(SpawnerType, spawner);
            break;
    }
    case BossTileType: {
            BossSpawnPosition = Vector2{bbox_x, bbox_y};
            break;
    };
    case UpgradeStationTileType: {
            std::shared_ptr<UpgradeStation> station = std::make_shared<UpgradeStation>(*game, bbox_x, bbox_y);
            game->GameEntities->AddEntity(UpgradeStationType, station);
            break;
    }
    case TurretTileType: {
            std::shared_ptr<Turret> t = std::make_shared<Turret>(*game,
                "Sniper", bbox_x, bbox_y);
            game->GameEntities->AddEntity(TurretType, t);
            break;
    }
    case CatchEnemyTileType:
        {

            std::shared_ptr<Enemy> e= make_shared<Enemy>(bbox_x, bbox_y, 100.0f, 0.0f, GetRandomValue(0, 25), "", game->GameResources->Textures["enemy"], *game);
            std::unique_ptr<EnemyBehavior> catch_behavior = std::make_unique<CatchBehavior>(*e, *game);
            e->Behavior.reset();
            e->Behavior = std::move(catch_behavior);

            game->GameEntities->AddEntity(EnemyType, e);
            break;
        }
    case DwellerEnemyTileType:
        {
            std::shared_ptr<Enemy> e= make_shared<Enemy>(bbox_x, bbox_y, 100.0f, 0.0f, GetRandomValue(0, 25), "Enemy Sword", game->GameResources->Textures["enemy"], *game);
            std::unique_ptr<EnemyBehavior> dwell_behavior = std::make_unique<DwellerBehavior>(*e, *game);
            auto* db = dynamic_cast<DwellerBehavior*>(dwell_behavior.get());
            db->HasMovedIntoPlace = true;

            e->Behavior.reset();
            e->Behavior = std::move(dwell_behavior);

            game->GameEntities->AddEntity(EnemyType, e);
            break;
        }
    case NothingTileType:
        {
            break;
        }
    case WallTileType:
        {
            break;
        }
    case EnemyWallTileType:
        {
            break;
        }
    case TileTypeEnd:
        {
            break;
        }
    }
}

void TileManager::ReadMapDataFile(std::string FileName) {
    int y = 0;
    int x = 0;

    Clear();

    bool PlayerSpawnFound = false;

    if (PrevFileName != FileName)
    {
        Lines.clear();
        std::ifstream  data(FileName);

        std::string da;
        while(std::getline(data,da))
            Lines.push_back(da);

        data.close();
    }

    for (const std::string& line : Lines)
    {
        std::stringstream  lineStream(line);
        std::string        cell;
        while(std::getline(lineStream,cell,','))
        {
            ProcessTile(cell,x,y,&PlayerSpawnFound);
            x += 1;
        }
        if (x > MapWidth) {
            MapWidth = x;
        }
        x = 0;
        y += 1;
    }

    MapHeight = y;

    if (!PlayerSpawnFound)
        PlayerSpawnPosition = {MapWidth * TileSize / 2.0f, MapHeight * TileSize / 2.0f};

    PrevFileName = FileName;
}

void TileManager::CreateFileEntity(FileEntity& NewFileEntity)
{
    // TEMPORARY CHECK, SINCE OTHER ENTITIES ARE NOT SUPPORTED
    if (NewFileEntity.Type != EnemyType)
        return;
    // REMOVE THIS IF YOU PLAN TO INCLUDE MORE ENTITY TYPES!

    std::shared_ptr<Entity> NewEntity;

    if (NewFileEntity.Type == EnemyType)
    {
        NewEntity = make_shared<Enemy>(NewFileEntity.X, NewFileEntity.Y, NewFileEntity.Health, NewFileEntity.Speed,
            NewFileEntity.Armor, NewFileEntity.Weapon, game->GameResources->Textures["enemy"], *game);
    }

    NewEntity->BoundingBox.width = NewFileEntity.W;
    NewEntity->BoundingBox.height = NewFileEntity.H;

    game->GameEntities->AddEntity(NewEntity->Type, NewEntity);
}

void TileManager::SetPropertiesOfFileEntity(FileEntity& ThisFileEntity, int i, std::string cell)
{
    try
    {
        if (cell.starts_with(" "))
            cell = cell.substr(1, cell.size() - 1);
        if (i < 8 && !std::isdigit(cell[0]))
        {
            cout << "CELL is invalid " << cell << " " << i << "\n";
            return;
        }
        switch (i)
        {
        case 0:
            {
                if (std::stoi(cell) >= 0 && std::stoi(cell) < End)
                {
                    ThisFileEntity.Type = (EntityType)std::stoi(cell);
                } else
                {
                    ThisFileEntity.Type = DefaultType;
                }
                break;
            }
        case 1:
            ThisFileEntity.X = (float)std::stoi(cell);
            break;
        case 2:
            ThisFileEntity.Y = (float)std::stoi(cell);
            break;
        case 3:
            ThisFileEntity.W = (float)std::stoi(cell);
            break;
        case 4:
            ThisFileEntity.H = (float)std::stoi(cell);
            break;
        case 5:
            ThisFileEntity.Health = (float)std::stoi(cell);
            break;
        case 6:
            ThisFileEntity.Armor = (float)std::stoi(cell);
            break;
        case 7:
            ThisFileEntity.Speed = (float)std::stoi(cell);
            break;
        case 8:
            ThisFileEntity.Weapon = cell;
            break;
        }
    } catch (std::invalid_argument& e)
    {
        cout << e.what() << "\n";
    }
}

void TileManager::ReadEntitiesFile(std::string FileName)
{
    std::ifstream  data(FileName);

    std::string line;
    while(std::getline(data,line))
    {
        if (line.starts_with("Type"))
            continue;
        std::stringstream lineStream(line);
        std::string cell;

        FileEntity ThisFileEntity = {DefaultType};

        int i = 0;

        while(std::getline(lineStream,cell,','))
        {
            SetPropertiesOfFileEntity(ThisFileEntity,i,cell);
            i++;
        }

        if (i > 8)
            CreateFileEntity(ThisFileEntity);
    }
}

void TileManager::AddEnemy(float bbox_x, float bbox_y, int tile_id) {
    std::string Weapon = "Default Gun";
    float Speed = 350.0f;
    float Health = 100.0f;
    float Armor = 0.0f;
    switch (tile_id)
    {
    case 4:
        Weapon = game->EnemyRoleWeapons["sword"];
        Speed = 280.0f;
        Health = 90.0f;
        Armor = 50.0f;
        break;
    case 5:
        Weapon = game->EnemyRoleWeapons["shotgun"];
        Speed = 300.0f;
        Health = 140.0f;
        Armor = 0.0f;
        break;
    }
    float Multiplier = 1.0f + 0.15f * (game->GameShared->LevelData[game->GameMode->GetCurrentLevelName()]["difficulty"].get<int>()-3);
    Armor *= Multiplier;
    Speed *= Multiplier;
    Health *= Multiplier;
    game->GameEntities->AddEntity(EnemyType, make_shared<Enemy>(bbox_x, bbox_y, Health, Speed, Armor, Weapon, game->GameResources->Textures["enemy"], *game));
}