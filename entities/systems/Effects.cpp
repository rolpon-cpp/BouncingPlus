//
// Created by lalit on 4/20/2026.
//

#include "Effects.h"

Effects::Effects(std::shared_ptr<Entity> Owner, Game& game)
{
    this->Owner = Owner;
    this->game = &game;
}

Effects::Effects()
{
}

void Effects::Update()
{
}

Effects::~Effects()
{
}
