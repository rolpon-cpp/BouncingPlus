//
// Created by lalit on 1/23/2026.
//

#ifndef BOUNCINGPLUS_POWERUPS_H
#define BOUNCINGPLUS_POWERUPS_H
#include <memory>
#include <string>

class Player;
class Game;
class Bullet;

struct Powerup
{
    float Cooldown = 0;
    float Length = 0;
    std::string Name = "";
    virtual void Complete(std::shared_ptr<Player> Owner);
    virtual void Undo(std::shared_ptr<Player> Owner);
};

struct SpeedPowerup : Powerup
{
    SpeedPowerup();
    void Complete(std::shared_ptr<Player> Owner) override;
    void Undo(std::shared_ptr<Player> Owner) override;
};

struct FreezePowerup : Powerup
{
    FreezePowerup();
    void Complete(std::shared_ptr<Player> Owner) override;
    void Undo(std::shared_ptr<Player> Owner) override;
};

struct ShieldPowerup : Powerup
{
    float TransBuff = 0;
    float DefaultTrans = 0.1f;
    float LerpSpeed = 2.0f;
    float FieldSize = 250.0f;
    float displayFieldSize = 0.0f;
    int ShieldThickness = 7;
    ShieldPowerup();
    void Complete(std::shared_ptr<Player> Owner) override;
    void Undo(std::shared_ptr<Player> Owner) override;
};

struct TankyPowerup : Powerup
{
    TankyPowerup();
    void Complete(std::shared_ptr<Player> Owner) override;
    void Undo(std::shared_ptr<Player> Owner) override;
};

class PowerupSystem
{
    public:
    std::weak_ptr<Player> Owner;
    Game* game;
    float CurrentCooldown = 0;
    float CurrentLength = 0;
    bool PowerupIsActive = false;
    Powerup *CurrentPowerup = nullptr;
    PowerupSystem(std::shared_ptr<Player> Owner, Game &game);
    PowerupSystem();
    void Update();
    void Activate();
    void SetPowerup(Powerup *Powerup);
    virtual ~PowerupSystem();
};


#endif //BOUNCINGPLUS_POWERUPS_H