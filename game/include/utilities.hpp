#pragma once
#include <chrono>
#include <cmath>

class Timer
{
public:
    using clock_t        = std::chrono::steady_clock;
    using seconds_t      = std::chrono::duration<double, std::ratio<1>>;
    using milliseconds_t = std::chrono::duration<double, std::milli>;
    using time_point_t   = std::chrono::time_point<clock_t>;

    Timer();

    void                                    reset();
    void                                    pause();
    void                                    proceed();
    seconds_t                               elapsed() const;
    std::chrono::time_point<Timer::clock_t> timerNow();

private:
    void handlePauseDuration();

    mutable clock_t::duration                m_elapsed{};
    mutable std::chrono::time_point<clock_t> m_lastUpdate{};
    bool                                     m_isPauseEnabled{};
};

void normalizeLoopDuration(
    std::chrono::duration<double, std::ratio<1>> loopElapsedTime,
    int                                          refreshRate = 60);

template <typename T>
bool almostEqual(T p1, T p2)
{
    return std::abs(p1 - p2) < std::numeric_limits<T>::epsilon();
}

int getRandom(int lowRange, int highRange);
