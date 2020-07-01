#pragma once
#include <cstdlib>
#include <limits>
#define _USE_MATH_DEFINES
#include <math.h>

namespace Model
{
using worldCalcType = double;

class Gravity
{
public:
    static constexpr worldCalcType calcG(
        const worldCalcType G = gravityConstant,
        const worldCalcType M = massOfEarth,
        const worldCalcType R = radiusOfEarth)
    {
        const auto safeR =
            (R > std::numeric_limits<worldCalcType>::epsilon()) ? R : 1.0;
        return G * M / (safeR * safeR);
    }

    static constexpr worldCalcType calcFgravity(const worldCalcType m,
                                                const worldCalcType g = calcG())
    {
        return m * g;
    }

    static constexpr worldCalcType calcFgravity(const worldCalcType m1,
                                                const worldCalcType m2,
                                                const worldCalcType r)
    {
        return calcG(gravityConstant, m1, r) * m2;
    }

    static constexpr worldCalcType gravityConstant = 6.67384e-11;
    static constexpr worldCalcType massOfEarth     = 5.972e24;
    static constexpr worldCalcType radiusOfEarth   = 6'371e3;
};

class Motion
{
public:
    static constexpr worldCalcType calcDs(const worldCalcType v0,
                                          const worldCalcType v1,
                                          const worldCalcType dt)
    {
        return (v0 + v1) * dt / 2;
    }

    static constexpr worldCalcType calcNextV(const worldCalcType v0,
                                             const worldCalcType a,
                                             const worldCalcType dt)
    {
        return v0 + a * dt;
    }

    static constexpr worldCalcType calcNextA(const worldCalcType m,
                                             const worldCalcType superForce)
    {
        // zero m should be checked before calling calcNextA
        if (m == 0)
        {
            return 0;
        }
        return (superForce) / m;
    }

    static constexpr worldCalcType calcCommandA(const worldCalcType F,
                                                const worldCalcType m)
    {
        if (m == 0)
        {
            return 0;
        }
        return F / m;
    }
};

class Resist
{
public:
    static worldCalcType calcFresist(const worldCalcType v,
                                     const worldCalcType k1,
                                     const worldCalcType k2)
    {
        const auto fresistK1 = -(k1 * v);
        const auto fresistK2 = -(k2 * v * std::abs(v));
        return fresistK1 + fresistK2;
    }

    static constexpr worldCalcType calcK1(
        const worldCalcType I, const worldCalcType mu = defaultMuOfEnvironement)
    {
        return static_cast<worldCalcType>(6) *
               static_cast<worldCalcType>(M_PI) * mu * I;
    }

    static constexpr worldCalcType calcK2(
        const worldCalcType s, const worldCalcType c,
        const worldCalcType ro = defaultRoOfEnvironement)
    {
        return s * c * ro / 2;
    }

    static constexpr worldCalcType defaultMuOfEnvironement = 0.0005; //= 0.0182;
    static constexpr worldCalcType defaultRoOfEnvironement = 0.0005; //= 1.29;
};

class Rotation
{
public:
    static constexpr worldCalcType calcShiftAngle(
        const worldCalcType angleSpeed0, const worldCalcType angleSpeed1,
        const worldCalcType dt)
    {
        return (angleSpeed0 + angleSpeed1) * dt / 2;
    }

    static constexpr worldCalcType calcNextAngleSpeed(
        const worldCalcType angleSpeed0, const worldCalcType angleAcceleration,
        const worldCalcType dt)
    {
        return angleSpeed0 + angleAcceleration * dt;
    }

    static constexpr worldCalcType calcNextAngleAcceleration(
        const worldCalcType fullMomentOfForce,
        const worldCalcType momentOfInertion)
    {
        if (momentOfInertion == 0)
        {
            return 0;
        }
        return fullMomentOfForce / momentOfInertion;
    }

    static constexpr worldCalcType calcForceMoment(const worldCalcType force,
                                                   const worldCalcType shoulder)
    {
        return force * shoulder;
    }

    static constexpr worldCalcType calcLinearSpeed(
        const worldCalcType angleSpeed, const worldCalcType r)
    {
        return angleSpeed * r;
    }

    static constexpr worldCalcType calcMomentInertion(const worldCalcType m,
                                                      const worldCalcType d)
    {
        return m * d * d / 12;
    }
};
} // namespace Model
