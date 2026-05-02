//
// Created by lalit on 8/27/2025.
//

#include "TileManager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <raymath.h>
#include <string>
#include <nlohmann/json.hpp>

#include "../entities/subentities/Enemy.h"
#include "../game/Game.h"
#include "../entities/subentities/Spawner.h"
#include "../entities/subentities/Turret.h"
#include "../entities/subentities/UpgradeStation.h"

TileManager::TileManager() {

}

TileManager::TileManager(Game &game) {
    this->game = &game;
    PrevFileName = "";
    Lines = std::vector<std::string>();
    Distortions = std::vector<Distortion>();
    TileMapTex = LoadRenderTexture(1,1);
    ForceFieldTex = LoadRenderTexture(1,1);
    TileTypes[0] = NothingTileType; // air
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
    return GetTileAt(coord.x,coord.y);
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

void TileManager::SetTileAt(Vector2 coord, int id)
{
    SetTileAt(coord.x, coord.y, id);
}

void TileManager::DrawWallTile(int curr_tile_x, int curr_tile_y, Texture* tile_tex)
{
    float bbox_x = curr_tile_x * TileSize;
    float bbox_y = curr_tile_y * TileSize;
    Rectangle rec = {0, 0, (float)tile_tex->width, (float)tile_tex->height};

    bool left = TileTypes[GetTileAt(curr_tile_x - 1, curr_tile_y)] == WallTileType;
    bool right = TileTypes[GetTileAt(curr_tile_x + 1, curr_tile_y)] == WallTileType;
    bool up = TileTypes[GetTileAt(curr_tile_x, curr_tile_y - 1)] == WallTileType;
    bool down = TileTypes[GetTileAt(curr_tile_x, curr_tile_y + 1)] == WallTileType;

    bool diagonal_lu = TileTypes[GetTileAt(curr_tile_x - 1, curr_tile_y - 1)] == WallTileType;
    bool diagonal_ru = TileTypes[GetTileAt(curr_tile_x + 1, curr_tile_y - 1)] == WallTileType;
    bool diagonal_ld = TileTypes[GetTileAt(curr_tile_x - 1, curr_tile_y + 1)] == WallTileType;
    bool diagonal_rd = TileTypes[GetTileAt(curr_tile_x + 1, curr_tile_y + 1)] == WallTileType;

    if (left)
    {
        rec.x = 4;
        rec.width -= 4;
    }
    if (right)
    {
        rec.width -= 4;
    }
    if (up)
    {
        rec.y = 4;
        rec.height -= 4;
    }
    if (down)
    {
        rec.height -= 4;
    }

    if (left)
    {
        DrawTexturePro(*tile_tex, {10, 0, 1, 36}, Rectangle{bbox_x,
                                                            bbox_y, 8, 72}, {0, 0}, 0, WHITE);
    }
    if (right)
    {
        DrawTexturePro(*tile_tex, {10, 0, 1, 36}, Rectangle{bbox_x + TileSize - 8,
                                                            bbox_y, 8, 72}, {0, 0}, 0, WHITE);
    }
    if (up)
    {
        DrawTexturePro(*tile_tex, {0, 10, 36, 1}, Rectangle{bbox_x,
                                                            bbox_y, 72, 8}, {0, 0}, 0, WHITE);
    }
    if (down)
    {
        DrawTexturePro(*tile_tex, {0, 10, 36, 1}, Rectangle{bbox_x,
                                                            bbox_y + TileSize - 8, 72, 8}, {0, 0}, 0, WHITE);
    }

    DrawTexturePro(*tile_tex, rec, Rectangle{bbox_x + (rec.x * 2),
                                             bbox_y + (rec.y * 2), rec.width * 2, rec.height * 2}, {0, 0}, 0, WHITE);

    if (left && up && diagonal_lu)
        DrawTexturePro(*tile_tex, {10, 10, 1, 1},
                       {bbox_x, bbox_y, 8, 8},
                       {0, 0}, 0,WHITE);
    if (left && down && diagonal_ld)
        DrawTexturePro(*tile_tex, {10, 10, 1, 1},
                       {bbox_x, bbox_y + TileSize - 8, 8, 8},
                       {0, 0}, 0,WHITE);
    if (right && up && diagonal_ru)
        DrawTexturePro(*tile_tex, {10, 10, 1, 1},
                       {bbox_x + TileSize - 8, bbox_y, 8, 8},
                       {0, 0}, 0,WHITE);
    if (right && down && diagonal_rd)
        DrawTexturePro(*tile_tex, {10, 10, 1, 1},
                       {bbox_x + TileSize - 8, bbox_y + TileSize - 8, 8, 8},
                       {0, 0}, 0,WHITE);
}

void TileManager::DrawTileMap()
{

    vector<Vector2> ForceFieldPos;

    Vector2 *CameraPosition = &this->game->GameCamera.CameraPosition;
    int tile_x = static_cast<int> ((CameraPosition->x + GetRenderWidth()/2) / TileSize);
    int tile_y = static_cast<int> ((CameraPosition->y + GetRenderHeight()/2) / TileSize);
    for (int y = 0; y < UpdateDistance.y; y++) {
        for (int x = 0; x < UpdateDistance.x; x++) {
            int curr_tile_x = tile_x + x - static_cast<int> (UpdateDistance.x / 2);
            int curr_tile_y = tile_y + y - static_cast<int> (UpdateDistance.y / 2);
            int tile_id = GetTileAt(curr_tile_x,curr_tile_y);
            Texture* tile_tex = nullptr;
            if (tile_id == 1)
                tile_tex = &game->GameResources.Textures["bouncy_wall"];
            if (tile_id == 2)
                tile_tex = &game->GameResources.Textures["delete_wall"];
            if (tile_id == 12)
                ForceFieldPos.push_back(Vector2{(float)curr_tile_x, (float)curr_tile_y});

            if (tile_tex == nullptr)
                continue;

            if (tile_id < 3)
                DrawWallTile(curr_tile_x,curr_tile_y,tile_tex);
        }
    }

    RenderForceFields(ForceFieldPos);
}

void TileManager::RenderForceFields(std::vector<Vector2> ForceFieldPos)
{
    if (ForceFieldPos.size() == 0)
        return;

    bool found = false;

    for (Vector2 vec : ForceFieldPos)
    {
        Vector2 p1 = vec;
        Vector2 p2 = {vec.x + TileSize, vec.y};
        Vector2 p3 = {vec.x + TileSize, vec.y + TileSize};
        Vector2 p4 = {vec.x, vec.y + TileSize};

        if (p1.x >= 0 && p1.x < GetRenderWidth() && p1.y >= 0 && p1.y < GetRenderHeight())
        {
            found = true;
            break;
        }

        if (p2.x >= 0 && p2.x < GetRenderWidth() && p2.y >= 0 && p2.y < GetRenderHeight())
        {
            found = true;
            break;
        }

        if (p3.x >= 0 && p3.x < GetRenderWidth() && p3.y >= 0 && p3.y < GetRenderHeight())
        {
            found = true;
            break;
        }

        if (p4.x >= 0 && p4.x < GetRenderWidth() && p4.y >= 0 && p4.y < GetRenderHeight())
        {
            found = true;
            break;
        }
    }

    if (!found)
        return;

    game->GameCamera.EndRenderTexture();
    game->GameCamera.BeginRenderTexture(ForceFieldTex,true);

    ClearBackground(BLANK);

    for (Vector2 p : ForceFieldPos)
    {
        Rectangle rec = {p.x*TileSize, p.y*TileSize, TileSize, TileSize};
        DrawTexturePro(game->GameResources.Textures["noise"], {(p.x * TileSize / 2) + (float)(game->GetGameTime()),0,rec.width, rec.height},rec,{0,0},0,WHITE);
        DrawRectangleLinesEx(rec, 4.0f + sin(game->GetGameTime()) * 2.0f, ColorLerp(GREEN, DARKGREEN, cos(game->GetGameTime())/3.0f));
    }

    game->GameCamera.EndRenderTexture();
    game->GameCamera.BeginRenderTexture(TileMapTex);
    DrawTextureRec(ForceFieldTex.texture,{0,0,(float)ForceFieldTex.texture.width,-(float)ForceFieldTex.texture.height},{0,0},WHITE);
}

void TileManager::ProcessUniformLocations()
{
    if (DistortionUniformLocations.size() <= 0)
    {
        BeginShaderMode(game->GameResources.Shaders["bounce_distort"]);

        for (int i = 0; i < 100; i++)
        {

            int loc1 = GetShaderLocation(game->GameResources.Shaders["bounce_distort"], ("distortions[" + to_string(i) + "].position").c_str());
            int loc2 = GetShaderLocation(game->GameResources.Shaders["bounce_distort"], ("distortions[" + to_string(i) + "].strength").c_str());
            int loc3 = GetShaderLocation(game->GameResources.Shaders["bounce_distort"], ("distortions[" + to_string(i) + "].radius").c_str());
            std::tuple locs(loc1, loc2, loc3);

            DistortionUniformLocations.push_back(locs);
        }

        DistortionCountLocation = GetShaderLocation(game->GameResources.Shaders["bounce_distort"], "distortionCount");

        EndShaderMode();
    }
}

void TileManager::Update() {

    if (TileMapTex.texture.width != GetRenderWidth() || TileMapTex.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(TileMapTex);
        TileMapTex = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    UpdateDistance = Vector2{round(GetRenderWidth() / game->GameCamera.RaylibCamera.zoom / TileSize) + 3, round(GetRenderHeight() / game->GameCamera.RaylibCamera.zoom / TileSize) + 3};

    ProcessUniformLocations();
    ProcessDistortions();

    game->GameCamera.BeginRenderTexture(TileMapTex, true);
    BeginBlendMode(BLEND_ALPHA);
    ClearBackground(BLANK);

    DrawTileMap();

    EndBlendMode();
    game->GameCamera.EndRenderTexture();

    game->GameCamera.StopCamera();

    BeginShaderMode(game->GameResources.Shaders["bounce_distort"]);

    int DistortionCount = (int)min(100.0f, (float)Distortions.size());

    SetShaderValue(game->GameResources.Shaders["bounce_distort"], DistortionCountLocation, &DistortionCount, SHADER_UNIFORM_INT);

    BeginBlendMode(BLEND_ALPHA);
    DrawTexturePro(TileMapTex.texture, {0, 0, (float)TileMapTex.texture.width, (float)-TileMapTex.texture.height}, {
        0,0, (float)TileMapTex.texture.width,(float)TileMapTex.texture.height
    }, {0,0}, 0, WHITE);
    EndBlendMode();
    EndShaderMode();

    game->GameCamera.BeginCamera();
}

void TileManager::DistortArea(Distortion DistortionForArea)
{
    if (Distortions.size() >= 100)
        return;
    for (Distortion &otherDis : Distortions)
    {
        if (Vector2Distance(otherDis.Position, DistortionForArea.Position) < 25 && game->GetGameTime() - otherDis.SpawnTime <= FXLifetime/2)
            return;
    }
    DistortionForArea.SpawnTime = game->GetGameTime();
    Distortions.push_back(DistortionForArea);
}

void TileManager::ProcessDistortions()
{
    std::erase_if(Distortions, [this](Distortion &d)
    {
        return game->GetGameTime() - d.SpawnTime >= FXLifetime;
    });

    for (Distortion &d : Distortions)
    {
        if (game->GetGameTime() - d.SpawnTime < FXLifetime/2)
            d.Strength = (game->GetGameTime() - d.SpawnTime) / (FXLifetime/2);
        else if (game->GetGameTime() - d.SpawnTime >= 0.125)
            d.Strength = FXLifetime - (game->GetGameTime() - d.SpawnTime - (FXLifetime/2)) / (FXLifetime/2);

        d.Strength = clamp((float)pow(d.Strength, 2) * 3.1f, 0.0f, 3.1f);
    }

    int DistortionCount = (int)min(100.0f, (float)Distortions.size());
    for (int i = 0; i < DistortionCount; i++)
    {
        int PositionLocation = get<0>(DistortionUniformLocations[i]);
        int StrengthLocation = get<1>(DistortionUniformLocations[i]);
        int RadiusLocation = get<2>(DistortionUniformLocations[i]);

        Vector2 SPosition = GetWorldToScreen2D(Distortions[i].Position, game->GameCamera.RaylibCamera);
        if (game->DebugDraw)
            DrawCircle(Distortions[i].Position.x, Distortions[i].Position.y, 5, ColorAlpha(PINK, 0.5f));

        SetShaderValue(game->GameResources.Shaders["bounce_distort"], PositionLocation, &SPosition, SHADER_UNIFORM_VEC2);
        SetShaderValue(game->GameResources.Shaders["bounce_distort"], StrengthLocation, &Distortions[i].Strength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->GameResources.Shaders["bounce_distort"], RadiusLocation, &Distortions[i].Radius, SHADER_UNIFORM_FLOAT);
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
    float Multiplier = 1.0f + 0.15f * (game->LevelData[game->CurrentLevelName]["difficulty"].get<int>()-3);
    Armor *= Multiplier;
    Speed *= Multiplier;
    Health *= Multiplier;
    game->GameEntities.AddEntity(EnemyType, make_shared<Enemy>(bbox_x, bbox_y, Health, Speed, Armor, Weapon, game->GameResources.Textures["enemy"], *game));
}

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
            cout << e.what() << endl;
        }
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
            game->GameEntities.AddEntity(SpawnerType, spawner);
            break;
    }
    case BossTileType: {
            BossSpawnPosition = Vector2{bbox_x, bbox_y};
            break;
    };
    case UpgradeStationTileType: {
            std::shared_ptr<UpgradeStation> station = std::make_shared<UpgradeStation>(*game, bbox_x, bbox_y);
            game->GameEntities.AddEntity(UpgradeStationType, station);
            break;
    }
    case TurretTileType: {
            std::shared_ptr<Turret> t = std::make_shared<Turret>(*game,
                "Sniper", bbox_x, bbox_y);
            game->GameEntities.AddEntity(TurretType, t);
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
        NewEntity = make_shared<Enemy>(NewFileEntity.X, NewFileEntity.Y, NewFileEntity.Health, NewFileEntity.Speed, NewFileEntity.Armor, NewFileEntity.Weapon, game->GameResources.Textures["enemy"], *game);
    }

    NewEntity->BoundingBox.width = NewFileEntity.W;
    NewEntity->BoundingBox.height = NewFileEntity.H;

    game->GameEntities.AddEntity(NewEntity->Type, NewEntity);
}

