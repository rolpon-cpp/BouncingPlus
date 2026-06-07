//
// Created by lalit on 4/20/2026.
//

#include "Effects.h"

#include <iostream>

#include "../../game/Game.h"

#include "raymath.h"

Effect::Effect(double Duration, double ImpactTime)
{
    this->Duration = Duration;
    this->ImpactTime = ImpactTime;
}

Effect::Effect(double ImpactTime)
{
    this->Duration = 0.0f;
    this->ImpactTime = ImpactTime;
}

void Effect::Update(std::shared_ptr<Entity> Owner)
{
}

Burning::Burning(float Damage, double Duration, double ImpactTime) : Effect(Duration, ImpactTime)
{
    this->Damage = Damage;
    LastDidFireParticle = 0.0f;
}

Burning::Burning(double ImpactTime) : Effect(5.0f, ImpactTime)
{
    Damage = 5.0f;
    LastDidFireParticle = 0.0f;
}

void Burning::Update(std::shared_ptr<Entity> Owner)
{
    Effect::Update(Owner);
    Game *game = Owner->game;
    Owner->Health -= Damage * game->GetGameDeltaTime();
    if (game->GetGameTime() - LastDidFireParticle >= 0.025f)
    {
        game->GameCamera.QuickZoom(1.15f, 0.5f);
        game->GameParticles.ParticleEffect({
            Owner->GetCenter(),
            65.0f,
            ColorLerp(RED, ORANGE, 0.5f),
            5.0f,
            3.0f,
            1.25f,
            ColorLerp(RED, ORANGE, GetRandomValue(1, 100) / 100.0f)
        }, -90, 35, 3);
        LastDidFireParticle = game->GetGameTime();
    }
}

Swiftness::Swiftness(double ImpactTime) : Effect(5.0f, ImpactTime)
{
    this->LastDidParticle = 0.0f;
    this->SpeedInc = 500.0f;
}

Swiftness::Swiftness(float SpeedInc, double Duration, double ImpactTime) : Effect(Duration, ImpactTime)
{
    this->SpeedInc = SpeedInc;
    this->LastDidParticle = 0.0f;
}

void Swiftness::Update(std::shared_ptr<Entity> Owner)
{
    Effect::Update(Owner);
    Game *game = Owner->game;

    double EffectPercentage = (game->GetGameTime() - ImpactTime) / Duration;
    float Multiplier = abs(EffectPercentage - 0.5f) / 0.5f;
    Multiplier = 1.0f - Multiplier;
    Multiplier *= 1.5f;

    Multiplier = max(Multiplier, 0.5f);

    game->GameCamera.QuickZoom(0.85f, 0.5f);
    if (Vector2Distance(Owner->GetCenter(), LastPos) > 0 && game->GetGameTime() - LastDidParticle >= 0.1f)
    {
        game->GameParticles.ParticleEffect({
            Owner->GetCenter(),
            Owner->GetSpeed() * 1.5f,
            ColorLerp(ColorLerp(WHITE,SKYBLUE,0.65f), ColorBrightness(SKYBLUE, 0.5f), 0.5f),
            Owner->GetSpeed() * 2.5f,
            16.0f,
            0.65f,
            ColorLerp(ColorLerp(WHITE,SKYBLUE,0.65f), SKYBLUE, GetRandomValue(1, 100) / 100.0f)
        }, 180.0f - (Vector2LineAngle(Owner->GetCenter(),LastPos) * RAD2DEG), 35, GetRandomValue(1,2));
        LastDidParticle = game->GetGameTime();
    }

    Owner->FrameStackSpeed = SpeedInc * Multiplier;
    LastPos = Owner->GetCenter();
}

Swiftness::~Swiftness()
{
}

Effects::Effects(std::shared_ptr<Entity> Owner, Game& game)
{
    this->Owner = Owner;
    this->game = &game;
}

Effects::Effects()
{
}

Effect::~Effect()
{
}

Burning::~Burning()
{
}

void Effects::AddEffect(Effect* effect)
{
    if (effect == nullptr)
        return;
    effect->ImpactTime = game->GetGameTime();
    CurrentEffects.push_back(effect);
}

void Effects::RemoveOldestEffect()
{
    double EffectTime = std::numeric_limits<double>::max();
    int theIdx = -1;
    int i = 0;
    for (Effect* effect : CurrentEffects)
    {
        if (effect->ImpactTime < EffectTime)
        {
            EffectTime = effect->ImpactTime;
            theIdx = i;
        }
        i++;
    }

    if (theIdx != -1)
    {
        delete CurrentEffects[theIdx];
        CurrentEffects.erase(CurrentEffects.begin() + theIdx);
    }
}

void Effects::Update()
{
    if (Owner.lock() == nullptr)
        return;
    std::erase_if(CurrentEffects, [this](Effect* e)
    {
        bool ShouldRemove = game->GetGameTime() - e->ImpactTime >= e->Duration;
        if (ShouldRemove)
            delete e;
        return ShouldRemove;
    });

    while (CurrentEffects.size() > 10)
        RemoveOldestEffect();

    for (Effect* effect : CurrentEffects)
        effect->Update(Owner.lock());
}

void Effects::Cleanup()
{
    for (Effect* effect : CurrentEffects)
        delete effect;
    CurrentEffects.clear();
}
