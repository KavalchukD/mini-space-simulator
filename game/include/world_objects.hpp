#pragma once
#include "utilities.hpp"
#include "world_physics.hpp"
#include <array>
#include <deque>
#include <iosfwd>
#include <unordered_set>

namespace Model
{
class WorldObjectState
{
public:
    void updateTime(Timer::time_point_t currentTime)
    {
        m_currentTime = currentTime;
    }

    void set(bool state)
    {
        if (m_state == state)
        {
            return;
        }
        m_state           = state;
        m_changeStatetime = m_currentTime;
    }

    bool             getState() const { return m_state; }
    Timer::seconds_t getTime() const
    {
        return m_currentTime - m_changeStatetime;
    }

private:
    bool                m_state{};
    Timer::time_point_t m_changeStatetime{ Timer::clock_t::duration::max() };
    Timer::time_point_t m_currentTime{};
};

class PhysicalObject
{
public:
    using clock_t      = Timer::clock_t;
    using seconds_t    = Timer::seconds_t;
    using time_point_t = Timer::time_point_t;

    PhysicalObject(worldCalcType in_m, worldCalcType in_r, worldCalcType in_c,
                   worldCalcType in_width, worldCalcType in_height);
    PhysicalObject() = default;
    virtual ~PhysicalObject() {}
    worldCalcType x{};
    worldCalcType y{};

    worldCalcType width{};
    worldCalcType height{};

    worldCalcType vx{};
    worldCalcType vy{};
    worldCalcType ax{};
    worldCalcType ay{};

    worldCalcType angle{};
    worldCalcType angleSpeed{};
    worldCalcType angleAcceleration{};

    worldCalcType forceX{};
    worldCalcType forceY{};
    worldCalcType forceMoment{};

    worldCalcType getLinearRotationSpeed() const
    {
        return Rotation::calcLinearSpeed(angleSpeed, r);
    }

    const worldCalcType m{}; // kg
    const worldCalcType r{}; // meters

    const worldCalcType c{};                                          // o.e.
    const worldCalcType i = static_cast<worldCalcType>(r);            // meter
    const worldCalcType s = static_cast<worldCalcType>(M_PI * r * r); // meter2
    const worldCalcType k1{ Resist::calcK1(i) };
    const worldCalcType k2{ Resist::calcK2(s, c) };

    const worldCalcType inertionMoment = Rotation::calcMomentInertion(m, 2 * r);

    friend std::ostream& operator<<(std::ostream&         out,
                                    const PhysicalObject& object);

    void teleportationHack(worldCalcType newX, worldCalcType newY);

    virtual void update(seconds_t dt);

    virtual void applyExternalForce(worldCalcType externalForceX,
                                    worldCalcType externalForceY,
                                    worldCalcType forceMoment);

protected:
    seconds_t getDurationSinceLastUpdate() const
    {
        return std::chrono::duration_cast<seconds_t>(
            lastUpdateTime.time_since_epoch());
    }

    time_point_t lastUpdateTime{};
};

class RocketEngine
{
public:
    using clock_t      = Timer::clock_t;
    using seconds_t    = Timer::seconds_t;
    using time_point_t = Timer::time_point_t;

    RocketEngine() = default;

    RocketEngine(worldCalcType engineMaxForce,
                 worldCalcType engineInitialThrust = 100);

    const WorldObjectState& getEngineState() const;
    worldCalcType           getCurrentAbsoluteForce() const;
    void                    enable();
    void                    disable();
    void                    updateTime(time_point_t newTime);

    const worldCalcType engineMaxForce{};
    worldCalcType       enginePercentThrust{ 100 };

private:
    WorldObjectState m_engineState{};
    worldCalcType    m_engineHealth{ 100 };
};

struct TrailCloud
{
    worldCalcType                     x{};
    worldCalcType                     y{};
    worldCalcType                     angle{};
    worldCalcType                     initPower{};
    Timer::time_point_t               initialTime;
    Timer::time_point_t               lastUpdateTime;
    worldCalcType                     width{};
    worldCalcType                     height{};
    static constexpr worldCalcType    widthDefault{ 45 };
    static constexpr worldCalcType    heightDefault{ 45 };
    static constexpr Timer::seconds_t timeToLive{ 3.0 };
    worldCalcType                     getCurrentPower() const;
};

class Bullet : public PhysicalObject
{
public:
    Bullet()
        : PhysicalObject(0.0, bulletDefaultSize, 0.0, bulletDefaultWidth,
                         bulletDefaultHeight)
    {
    }

    ~Bullet() override {}

