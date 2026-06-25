//
// Created by Rolpon on 2/22/2026.
//

#ifndef BOUNCINGPLUS_PROFILER_H
#define BOUNCINGPLUS_PROFILER_H
#include <map>
#include <string>
#include <vector>

class Game;

class BaseProfiler
{
public:
    std::string LastReason;
    double LastTime = -1;
    double LastCleared = -1;
    std::map<std::string, double> DisplayTimes;
    std::map<std::string, std::vector<double>> AvgTimes;
    BaseProfiler();
    ~BaseProfiler();
    void ProfilerLog(std::string Reason);
    void StopLog();
    std::map<std::string, double> Finish();
};


#endif //BOUNCINGPLUS_PROFILER_H