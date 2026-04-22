//
// Created by lalit on 8/27/2025.
//

#ifndef BOUNCINGPLUS_TILEMANAGER_H
#define BOUNCINGPLUS_TILEMANAGER_H

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "TileType.h"

class Game;

struct Distortion
{
    Vector2 Position;
    float Strength;
    float Radius;
    double SpawnTime;
};

class TileManager {
    Game* game;
    Vector2 UpdateDistance;

    std::vector<std::tuple<int, int, int>> DistortionUniformLocations;
    std::vector<Distortion> Distortions;

    int uWidth, uHeight, uTime;

    int DistortionCountLocation;
    float FXLifetime;

    void DrawTileMap();
    void RenderForceFields(std::vector<Vector2> ForceFieldPos);
    void ProcessTile(std::string cell, int x, int y, bool* PlayerSpawnFound);
    void ProcessDistortions();
    void DrawWallTile(int curr_tile_x, int curr_tile_y, Texture* tile_tex);
    void AddEnemy(float bbox_x, float bbox_y, int tile_id);
    void ProcessUniformLocations();

    std::vector<std::string> Lines;
    std::string PrevFileName;

    public:
        float TileSize;
        int MapWidth, MapHeight;
        Vector2 PlayerSpawnPosition;
        Vector2 BossSpawnPosition;
        TileType TileTypes[13];
        std::vector<Vector2> EnemySpawnLocations;
        std::vector<int> Map;
        RenderTexture TileMapTex;
        RenderTexture ForceFieldTex;
        TileManager();
        TileManager(Game &game);
        int GetTileAt(int x, int y);
        int GetTileAt(Vector2 coord);
        void SetTileAt(int x, int y, int id);
        void SetTileAt(Vector2 coord, int id);
        void Update();
        void DistortArea(Distortion DistortionForArea);
        void ReadMapDataFile(std::string FileName);
        void Clear();
        void Quit();
};


#endif //BOUNCINGPLUS_TILEMANAGER_H