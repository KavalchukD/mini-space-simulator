#pragma once
#include <chrono>

struct WorldConstants
{
    std::chrono::duration<double, std::ratio<1>> dt{ 0.004 };
};
