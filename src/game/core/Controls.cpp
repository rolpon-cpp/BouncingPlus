//
// Created by lalit on 3/29/2026.
//

#include "Controls.h"
#include <raylib.h>

void Controls::SetDefaultBindings()
{
    Bindings["up"] = KEY_W;
    Bindings["down"] = KEY_S;
    Bindings["left"] = KEY_A;
    Bindings["right"] = KEY_D;
    Bindings["pause"] = KEY_ESCAPE;
    Bindings["debug"] = KEY_X;
    Bindings["debug2"] = KEY_C;
    Bindings["debug3"] = KEY_B;
    Bindings["debug4"] = KEY_N;
    Bindings["fullscreen"] = KEY_F11;
    Bindings["level_restart_or_finish"] = KEY_E;
    Bindings["attack_other"] = KEY_SPACE;
    Bindings["reload"] = KEY_R;
    Bindings["powerup"] = KEY_F;
    Bindings["drop"] = KEY_Q;
    Bindings["item1"] = KEY_ONE;
    Bindings["item2"] = KEY_TWO;
    Bindings["item3"] = KEY_THREE;
    Bindings["dash"] = KEY_LEFT_SHIFT;
}

Controls::Controls()
{
}

Controls::~Controls()
{
}

bool Controls::ControlsPrecheck(std::string ControlName)
{
    if (!Bindings.count(ControlName))
        return true;
    return false;
}

bool Controls::IsControlDown(std::string ControlName)
{
    if (ControlsPrecheck(ControlName))
        return false;
    return IsKeyDown(Bindings[ControlName]);
}

bool Controls::IsControlUp(std::string ControlName)
{
    if (ControlsPrecheck(ControlName))
        return false;
    return IsKeyUp(Bindings[ControlName]);
}

bool Controls::IsControlPressed(std::string ControlName)
{
    if (ControlsPrecheck(ControlName))
        return false;
    return IsKeyPressed(Bindings[ControlName]);
}

bool Controls::IsControlReleased(std::string ControlName)
{
    if (ControlsPrecheck(ControlName))
        return false;
    return IsKeyReleased(Bindings[ControlName]);
}

void Controls::Update()
{
}

void Controls::Quit()
{
    Bindings.clear();
}
