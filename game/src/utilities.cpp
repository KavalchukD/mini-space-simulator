#include "utilities.hpp"
#include <iostream>
#include <random>
#include <thread>

Timer::Timer()
    : m_lastUpdate(clock_t::now())
{
}

void Timer::reset()
{
    m_elapsed = std::chrono::seconds(0);
}

void Timer::pause()
{
    m_isPauseEnabled = true;
}

void Timer::proceed()
{
    if (m_isPauseEnabled)
    {
        m_isPauseEnabled = false;
        m_lastUpdate     = clock_t::now();
    }
}

Timer::seconds_t Timer::elapsed() const
{
    if (!m_isPauseEnabled)
    {
        m_elapsed += clock_t::now() - m_lastUpdate;
        m_lastUpdate = clock_t::now();
    }
    return std::chrono::duration_cast<seconds_t>(m_elapsed);
}

std::chrono::time_point<Timer::clock_t> Timer::timerNow()
{
    if (!m_isPauseEnabled)
    {
        m_elapsed += clock_t::now() - m_lastUpdate;
        m_lastUpdate = clock_t::now();
    }
    const std::chrono::time_point<clock_t> nowTime(m_elapsed);
    return nowTime;
}

void Timer::handlePauseDuration() {}

void normalizeLoopDuration(
    std::chrono::duration<double, std::ratio<1>> loopElapsedTime,
    int                                          refreshRate)
{
    using seconds_t      = std::chrono::duration<double, std::ratio<1>>;
    using milliseconds_t = std::chrono::duration<double, std::milli>;

    static const auto epsilon = 0.8;

    static const seconds_t loopNormalizedDuration{
        (seconds_t{ 1 } / refreshRate) - milliseconds_t{ epsilon }
    };

    auto timeToWaitDouble = (loopNormalizedDuration - loopElapsedTime);
    if (timeToWaitDouble < seconds_t::zero())
        timeToWaitDouble = seconds_t::zero();

    std::this_thread::sleep_for(timeToWaitDouble);
}

int getRandom(int lowRange, int highRange)
{
    static std::random_device rd;
    static std::mt19937       mersenne(rd());
    return ((mersenne()) % (highRange - lowRange + 1) + lowRange);
}