void TileManager::SetPropertiesOfFileEntity(FileEntity& ThisFileEntity, int i, std::string cell)
{
    if (cell.starts_with(" "))
        cell = cell.substr(1, cell.size() - 1);
    switch (i)
    {
    case 0:
        ThisFileEntity.Type = (EntityType)std::stoi(cell);
        break;
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
}

void TileManager::ReadEntitiesFile(std::string FileName)
{
    std::ifstream  data(FileName);

    std::string line;
    while(std::getline(data,line))
    {
        if (line == "Type,X,Y,Width,Height,Health,Armor,Speed,Weapon")
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

void TileManager::Clear()
{
    Map.clear();
    Distortions.clear();
    MapWidth = 0;
    MapHeight = 0;
    PlayerSpawnPosition = {0, 0};
    BossSpawnPosition = {0,0};
    EnemySpawnLocations = std::vector<Vector2>();
    FXLifetime = 0.75f;
    TileSize = 72;
    UpdateDistance = Vector2{round(game->GameCamera.IntendedScreenWidth / TileSize) + 1, round(game->GameCamera.IntendedScreenHeight / TileSize) + 1};
    if (IsRenderTextureValid(TileMapTex))
        UnloadRenderTexture(TileMapTex);
    TileMapTex = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    if (IsRenderTextureValid(ForceFieldTex))
        UnloadRenderTexture(ForceFieldTex);
    ForceFieldTex = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
}

void TileManager::Quit() {
    Clear();
    if (IsRenderTextureValid(TileMapTex))
        UnloadRenderTexture(TileMapTex);
    if (IsRenderTextureValid(ForceFieldTex))
        UnloadRenderTexture(ForceFieldTex);
}
