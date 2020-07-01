#include "world_objects.hpp"
#include <algorithm>
#include <iostream>
namespace Model
{

PhysicalObject::PhysicalObject(worldCalcType in_m, worldCalcType in_r,
                               worldCalcType in_c, worldCalcType in_width,
                               worldCalcType in_height)
    : width{ in_width }
    , height{ in_height }
    , m{ in_m }
    , r{ in_r }
    , c{ in_c }
{
}

void PhysicalObject::update(seconds_t dt)
{
    ax = Motion::calcNextA(m, forceX);

    ay = Motion::calcNextA(m, forceY);

    auto newVX =
        Motion::calcNextV(vx, ax, static_cast<worldCalcType>(dt.count()));

    auto newVY =
        Motion::calcNextV(vy, ay, static_cast<worldCalcType>(dt.count()));

    x += Motion::calcDs(vx, newVX, static_cast<worldCalcType>(dt.count()));

    y += Motion::calcDs(vy, newVY, static_cast<worldCalcType>(dt.count()));

    vx = newVX;
    vy = newVY;

    angleAcceleration =
        Rotation::calcNextAngleAcceleration(forceMoment, inertionMoment);

    auto newAngleSpeed = Rotation::calcNextAngleSpeed(
        angleSpeed, angleAcceleration, static_cast<worldCalcType>(dt.count()));

    angle += Rotation::calcShiftAngle(angleSpeed, newAngleSpeed,
                                      static_cast<worldCalcType>(dt.count()));

    if (angle >= 2 * M_PI)
    {
        angle -= 2 * M_PI;
    }

    if (angle <= 2 * M_PI)
    {
        angle += 2 * M_PI;
    }

    angleSpeed = newAngleSpeed;

    lastUpdateTime =
        std::chrono::time_point_cast<std::chrono::steady_clock::duration>(
            lastUpdateTime + dt);
}

void PhysicalObject::applyExternalForce(worldCalcType externalForceX,
                                        worldCalcType externalForceY,
                                        worldCalcType externalForceMoment)
{
    forceX      = externalForceX;
    forceY      = externalForceY;
    forceMoment = externalForceMoment;
}

void PhysicalObject::teleportationHack(worldCalcType newX, worldCalcType newY)
{
    x = newX;
    y = newY;
}

worldCalcType TrailCloud::getCurrentPower() const
{
    const auto lastPowerOfTime =
        1 - (lastUpdateTime - initialTime) / TrailCloud::timeToLive;

    const auto currentPowerCoef =
        (lastPowerOfTime > 0.0) ? lastPowerOfTime : 0.0;

    return initPower * currentPowerCoef;
}

Rocket::Rocket(worldCalcType in_m, worldCalcType in_r, worldCalcType in_c,
               worldCalcType in_width, worldCalcType in_height,
               worldCalcType in_mainEngineMaxThrust,
               worldCalcType in_sideEngineMaxThrust)
    : PhysicalObject(in_m, in_r, in_c, in_width, in_height)
    , mainEngine{ in_mainEngineMaxThrust }
    , sideEngines{ { { in_sideEngineMaxThrust },
                     { in_sideEngineMaxThrust },
                     { in_sideEngineMaxThrust },
                     { in_sideEngineMaxThrust } } }
{
}

void Rocket::setEvents(const RocketEvents& newEvents)
{
    events = newEvents;
}

void Rocket::update(seconds_t dt)
{
    handleEvents(events);

    PhysicalObject::update(dt);
    mainEngine.updateTime(lastUpdateTime);
    std::for_each(sideEngines.begin(), sideEngines.end(),
                  [this](RocketEngine& currentEngine) {
                      currentEngine.updateTime(lastUpdateTime);
                  });
    handleClouds(lastUpdateTime);
}

void Rocket::applyExternalForce(worldCalcType externalForceX,
                                worldCalcType externalForceY,
                                worldCalcType externalForceMoment)
{
    const auto directEngineForceAbsolute = mainEngine.getCurrentAbsoluteForce();
    const auto directEngineForceX =
        directEngineForceAbsolute * std::sin(-angle);
    const auto directEngineForceY =
        directEngineForceAbsolute * std::cos(-angle);

    std::array<worldCalcType, 4> sideEnginesAbsoluteForces;
    std::transform(sideEngines.begin(), sideEngines.end(),
                   sideEnginesAbsoluteForces.begin(),
                   [](const RocketEngine& currentEngine) {
                       return currentEngine.getCurrentAbsoluteForce();
                   });

    const auto sumEnginesForce =
        sideEnginesAbsoluteForces[0] + sideEnginesAbsoluteForces[1] -
        (sideEnginesAbsoluteForces[2] + sideEnginesAbsoluteForces[3]);

    const auto sumSideEnginesForceX =
        sumEnginesForce * std::sin(-(angle - M_PI_2));

    const auto sumSideEnginesForceY =
        sumEnginesForce * std::cos(-(angle - M_PI_2));

    const auto sumForceX =
        externalForceX + directEngineForceX + sumSideEnginesForceX;
    const auto sumForceY =
        externalForceY + directEngineForceY + sumSideEnginesForceY;

    std::array<worldCalcType, 4> enginesForceMoments;

    std::transform(sideEnginesAbsoluteForces.begin(),
                   sideEnginesAbsoluteForces.end(), enginesForceMoments.begin(),
                   [this](worldCalcType currentEngineForce) {
                       return Rotation::calcForceMoment(currentEngineForce, r);
                   });

    const auto sumEnginesMoment =
        enginesForceMoments[0] + enginesForceMoments[3] -
        (enginesForceMoments[1] + enginesForceMoments[2]);

    auto stabilizedEngineMoment = sumEnginesMoment;
    if (stabilizationLevel1Enabled)
    {

        if (((angleSpeed > 0) &&
             (sumEnginesMoment <
              -2 * std::numeric_limits<worldCalcType>::epsilon())) ||
            ((angleSpeed < 0) &&
             (sumEnginesMoment >
              2 * std::numeric_limits<worldCalcType>::epsilon())))
        {
            if (std::abs(angleSpeed) > stabilizationAccuracy)
            {
                stabilizedEngineMoment *= 2;
            }
        }
    }

    const auto sumMoment = externalForceMoment + stabilizedEngineMoment;

    PhysicalObject::applyExternalForce(sumForceX, sumForceY, sumMoment);
}

void Rocket::handleEvents(const RocketEvents& events)
{
    auto isContainUp = events.find(Events::commandForward) != events.end();
    auto isContainRotateLeft =
        events.find(Events::commandRotateLeft) != events.end();
    auto isContainRotateRight =
        events.find(Events::commandRotateRight) != events.end();

    if (isContainUp)
    {
        mainEngine.enable();
    }
    else
    {
        mainEngine.disable();
    }

    const auto isSideEnginesDisabled =
        !isContainRotateLeft && !isContainRotateRight;

    if (isSideEnginesDisabled && stabilizationLevel2Enabled)
    {
        if (angleSpeed > stabilizationAccuracy)
        {
            isContainRotateRight = true;
        }
        else if (angleSpeed < -stabilizationAccuracy)
        {
            isContainRotateLeft = true;
        }
    }

    if (isContainRotateLeft)
    {
        sideEngines[0].enable();
        sideEngines[3].enable();
    }
    else
    {
        sideEngines[0].disable();
        sideEngines[3].disable();
    }

    if (isContainRotateRight)
    {
        sideEngines[1].enable();
        sideEngines[2].enable();
    }
    else
    {
        sideEngines[1].disable();
        sideEngines[2].disable();
    }
}

void Rocket::setMainEngineThrust(worldCalcType mainEngineThrust)
{
    mainEngine.enginePercentThrust = mainEngineThrust;
}

void Rocket::setSideEnginesThrust(worldCalcType sideEnginesThrust)
{
    std::for_each(sideEngines.begin(), sideEngines.end(),
                  [sideEnginesThrust](RocketEngine& currentEngine) {
                      currentEngine.enginePercentThrust = sideEnginesThrust;
                  });
}

Bullet Rocket::getBullet()
{
    static const double    shoutPerSecond = 3;
    static const seconds_t generationPeriod{ 1 / shoutPerSecond };
    const auto             elapsedTime =
        std::chrono::duration_cast<seconds_t>(lastUpdateTime - lastTimeWeapon);
    Bullet bullet;
    if (elapsedTime >= generationPeriod)
    {
        bullet.creatingTime = lastUpdateTime;
        const auto verticalShift =
            0.5 * (defaultHeight + 2 * Bullet::bulletDefaultSize);
        bullet.x       = x - verticalShift * std::sin(angle);
        bullet.y       = y + verticalShift * std::cos(angle);
        bullet.angle   = angle;
        bullet.vx      = vx - Bullet::bulletRelativeSpeed * std::sin(angle);
        bullet.vy      = vy + Bullet::bulletRelativeSpeed * std::cos(angle);
        lastTimeWeapon = lastUpdateTime;
        std::clog << "Bullet throwed\n";
    }
    return bullet;
}

void Rocket::handleClouds(time_point_t lastUpdateTime)
{
    static const double cloudPerSecond = 1.5;
    static const double numberOfClouds =
        TrailCloud::timeToLive.count() * cloudPerSecond;
    static const seconds_t generationPeriod{ 1 / cloudPerSecond };
    const auto             elapsedTime =
        std::chrono::duration_cast<seconds_t>(lastUpdateTime - lastTimeClouds);

    if (elapsedTime >= generationPeriod)
    {
        const auto currentMainEnginePower =
            mainEngine.getCurrentAbsoluteForce() / mainEngine.engineMaxForce;
        const auto smokeVisibility = 0.8;
        const auto smokeSize =
            currentMainEnginePower * TrailCloud::heightDefault;
        const auto verticalShift = 0.5 * (defaultHeight + 2 * smokeSize);
        clouds.push_front({ x + verticalShift * std::sin(angle),
                            y - verticalShift * std::cos(angle), angle,
                            smokeVisibility, lastUpdateTime, lastUpdateTime,
                            smokeSize, smokeSize });

        if (clouds.size() > std::round(numberOfClouds))
        {
            clouds.pop_back();
        }
        lastTimeClouds = lastUpdateTime;
    }
    for (auto& cloud : clouds)
    {
        cloud.lastUpdateTime = lastUpdateTime;
    }
}

RocketEngine::RocketEngine(worldCalcType inEngineMaxForce,
                           worldCalcType inEngineInitialThrust)
    : engineMaxForce{ inEngineMaxForce }
    , enginePercentThrust{ inEngineInitialThrust }
{
}

void RocketEngine::enable()
{
    m_engineState.set(true);
}

void RocketEngine::disable()
{
    m_engineState.set(false);
}

void RocketEngine::updateTime(time_point_t newTime)
{
    m_engineState.updateTime(newTime);
}

const WorldObjectState& RocketEngine::getEngineState() const
{
    return m_engineState;
}

worldCalcType RocketEngine::getCurrentAbsoluteForce() const
{
    return (m_engineState.getState())
               ? engineMaxForce * (enginePercentThrust / 100) *
                     (m_engineHealth / 100)
               : 0.0;
}

Planet::Planet(worldCalcType in_m, worldCalcType in_r, worldCalcType in_c,
               worldCalcType in_width, worldCalcType in_height)
    : PhysicalObject(in_m, in_r, in_c, in_width, in_height)
{
}

Asteroid::Asteroid(worldCalcType in_m, worldCalcType in_r, worldCalcType in_c,
                   worldCalcType in_width, worldCalcType in_height)
    : PhysicalObject(in_m, in_r, in_c, in_width, in_height)
{
}

Star::Star(worldCalcType in_m, worldCalcType in_r, worldCalcType in_c,
           worldCalcType in_width, worldCalcType in_height)
    : Planet(in_m, in_r, in_c, in_width, in_height)
{
}

} // namespace Model
