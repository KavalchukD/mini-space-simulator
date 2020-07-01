#pragma once
#include "global.hpp"
#include "world.hpp"
#include <functional>
#include <iengine.hpp>
#include <unordered_map>
#include <unordered_set>

class Environement
{
public:
    class HashEvent
    {
    public:
        size_t operator()(const om::Event& s) const
        {
            auto h1 = std::hash<om::EventType>()(s.type);
            return h1;
        }
    };

    using eventSet = std::unordered_set<om::Event, HashEvent>;

    bool input(om::IEngine& engine, Model::World::WorldEvents*& retWorldEvents,
               eventSet& environementEvents);

    bool handleEnvironementEvents(om::IEngine& engine, eventSet envEvents);
    bool isPause() { return m_isPause; }
    [[nodiscard]] bool isReset()
    {
        auto returnValue = m_resetGame;
        m_resetGame      = false;
        return returnValue;
    }

private:
    std::vector<om::myGlfloat> mouseParams{ 0.0, 0.0, Global::initialScale };
    bool                       m_isPause{};
    bool                       m_resetGame{};
};
