//
// Created by lalit on 2/22/2026.
//

#include "Profiler.h"

#include "../Game.h"

Profiler::Profiler()
{
}

Profiler::Profiler(Game& game)
{
    this->game = &game;
}

Profiler::~Profiler()
{
}

void Profiler::ProfilerLog(std::string Reason)
{
    StopLog();
    LastReason = Reason;
    LastTime = GetTime();
}

void Profiler::StopLog()
{
    if (!LastReason.empty())
    {
        if (!AvgTimes.count(LastReason))
            AvgTimes[LastReason] = std::vector<double>();
        AvgTimes[LastReason].push_back(GetTime() - LastTime);
        LastReason = "";
        LastTime = -1;
    }
}

std::map<std::string, double> Profiler::Finish()
{
    StopLog();
    if (GetTime() - LastCleared >= 1)
    {
        for (auto &[name, val] : AvgTimes)
        {
            double f = 0;
            for (double g : val)
                f += g;
            if (f != 0)
                f /= val.size();

            DisplayTimes[name] = f;
        }

        AvgTimes.clear();
        LastCleared = GetTime();
    }
    return DisplayTimes;
}
