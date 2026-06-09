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
    Type = DEFAULT;
}

Effect::Effect(double ImpactTime)
{
    this->Duration = 0.0f;
    this->ImpactTime = ImpactTime;
    Type = DEFAULT;
}

void Effect::Update(std::shared_ptr<Entity> ImpactedEntity)
{
}

Burning::Burning(float Damage, double Duration, double ImpactTime) : Effect(Duration, ImpactTime)
{
    this->Damage = Damage;
    LastDidFireParticle = 0.0f;
    OwnerReward = 0;
    Type = BURNING;
    SmokeParticleCount = 0;
    GradientProg = 0;
    LastDidSFX= 0.0f;
}

Burning::Burning(double ImpactTime) : Effect(10.0f, ImpactTime)
{
    Damage = 15.0f;
    LastDidFireParticle = 0.0f;
    OwnerReward = 0;
    SmokeParticleCount = 0;
    Type = BURNING;
    GradientProg = 0;
    LastDidSFX = 0.0f;
}

void Burning::SetOwner(std::shared_ptr<Entity> Owner)
{
    if (Owner != nullptr)
    {
        this->Owner = Owner;
        OG_Color = Owner->EntityColor;
    }
}

void Burning::SetOwnerReward(double Reward)
{
    this->OwnerReward = Reward;
}

void Burning::Update(std::shared_ptr<Entity> ImpactedEntity)
{
    Effect::Update(ImpactedEntity);
    Game *game = ImpactedEntity->game;

    if (auto owner = Owner.lock())
    {
        if (owner != ImpactedEntity)
        {
            owner->DamageOther(ImpactedEntity, Damage * game->GetGameDeltaTime(), owner, OwnerReward);
            if (ImpactedEntity->Health <= 0.0f)
            {
                if (owner->Type == PlayerType)
                    owner->game->MainPlayer->LogicProcessor.IncreaseScore("Enemy Burn Kill", 50.0f, ORANGE);
            }
        }
    } else
    {
        ImpactedEntity->Health -= Damage * game->GetGameDeltaTime();
    }

    float Percent = (game->GetGameTime() - ImpactTime) / Duration;

    if (Percent >= 0.95f)
        ImpactedEntity->EntityColor = OG_Color;
    else
        ImpactedEntity->EntityColor = ColorBrightness(OG_Color, -min(Percent, 0.8f));

    if (Percent <= 0.8f)
        GradientProg = lerp(GradientProg, 1.0f, 3.0f * game->GetGameDeltaTime());
    else
        GradientProg = lerp(GradientProg, 0.0f, 1.75f * game->GetGameDeltaTime());

    float height = ImpactedEntity->BoundingBox.height * GradientProg;
    DrawRectangleGradientV(ImpactedEntity->BoundingBox.x, ImpactedEntity->BoundingBox.y, ImpactedEntity->BoundingBox.width, height, ColorAlpha(RED, 0.5f), BLANK);
    Rectangle r = {0,0,ImpactedEntity->BoundingBox.width + 1.25f, ImpactedEntity->BoundingBox.height + 1.25f};
    r.x = ImpactedEntity->GetCenter().x - r.width/2.0f;
    r.y = ImpactedEntity->GetCenter().y - r.height/2.0f;
    float P = 2.0f * (Percent > 0.5 ? 1.0f - Percent : Percent);
    DrawRectangleRoundedLinesEx(r, 0.1f, 10, 5.0f * max(P, 0.75f), ColorAlpha(RED, P/2.0f));

    if (game->GetGameTime() - LastDidSFX >= 0.95f)
    {
        float Distance = Vector2Distance(ImpactedEntity->GetCenter(), game->MainPlayer->GetCenter());

        float DistanceMultiplier = (1000.0f - Distance) / 1000.0f;
        DistanceMultiplier += GetRandomValue(-20, 20) / 100.0f;

        game->GameSounds.PlayGameSound((string("burn") + to_string(GetRandomValue(1,2))),
            0.5f * DistanceMultiplier, 1.0f - GetRandomValue(-50,50)/100.0f);
        LastDidSFX = game->GetGameTime();
    }
    if (game->GetGameTime() - LastDidFireParticle >= (Percent > 0.5f ? 0.1f : 0.2f))
    {
        game->GameParticles.ParticleEffect({
            ImpactedEntity->GetCenter(),
            70.0f,
            ColorLerp(RED, ORANGE, 0.5f),
            5.0f,
            6.0f,
            1.25f,
            ColorLerp(RED, ORANGE, GetRandomValue(1, 100) / 100.0f)
        }, -90, 35, 3);
        LastDidFireParticle = game->GetGameTime();

        SmokeParticleCount++;
        if (SmokeParticleCount > 3)
        {
            game->GameParticles.ParticleEffect({
            ImpactedEntity->GetCenter(),
            200.0f,
            ColorAlpha(GRAY, 0.5f),
            0.0f,
            36.0f,
            5.0f,
            ColorAlpha(BLACK, 0.5f)
        }, -90, 35, 3);
            SmokeParticleCount = 0;
        }
    }
}

