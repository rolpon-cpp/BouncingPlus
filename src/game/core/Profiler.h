//
// Created by lalit on 2/22/2026.
//

#ifndef BOUNCINGPLUS_PROFILER_H
#define BOUNCINGPLUS_PROFILER_H
#include <map>
#include <string>
#include <vector>

class Game;

class Profiler
{
public:
    Game *game;
    std::string LastReason;
    double LastTime = -1;
    double LastCleared = -1;
    std::map<std::string, double> DisplayTimes;
    std::map<std::string,std::vector<double>> AvgTimes;
    Profiler();
    Profiler(Game &game);
    ~Profiler();
    void ProfilerLog(std::string Reason);
    void StopLog();
    std::map<std::string, double> Finish();
};


#endif //BOUNCINGPLUS_PROFILER_H