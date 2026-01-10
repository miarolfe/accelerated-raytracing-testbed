// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <chrono>

namespace ART
{

class Timer
{
public:
    void Start();

    void Stop();

    double ElapsedMilliseconds() const;

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
};

} // namespace ART
