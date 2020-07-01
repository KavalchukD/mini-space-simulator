#include "world.hpp"
#include "collision_detection.hpp"
#include "global.hpp"
#include <algorithm>
#include <iostream>

namespace Model
{

World::World(std::chrono::time_point<clock_t> initialTime)
    : lastUpdateTime{ initialTime }
{
    rockets.push_back({});
    userShipPtr    = &rockets.front();
    userShipPtr->x = 0;
    userShipPtr->y = 6000;

    rockets.push_back({});
    rockets.back().x = 400;
    rockets.back().y = 6100;
    rockets.back().setMainEngineThrust(50);

    planets.push_back({ Planet::defaultM / 3 });
    planets.back().x  = 0;
    planets.back().y  = 1200;
    planets.back().vx = std::sqrt(
        (Gravity::gravityConstant * Star::defaultM / planets.back().y));
    planets.back().angleSpeed = 0.25;

    planets.push_back({});
    planets.back().x  = 0;
    planets.back().y  = 5200;
    planets.back().vx = std::sqrt(
        (Gravity::gravityConstant * Star::defaultM / planets.back().y));
    planets.back().angleSpeed = 0.25;

    userShipPtr->vx = planets.back().vx +
                      std::sqrt(Gravity::gravityConstant * planets.back().m /
                                (userShipPtr->y - planets.back().y));
    rockets.back().vx = planets.back().vx +
                        std::sqrt(Gravity::gravityConstant * planets.back().m /
                                  (rockets.back().y - planets.back().y));

    asteroids.push_back({});
    asteroids.back().x = 0;
    asteroids.back().y = 5700;
    asteroids.back().vx =
        planets.back().vx -
        std::sqrt(Gravity::gravityConstant * Planet::defaultM /
                  (asteroids.back().y - planets.back().y));
    asteroids.back().angleSpeed = 0.5;

    asteroids.push_back({});
    asteroids.back().x  = 700;
    asteroids.back().y  = 5200;
    asteroids.back().vx = planets.back().vx;
    asteroids.back().vy =
        std::sqrt(Gravity::gravityConstant * Planet::defaultM /
                  (asteroids.back().x - planets.back().x));
    asteroids.push_back({});
    asteroids.back().x  = -700;
    asteroids.back().y  = 5200;
    asteroids.back().vx = planets.back().vx;
    asteroids.back().vy =
        std::sqrt(Gravity::gravityConstant * Planet::defaultM /
                  std::abs(asteroids.back().x - planets.back().x));

    asteroids.back().angleSpeed = 0.4;

    asteroids.push_back({});
    asteroids.back().x  = 0;
    asteroids.back().y  = 2400;
    asteroids.back().vx = -std::sqrt(Gravity::gravityConstant * Star::defaultM /
                                     (asteroids.back().y));
    asteroids.back().angleSpeed = 1.0;

    asteroids.push_back({});
    asteroids.back().x  = 0;
    asteroids.back().y  = -2400;
    asteroids.back().vx = std::sqrt(Gravity::gravityConstant * Star::defaultM /
                                    std::abs(asteroids.back().y));

    asteroids.back().angleSpeed = -1.5;

    asteroids.push_back({});
    asteroids.back().x  = 3600;
    asteroids.back().y  = 0;
    asteroids.back().vy = std::sqrt(Gravity::gravityConstant * Star::defaultM /
                                    asteroids.back().x);
    asteroids.back().angleSpeed = -0.5;

    asteroids.push_back({});
    asteroids.back().x  = 4200;
    asteroids.back().y  = 0;
    asteroids.back().vy = std::sqrt(Gravity::gravityConstant * Star::defaultM /
                                    (asteroids.back().x));

    asteroids.back().angleSpeed = 0.4;

    stars.push_back({});
    stars.front().angleSpeed = 0.1;
}

void World::getUserRocketEvents(const WorldEvents& events)
{
    Rocket::RocketEvents rocketEvents{};
    auto&                currentUserShip = *userShipPtr;
    auto isContainUp = events.find(Events::userCommandShipUp) != events.end();
    if (isContainUp)
    {
        rocketEvents.insert(Rocket::Events::commandForward);
    }

    auto isContainRotateLeft =
        events.find(Events::userCommandShipRotateLeft) != events.end();
    if (isContainRotateLeft)
    {
        rocketEvents.insert(Rocket::Events::commandRotateLeft);
    }

    auto isContainRotateRight =
        events.find(Events::userCommandShipRotateRight) != events.end();
    if (isContainRotateRight)
    {
        rocketEvents.insert(Rocket::Events::commandRotateRight);
    }

    currentUserShip.setEvents(rocketEvents);

    auto isContainShoot =
        events.find(Events::userCommandShipAttack) != events.end();
    if (isContainShoot)
    {
        auto bullet = userShipPtr->getBullet();
        if (bullet.creatingTime != Timer::time_point_t{})
        {
            auto midX = userShipPtr->x;
            auto midY = userShipPtr->y;
#ifdef DEBUG_CONFIGURATION
            std::clog << "Shout \n";
#endif
            outEvents.push_back(
                { midX, midY, lastUpdateTime, OutEvent::Type::shoot });
            bullets.push_back(bullet);
        }
    }

    auto teleportationEventIt =
        events.find(Events::userCommandShipTeleporation);

    if (teleportationEventIt != events.end())
    {
        const auto& newXY = teleportationEventIt->second;
        currentUserShip.teleportationHack(newXY[0], newXY[1]);
    }
}

void World::enemiesForward()
{
    std::for_each(++rockets.begin(), rockets.end(), [](Rocket& rocket) {
        rocket.setEvents({ Rocket::Events::commandForward });
    });
}

static std::array<worldCalcType, 2> getForceFirstObjectToSecond(
    const PhysicalObject& obj1, const PhysicalObject& obj2)
{
    const auto dx = obj2.x - obj1.x;
    const auto dy = obj2.y - obj1.y;

    const auto distance{ std::sqrt(dx * dx + dy * dy) };

    const auto safeDistance{ ((obj1.r + obj2.r) < distance)
                                 ? distance
                                 : (obj1.r + obj2.r) };

    const auto gravityAbsoluteForce =
        Gravity::calcFgravity(obj1.m, obj2.m, safeDistance);

    const auto sinA = dy / safeDistance;
    const auto cosA = dx / safeDistance;

    const auto xProjection = cosA * gravityAbsoluteForce;
    const auto yProjection = sinA * gravityAbsoluteForce;
    return { xProjection, yProjection };
}

std::array<worldCalcType, 2> World::calcSumGravityForceToObject(
    const PhysicalObject& obj)
{
    std::array<worldCalcType, 2> sumForce{};

    auto addForceToOnePlanet = [&sumForce, &obj](const Planet& planet) {
        if (std::abs(obj.x - planet.x) <
                std::numeric_limits<worldCalcType>::epsilon() &&
            std::abs(obj.y - planet.y) <
                std::numeric_limits<worldCalcType>::epsilon())
            return std::array<worldCalcType, 2>{};
        const auto vectorOfGravityForce =
            getForceFirstObjectToSecond(obj, planet);
        sumForce[0] += vectorOfGravityForce[0];
        sumForce[1] += vectorOfGravityForce[1];
        return sumForce;
    };
    std::for_each(stars.begin(), stars.end(), addForceToOnePlanet);

    std::for_each(planets.begin(), planets.end(), addForceToOnePlanet);
    return sumForce;
}

void World::applyAllExternalForceToOneObject(PhysicalObject& object)
{
    const auto vectorOfGravityForce = calcSumGravityForceToObject(object);
    auto       externalForceX       = vectorOfGravityForce[0] +
                          Resist::calcFresist(object.vx, object.k1, object.k2);

    auto externalForceY =
        //-Gravity::calcFgravity(rocket.m) +
        vectorOfGravityForce[1] +
        Resist::calcFresist(object.vy, object.k1, object.k2);

    auto forceOfResistance = Resist::calcFresist(
        object.getLinearRotationSpeed(), object.k1, object.k2);

    auto externalForceMoments =
        2 * Rotation::calcForceMoment(forceOfResistance, object.r / 2);
    object.applyExternalForce(externalForceX, externalForceY,
                              externalForceMoments);
}

bool World::checkCollision(const Bullet& obj1, const Star& obj2)
{
    return World::checkCollision(obj1, static_cast<Planet>(obj2));
}

bool World::checkCollision(const Bullet& obj1, const Planet& obj2)
{
    const auto isCollided = CollisionDetection::isCollidedAABB(
        { { obj1.x, obj1.y }, { obj1.width, obj1.height } },
        { { obj2.x, obj2.y }, { obj2.width, obj2.height } });

    if (isCollided)
    {
        auto midX = (obj1.x + obj2.x) / 2;
        auto midY = (obj1.y + obj2.y) / 2;
#ifdef DEBUG_CONFIGURATION
        std::clog << "\n"
                  << "!!!Collision happened between: \n"
                  << obj1 << obj2 << "\n";
#endif
        outEvents.push_back(
            { midX, midY, lastUpdateTime, OutEvent::Type::hit });
    }
    return isCollided;
}

bool World::checkCollision(const PhysicalObject& obj1,
                           const PhysicalObject& obj2)
{
    const auto isCollided = CollisionDetection::isCollidedAABB(
        { { obj1.x, obj1.y }, { obj1.width, obj1.height } },
        { { obj2.x, obj2.y }, { obj2.width, obj2.height } });

    if (isCollided)
    {
        auto midX = (obj1.x + obj2.x) / 2;
        auto midY = (obj1.y + obj2.y) / 2;
#ifdef DEBUG_CONFIGURATION
        std::clog << "\n"
                  << "!!!Collision happened between: \n"
                  << obj1 << obj2 << "\n";
#endif
        outEvents.push_back(
            { midX, midY, lastUpdateTime, OutEvent::Type::explosion });
    }
    return isCollided;
}

void World::detectCollisionsBullets()
{
    auto nextBullet1 = bullets.begin();
    for (auto bullet1 = nextBullet1; bullet1 != bullets.end();
         bullet1      = nextBullet1)
    {
        bool stopCurrentIter = false;
        ++nextBullet1;

        for (auto rocket2 = rockets.begin(); rocket2 != rockets.end();
             ++rocket2)
        {
            if (checkCollision(*bullet1, *rocket2))
            {
                bullets.erase(bullet1);
                rockets.erase(rocket2);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto asteroid = asteroids.begin(); asteroid != asteroids.end();
             ++asteroid)
        {
            if (checkCollision(*bullet1, *asteroid))
            {
                bullets.erase(bullet1);
                asteroids.erase(asteroid);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto planet = planets.begin(); planet != planets.end(); ++planet)
        {
            if (checkCollision(*bullet1, *planet))
            {
                bullets.erase(bullet1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto star = stars.begin(); star != stars.end(); ++star)
        {
            if (checkCollision(*bullet1, *star))
            {
                bullets.erase(bullet1);
                break;
            }
        }
    }
}

void World::detectCollisionsRockets()
{
    auto nextRocket1 = rockets.begin();
    for (auto rocket1 = rockets.begin(); rocket1 != rockets.end();
         rocket1      = nextRocket1)
    {
        bool stopCurrentIter = false;
        ++nextRocket1;

        for (auto rocket2 = nextRocket1; rocket2 != rockets.end(); ++rocket2)
        {
            if (checkCollision(*rocket1, *rocket2))
            {
                rockets.erase(rocket2);
                nextRocket1     = rockets.erase(rocket1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto asteroid = asteroids.begin(); asteroid != asteroids.end();
             ++asteroid)
        {
            if (checkCollision(*rocket1, *asteroid))
            {
                rockets.erase(rocket1);
                asteroids.erase(asteroid);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto planet = planets.begin(); planet != planets.end(); ++planet)
        {
            if (checkCollision(*rocket1, *planet))
            {
                rockets.erase(rocket1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto star = stars.begin(); star != stars.end(); ++star)
        {
            if (checkCollision(*rocket1, *star))
            {
                rockets.erase(rocket1);
                break;
            }
        }
    }
}

void World::detectCollisionsAsteroids()
{
    auto nextAsteroid1 = asteroids.begin();
    for (auto asteroid1 = asteroids.begin(); asteroid1 != asteroids.end();
         asteroid1      = nextAsteroid1)
    {
        bool stopCurrentIter = false;
        ++nextAsteroid1;
        for (auto asteroid2 = nextAsteroid1; asteroid2 != asteroids.end();
             ++asteroid2)
        {
            if (checkCollision(*asteroid1, *asteroid2))
            {
                asteroids.erase(asteroid2);
                nextAsteroid1   = asteroids.erase(asteroid1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto planet = planets.begin(); planet != planets.end(); ++planet)
        {
            if (checkCollision(*asteroid1, *planet))
            {
                asteroids.erase(asteroid1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto star = stars.begin(); star != stars.end(); ++star)
        {
            if (checkCollision(*asteroid1, *star))
            {
                asteroids.erase(asteroid1);
                break;
            }
        }
    }
}

void World::detectCollisionsPlanets()
{
    auto nextPlanet1 = planets.begin();
    for (auto planet1 = planets.begin(); planet1 != planets.end();
         planet1      = nextPlanet1)
    {
        bool stopCurrentIter = false;
        ++nextPlanet1;
        for (auto planet2 = nextPlanet1; planet2 != planets.end(); ++planet2)
        {
            if (checkCollision(*planet1, *planet2))
            {
                planets.erase(planet2);
                nextPlanet1     = planets.erase(planet1);
                stopCurrentIter = true;
                break;
            }
        }
        if (stopCurrentIter)
        {
            continue;
        }
        for (auto star = stars.begin(); star != stars.end(); ++star)
        {
            if (checkCollision(*planet1, *star))
            {
                planets.erase(planet1);
                break;
            }
        }
    }
}

void World::detectCollisions()
{
    detectCollisionsBullets();
    detectCollisionsRockets();
    detectCollisionsAsteroids();
    detectCollisionsPlanets();
    auto isUserRocketOk = std::any_of(
        rockets.begin(), rockets.end(), [this](const Rocket& currentRocket) {
            return (&currentRocket == userShipPtr);
        });
    gameOver = !isUserRocketOk;
}

void World::asteroidFunRandomizer()
{
    static time_point_t        lastTime{ lastUpdateTime };
    static constexpr seconds_t period{ 15 };
    static constexpr size_t    maxAmountOfAsteroids{ 50 };
    const auto                 dt = lastUpdateTime - lastTime;

    if ((dt > period) && (asteroids.size() < maxAmountOfAsteroids))
    {
        auto isAddAsteroidOk = false;
        while (!isAddAsteroidOk)
        {
            const auto x = static_cast<worldCalcType>(getRandom(-6000, 6000));
            const auto y = static_cast<worldCalcType>(getRandom(-6000, 6000));

            const auto distance = std::sqrt(x * x + y * y);

            const auto dxToUser = (x - userShipPtr->x);
            const auto dyToUser = (y - userShipPtr->y);
            const auto distanceToUser =
                std::sqrt(dxToUser * dxToUser + dyToUser * dyToUser);

            if (distance < 300 || distanceToUser < 800)
                continue;

            const auto absoluteV = std::sqrt(Gravity::gravityConstant *
                                             Planet::defaultM / distance);
            const auto sinA      = x / distance;
            const auto cosA      = y / distance;

            asteroids.push_back({});
            asteroids.back().x          = x;
            asteroids.back().y          = y;
            asteroids.back().vx         = absoluteV * cosA;
            asteroids.back().vy         = absoluteV * sinA;
            asteroids.back().angleSpeed = (x + y) / 9000;
            lastTime                    = lastUpdateTime;
            isAddAsteroidOk             = true;
        }
    }
}

bool World::update(std::chrono::time_point<clock_t> nowTime,
                   const WorldEvents&               events)
{
    outEvents.clear();
    getUserRocketEvents(events);
    enemiesForward();
    asteroidFunRandomizer();

    while (lastUpdateTime + dt < nowTime)
    {
        lastUpdateTime =
            std::chrono::time_point_cast<std::chrono::steady_clock::duration>(
                lastUpdateTime + dt);

        for (auto& rocket : rockets)
        {
            applyAllExternalForceToOneObject(rocket);
        }
        for (auto& planet : planets)
        {
            applyAllExternalForceToOneObject(planet);
        }
        for (auto& asteroid : asteroids)
        {
            applyAllExternalForceToOneObject(asteroid);
        }
        for (auto& rocket : rockets)
        {
            rocket.update(dt);
        }
        for (auto& planet : planets)
        {
            planet.update(dt);
        }
        for (auto& asteroid : asteroids)
        {
            asteroid.update(dt);
        }
        for (auto& bullet : bullets)
        {
            bullet.update(dt);
        }
        detectCollisions();

        if (gameOver)
        {
            return false;
        }
    }

    bullets.remove_if([this](const Bullet& bullet) {
        return !bullet.isAlive(lastUpdateTime);
    });

    Global::setUserPosition(userShipPtr->x, userShipPtr->y);
    return true;
}

std::ostream& operator<<(std::ostream& out, const PhysicalObject& object)
{
    out << "X: " << object.x << "Y: " << object.y << " Vx: " << object.vx
        << " Vy: " << object.vy << " Ax: " << object.ax << " Ay: " << object.ay
        << " angle: " << object.angle << " angleSpeed : " << object.angleSpeed
        << " angleAccel " << object.angleAcceleration
        << " linear rotating speed "
        << Rotation::calcLinearSpeed(object.angleSpeed, object.r) << '\n';
    return out;
}

std::ostream& operator<<(std::ostream& out, const World& world)
{
    out << "List of objects: " << '\n';
    std::for_each_n(world.rockets.begin(), world.rockets.size(),
                    [&out](const PhysicalObject& object) { out << object; });
    out << " T: " << world.lastUpdateTime.time_since_epoch().count() / 1.0e9
        << '\n';
    return out;
}
} // namespace Model