Swiftness::Swiftness(double ImpactTime) : Effect(5.0f, ImpactTime)
{
    this->LastDidParticle = 0.0f;
    Type = SWIFTNESS;
    this->SpeedInc = 500.0f;
}

Swiftness::Swiftness(float SpeedInc, double Duration, double ImpactTime) : Effect(Duration, ImpactTime)
{
    this->SpeedInc = SpeedInc;
    Type = SWIFTNESS;
    this->LastDidParticle = 0.0f;
}

void Swiftness::Update(std::shared_ptr<Entity> ImpactedEntity)
{
    Effect::Update(ImpactedEntity);
    Game *game = ImpactedEntity->game;

    double EffectPercentage = (game->GetGameTime() - ImpactTime) / Duration;
    float Multiplier = abs(EffectPercentage - 0.5f) / 0.5f;
    Multiplier = 1.0f - Multiplier;
    Multiplier *= 1.5f;

    Multiplier = max(Multiplier, 0.5f);

    game->GameCamera.QuickZoom(0.85f, 0.5f);
    if (Vector2Distance(ImpactedEntity->GetCenter(), LastPos) > 0 && game->GetGameTime() - LastDidParticle >= 0.1f)
    {
        game->GameParticles.ParticleEffect({
            ImpactedEntity->GetCenter(),
            ImpactedEntity->GetSpeed() * 1.5f,
            ColorLerp(ColorLerp(WHITE,SKYBLUE,0.65f), ColorBrightness(SKYBLUE, 0.5f), 0.5f),
            ImpactedEntity->GetSpeed() * 2.5f,
            16.0f,
            0.65f,
            ColorLerp(ColorLerp(WHITE,SKYBLUE,0.65f), SKYBLUE, GetRandomValue(1, 100) / 100.0f)
        }, 180.0f - (Vector2LineAngle(ImpactedEntity->GetCenter(),LastPos) * RAD2DEG), 35, GetRandomValue(1,2));
        LastDidParticle = game->GetGameTime();
    }

    ImpactedEntity->FrameStackSpeed = SpeedInc * Multiplier;
    LastPos = ImpactedEntity->GetCenter();
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

void Effects::AddEffect(EffectData data)
{
    switch (data.Type)
    {
    case DEFAULT:
        break;
    case SWIFTNESS:
        {
            Swiftness* s = new Swiftness(data.Power, data.Duration, game->GetGameTime());
            AddEffect(s);
            break;
        }

    case BURNING:
        {
            Burning* s = new Burning(data.Power, data.Duration, game->GetGameTime());
            if (auto owner = data.Owner.lock())
                s->SetOwner(owner);
            s->SetOwnerReward(data.Reward);
            AddEffect(s);
            break;
        }
    }
}

void Effects::AddEffect(Effect* effect)
{
    if (effect == nullptr)
        return;
    std::erase_if(CurrentEffects, [this, effect](Effect* e)
    {
        bool sd = false;

        if (e->Type == effect->Type)
            sd = true;

        if (sd)
            delete e;
        return sd;
    });
    effect->ImpactTime = game->GetGameTime();
    CurrentEffects.push_back(effect);
}

bool Effects::HasEffect(EffectType Type)
{
    for (Effect* effect : CurrentEffects)
    {
        if (effect->Type == Type)
        {
            return true;
        }
    }
    return false;
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
