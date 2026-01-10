// Copyright Mia Rolfe. All rights reserved.
#include <Timer.h>

namespace ART
{

void Timer::Start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

void Timer::Stop()
{
    m_end = std::chrono::high_resolution_clock::now();
}

double Timer::ElapsedMilliseconds() const
{
    const std::chrono::duration<double, std::milli> elapsed = m_end - m_start;
    return elapsed.count();
}

} // namespace ART
