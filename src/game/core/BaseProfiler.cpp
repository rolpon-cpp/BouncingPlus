//
// Created by lalit on 2/22/2026.
//

#include "BaseProfiler.h"

#include "../Game.h"

BaseProfiler::BaseProfiler()
{
}

BaseProfiler::~BaseProfiler()
{
}

void BaseProfiler::ProfilerLog(std::string Reason)
{
    StopLog();
    LastReason = Reason;
    LastTime = GetTime();
}

void BaseProfiler::StopLog()
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

std::map<std::string, double> BaseProfiler::Finish()
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
