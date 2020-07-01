#pragma once
#include "utilities.hpp"
#include "world_objects.hpp"
#include "world_physics.hpp"
#include <array>
#include <iosfwd>
#include <list>
#include <unordered_map>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

namespace Model
{

struct OutEvent
{
    enum class Type
    {
        explosion,
        hit,
        shoot
    };

    worldCalcType       x;
    worldCalcType       y;
    Timer::time_point_t time;
    Type                type{};
    worldCalcType       sizeX{ defaultSizeX };
    worldCalcType       sizeY{ defaultSizeY };

    static constexpr worldCalcType defaultSizeX = 150;
    static constexpr worldCalcType defaultSizeY = 150;
};

class World
{
public:
    enum class Events : size_t
    {
        userCommandShipUp,
        userCommandShipTeleporation,
        userCommandShipRotateLeft,
        userCommandShipRotateRight,
        userCommandShipAttack,
        maxType,
    };

    using WorldEvents =
        std::unordered_map<World::Events, std::array<worldCalcType, 2>>;

    using clock_t        = Timer::clock_t;
    using seconds_t      = Timer::seconds_t;
    using milliseconds_t = Timer::milliseconds_t;
    using time_point_t   = Timer::time_point_t;

    World(time_point_t initialTime);

    [[nodiscard]] bool update(time_point_t nowTime, const WorldEvents& events);

    /// TODO change to other container
    std::list<Rocket> rockets;

    Rocket* userShipPtr{};

    std::list<Star> stars;

    std::list<Planet> planets;

    std::list<Asteroid> asteroids;

    std::list<Bullet> bullets;

    std::list<OutEvent> outEvents;

    time_point_t lastUpdateTime;
    seconds_t    dt{ milliseconds_t{ 4 } };

    friend std::ostream& operator<<(std::ostream& out, const World& world);

    bool gameOver{};

private:
    void                         enemiesForward();
    void                         getUserRocketEvents(const WorldEvents& events);
    std::array<worldCalcType, 2> calcSumGravityForceToObject(
        const PhysicalObject& obj1);
    void applyAllExternalForceToOneObject(PhysicalObject& object);
    void detectCollisions();
    void detectCollisionsBullets();
    void detectCollisionsRockets();
    void detectCollisionsAsteroids();
    void detectCollisionsPlanets();
    bool checkCollision(const Bullet& obj1, const Planet& obj2);
    bool checkCollision(const Bullet& obj1, const Star& obj2);
    bool checkCollision(const PhysicalObject& obj1, const PhysicalObject& obj2);
    void asteroidFunRandomizer();
};

} // namespace Model