    bool isAlive(time_point_t nowTime) const
    {
        auto dif = nowTime - creatingTime;
        return dif <= timeToLive;
    }
    Timer::time_point_t               creatingTime;
    static constexpr Timer::seconds_t timeToLive{ 3.0 };
    static constexpr worldCalcType    bulletDefaultSize{ 40.0 };
    static constexpr worldCalcType    bulletDefaultHeight{ bulletDefaultSize };
    static constexpr worldCalcType    bulletDefaultWidth{ bulletDefaultHeight /
                                                       4 };
    static constexpr worldCalcType    bulletRelativeSpeed{ 750.0 };
};

class Rocket : public PhysicalObject
{
public:
    enum class Events
    {
        commandForward,
        commandRotateLeft,
        commandRotateRight
    };
    Rocket(worldCalcType in_m = defaultM, worldCalcType in_r = defaultR,
           worldCalcType in_c = defaultC, worldCalcType in_width = defaultWidth,
           worldCalcType in_height              = defaultHeight,
           worldCalcType in_mainEngineMaxThrust = mainEngineDefaultThrust,
           worldCalcType in_sideEngineMaxThrust = sideEngineDefaultThrust);

    ~Rocket() override {}

    using RocketEvents = std::unordered_set<Rocket::Events>;

    void setEvents(const RocketEvents& newEvents);

    void update(seconds_t dt) override;
    void applyExternalForce(worldCalcType externalForceX,
                            worldCalcType externalForceY,
                            worldCalcType externalForceMoment) override;

    void setMainEngineThrust(worldCalcType mainEngineThrust);
    void setSideEnginesThrust(worldCalcType sideEnginesThrust);

    bool stabilizationLevel1Enabled{ true };
    bool stabilizationLevel2Enabled{ false };

    RocketEngine mainEngine{ mainEngineDefaultThrust };

    /// 0 - down left, 1 - upper left, 2 - down right, 3 - upper right
    std::array<RocketEngine, 4>    sideEngines{ { { sideEngineDefaultThrust },
                                               { sideEngineDefaultThrust },
                                               { sideEngineDefaultThrust },
                                               { sideEngineDefaultThrust } } };
    std::deque<TrailCloud>         clouds{};
    static constexpr worldCalcType defaultWidth{ 60 };
    static constexpr worldCalcType defaultHeight{ defaultWidth * 2 };
    static constexpr worldCalcType mainEngineDefaultThrust{ 5000 };
    static constexpr worldCalcType sideEngineDefaultThrust{ 1 };
    static constexpr worldCalcType defaultM{ 100 };
    static constexpr worldCalcType defaultR{ 0.3 };
    static constexpr worldCalcType defaultC{ 0.4 };
    Bullet                         getBullet();

private:
    RocketEvents                   events;
    static constexpr worldCalcType stabilizationAccuracy{ 0.003 };

    void         handleClouds(time_point_t lastUpdateTime);
    void         handleEvents(const RocketEvents& events);
    time_point_t lastTimeClouds{};
    time_point_t lastTimeWeapon{};
};

class Asteroid : public PhysicalObject
{
public:
    Asteroid(worldCalcType in_m = defaultM, worldCalcType in_r = defaultR,
             worldCalcType in_c      = defaultC,
             worldCalcType in_width  = defaultWidth,
             worldCalcType in_height = defaultHeight);

    ~Asteroid() override {}

    static constexpr worldCalcType defaultR{ 120 };
    static constexpr worldCalcType defaultWidth{ defaultR };
    static constexpr worldCalcType defaultHeight{ defaultR };
    static constexpr worldCalcType defaultM{ 10e5 };
    static constexpr worldCalcType defaultC{ 0.4 };
};

class Planet : public PhysicalObject
{
public:
    Planet(worldCalcType in_m = defaultM, worldCalcType in_r = defaultR,
           worldCalcType in_c = defaultC, worldCalcType in_width = defaultWidth,
           worldCalcType in_height = defaultHeight);

    ~Planet() override {}

    static constexpr worldCalcType defaultR{ 250 };
    static constexpr worldCalcType defaultWidth{ defaultR };
    static constexpr worldCalcType defaultHeight{ defaultR };
    static constexpr worldCalcType defaultM{ Gravity::calcG() * 250.0e4 /
                                             (Gravity::gravityConstant) };
    static constexpr worldCalcType defaultC{ 0.4 };
};

class Star : public Planet
{
public:
    Star(worldCalcType in_m = defaultM, worldCalcType in_r = defaultR,
         worldCalcType in_c = defaultC, worldCalcType in_width = defaultWidth,
         worldCalcType in_height = defaultHeight);

    ~Star() override {}

    static constexpr worldCalcType defaultR{ 500 };
    static constexpr worldCalcType defaultWidth{ defaultR };
    static constexpr worldCalcType defaultHeight{ defaultR };
    static constexpr worldCalcType defaultM{ Gravity::calcG() * 900.0e4 /
                                             (Gravity::gravityConstant) };
    static constexpr worldCalcType defaultC{ 0.4 };
};
} // namespace Model
