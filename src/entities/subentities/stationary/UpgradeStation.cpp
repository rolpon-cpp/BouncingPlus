#include "UpgradeStation.h"
#include <raymath.h>
#include "../../../game/Game.h"
#include "../../../game/managers/CameraManager.h"
#include "../../../game/managers/SoundManager.h"
#include "../../../game/managers/ResourceManager.h"
#include "../../../game/managers/EntityManager.h"
#include "../player/Player.h"
#include <nlohmann/json.hpp>

UpgradeStation::UpgradeStation()
{
}

UpgradeStation::UpgradeStation(Game& game, float bbox_x, float bbox_y) : Entity(
    game.GameResources->Textures["upgrade_core"], {bbox_x - 49.0f, bbox_y - 49.0f, 98, 98}, 0, game)
{
    this->Type = UpgradeStationType;
}

void UpgradeStation::Render()
{
    float center_x = BoundingBox.x + BoundingBox.width / 2;
    float center_y = BoundingBox.y + BoundingBox.height / 2;

    Vector2 mov = Vector2Normalize(Vector2{1, 1});
    float dist = 58;
    float div = 15.0f;

    Color my_color1 = ColorLerp(GRAY, DARKGRAY, 0.35f + 0.5f * sin(game->GetGameTime() * 2.9f + 184.85f));
    Color my_color2 = ColorLerp(GRAY, BLACK, 0.15f + 0.5f * sin(game->GetGameTime() * 2.2f + 6.29f));
    Color my_color3 = ColorLerp(GRAY, WHITE, 0.75f + 0.5f * sin(game->GetGameTime() * 1.5f - 8.29f));
    Color my_color4 = ColorLerp(GRAY, LIGHTGRAY, 0.75f + 0.5f * sin(game->GetGameTime() * 3.3f - 5.38f));

    float rand_mov_x_1 = sin(game->GetGameTime() * 1.35f) * (dist / div);
    float rand_mov_y_1 = cos(game->GetGameTime() * 1.35f) * (dist / div);

    float rand_mov_x_2 = sin(game->GetGameTime() * 1.35f + 483.38f) * (dist / div);
    float rand_mov_y_2 = cos(game->GetGameTime() * 1.35f + 483.38f) * (dist / div);

    float rand_mov_x_3 = sin(game->GetGameTime() * 1.35f - 109.38f) * (dist / div);
    float rand_mov_y_3 = cos(game->GetGameTime() * 1.35f - 109.38f) * (dist / div);

    float rand_mov_x_4 = sin(game->GetGameTime() * 1.35f + 584.33f) * (dist / div);
    float rand_mov_y_4 = cos(game->GetGameTime() * 1.35f + 584.33f) * (dist / div);

    DrawTexturePro(game->GameResources->Textures["upgrade_side_blade"], {0, 0, 49, 49}, {
                       center_x - mov.x * dist + rand_mov_x_1, center_y + mov.y * dist + rand_mov_y_1, 68.6f, 68.6f
                   }, {34.3f, 34.3f}, 0, my_color1);
    DrawTexturePro(game->GameResources->Textures["upgrade_side_blade"], {0, 0, 49, 49}, {
                       center_x + mov.x * dist + rand_mov_x_2, center_y + mov.y * dist + rand_mov_y_2, 68.6f, 68.6f
                   }, {34.3f, 34.3f}, -90, my_color2);

    DrawTexturePro(game->GameResources->Textures["upgrade_side_blade"], {0, 0, 49, 49}, {
                       center_x - mov.x * dist + rand_mov_x_3, center_y - mov.y * dist + rand_mov_y_3, 68.6f, 68.6f
                   }, {34.3f, 34.3f}, 90, my_color3);
    DrawTexturePro(game->GameResources->Textures["upgrade_side_blade"], {0, 0, 49, 49}, {
                       center_x + mov.x * dist + rand_mov_x_4, center_y - mov.y * dist + rand_mov_y_4, 68.6f, 68.6f
                   }, {34.3f, 34.3f}, 180, my_color4);
}

void UpgradeStation::PhysicsUpdate(float dt, double time)
{
    Entity::PhysicsUpdate(dt, time);
}

UpgradeStation::~UpgradeStation()
{
}

void UpgradeStation::Update()
{
    Entity::Update();
    Render();
}