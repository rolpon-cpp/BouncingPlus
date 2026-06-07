//
// Created by lalit on 3/29/2026.
//

#ifndef BOUNCINGPLUS_CONTROLS_H
#define BOUNCINGPLUS_CONTROLS_H
#include <map>
#include <string>


class Controls
{
    public:
    std::map<std::string, int> Bindings;
    void SetDefaultBindings();
    Controls();
    ~Controls();
    bool ControlsPrecheck(std::string ControlName);
    bool IsControlDown(std::string ControlName);
    bool IsControlUp(std::string ControlName);
    bool IsControlPressed(std::string ControlName);
    bool IsControlReleased(std::string ControlName);
    void Update();
    void Quit();
};


#endif //BOUNCINGPLUS_CONTROLS_H