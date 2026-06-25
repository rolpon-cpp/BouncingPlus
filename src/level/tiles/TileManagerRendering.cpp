//
// Created by Rolpon on 5/27/2026.
//

#include "TileManager.h"
#include <raymath.h>
#include "../../game/managers/CameraManager.h"
#include "../../game/managers/SoundManager.h"
#include "../../game/managers/ResourceManager.h"
#include <nlohmann/json.hpp>
#include "../../game/Game.h"

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
    std::vector<Vector2> ForceFieldPos;

    Vector2 *CameraPosition = &this->game->GameCamera->CameraPosition;
    int tile_x = static_cast<int> ((CameraPosition->x + GetRenderWidth()/2) / TileSize);
    int tile_y = static_cast<int> ((CameraPosition->y + GetRenderHeight()/2) / TileSize);
    for (int y = 0; y < UpdateDistance.y; y++) {
        for (int x = 0; x < UpdateDistance.x; x++) {
            int curr_tile_x = tile_x + x - static_cast<int> (UpdateDistance.x / 2);
            int curr_tile_y = tile_y + y - static_cast<int> (UpdateDistance.y / 2);
            int tile_id = GetTileAt(curr_tile_x,curr_tile_y);
            Texture* tile_tex = nullptr;
            if (tile_id == 1)
                tile_tex = &game->GameResources->Textures["bouncy_wall"];
            if (tile_id == 2)
                tile_tex = &game->GameResources->Textures["delete_wall"];
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

    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    for (Vector2 p : ForceFieldPos)
    {
        Rectangle rec = {p.x*TileSize, p.y*TileSize, TileSize, TileSize};
        DrawTexturePro(game->GameResources->Textures["noise"], {(p.x * TileSize / 2) + (float)(game->GetGameTime()),0,rec.width, rec.height},rec,{0,0},0,WHITE);
        DrawRectangleLinesEx(rec, 4.0f + sin(game->GetGameTime()) * 2.0f, ColorLerp(GREEN, DARKGREEN, cos(game->GetGameTime())/3.0f));
    }
    EndBlendMode();

}

void TileManager::ProcessUniformLocations()
{
    if (DistortionUniformLocations.size() <= 0)
    {
        BeginShaderMode(game->GameResources->Shaders["bounce_distort"]);

        for (int i = 0; i < 100; i++)
        {

            int loc1 = GetShaderLocation(game->GameResources->Shaders["bounce_distort"], ("distortions[" + std::to_string(i) + "].position").c_str());
            int loc2 = GetShaderLocation(game->GameResources->Shaders["bounce_distort"], ("distortions[" + std::to_string(i) + "].strength").c_str());
            int loc3 = GetShaderLocation(game->GameResources->Shaders["bounce_distort"], ("distortions[" + std::to_string(i) + "].radius").c_str());
            std::tuple locs(loc1, loc2, loc3);

            DistortionUniformLocations.push_back(locs);
        }

        DistortionCountLocation = GetShaderLocation(game->GameResources->Shaders["bounce_distort"], "distortionCount");

        EndShaderMode();
    }
}

void TileManager::Update() {

    UpdateDistance = Vector2{round(game->GameCamera->IntendedScreenWidth / TileSize) + 1, round(game->GameCamera->IntendedScreenHeight / TileSize) + 1};

    if (TileMapTex.texture.width != GetRenderWidth() || TileMapTex.texture.height != GetRenderHeight()) {
        UnloadRenderTexture(TileMapTex);
        TileMapTex = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    }

    UpdateDistance = Vector2{round(GetRenderWidth() / game->GameCamera->RaylibCamera.zoom / TileSize) + 3, round(GetRenderHeight() / game->GameCamera->RaylibCamera.zoom / TileSize) + 3};

    ProcessUniformLocations();
    ProcessDistortions();

    game->GameCamera->BeginRenderTexture(TileMapTex, true);
    BeginBlendMode(BLEND_ALPHA);
    ClearBackground(BLANK);

    DrawTileMap();

    EndBlendMode();
    game->GameCamera->EndRenderTexture();

    game->GameCamera->StopCamera();

    BeginShaderMode(game->GameResources->Shaders["bounce_distort"]);

    int DistortionCount = (int)std::min(100.0f, (float)Distortions.size());

    SetShaderValue(game->GameResources->Shaders["bounce_distort"], DistortionCountLocation, &DistortionCount, SHADER_UNIFORM_INT);

    BeginBlendMode(BLEND_ALPHA);
    DrawTexturePro(TileMapTex.texture, {0, 0, (float)TileMapTex.texture.width, (float)-TileMapTex.texture.height}, {
        0,0, (float)TileMapTex.texture.width,(float)TileMapTex.texture.height
    }, {0,0}, 0, WHITE);
    EndBlendMode();
    EndShaderMode();

    game->GameCamera->BeginCamera();
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

        d.Strength = std::ranges::clamp((float)pow(d.Strength, 2) * 3.1f, 0.0f, 3.1f);
    }

    int DistortionCount = (int)std::min(100.0f, (float)Distortions.size());
    for (int i = 0; i < DistortionCount; i++)
    {
        int PositionLocation = get<0>(DistortionUniformLocations[i]);
        int StrengthLocation = get<1>(DistortionUniformLocations[i]);
        int RadiusLocation = get<2>(DistortionUniformLocations[i]);

        Vector2 SPosition = GetWorldToScreen2D(Distortions[i].Position, game->GameCamera->RaylibCamera);
        if (game->DebugDraw)
            DrawCircle(Distortions[i].Position.x, Distortions[i].Position.y, 5, ColorAlpha(PINK, 0.5f));

        SetShaderValue(game->GameResources->Shaders["bounce_distort"], PositionLocation, &SPosition, SHADER_UNIFORM_VEC2);
        SetShaderValue(game->GameResources->Shaders["bounce_distort"], StrengthLocation, &Distortions[i].Strength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->GameResources->Shaders["bounce_distort"], RadiusLocation, &Distortions[i].Radius, SHADER_UNIFORM_FLOAT);
    }
}