#include "environement.hpp"
#include <iostream>
#include <memory>
#include <sstream>

static bool isContiniousCommand(Model::World::Events worldEvent)
{
    using namespace Model;
    return (worldEvent == World::Events::userCommandShipUp ||
            worldEvent == World::Events::userCommandShipRotateLeft ||
            worldEvent == World::Events::userCommandShipRotateRight);
}

bool Environement::input(om::IEngine&                engine,
                         Model::World::WorldEvents*& retWorldEvents,
                         eventSet&                   environementEvents)
{
    using namespace Model;
    static auto worldEvents{ std::make_unique<Model::World::WorldEvents>(
        static_cast<size_t>(Model::World::Events::maxType)) };

    retWorldEvents = worldEvents.get();

    for (auto it = worldEvents->begin(); it != worldEvents->end();)
    {
        if (!isContiniousCommand(it->first))
            it = worldEvents->erase(it);
        else
            ++it;
    }
    om::Event event;
    while (engine.read_input(event))
    {
#ifdef DEBUG_CONFIGURATION
        std::cout << event.type << std::endl;
#endif
        switch (event.type)
        {
            case om::EventType::turn_off:
                return false;
            case om::EventType::up_pressed:
                worldEvents->insert({ World::Events::userCommandShipUp, {} });
                break;
            case om::EventType::up_released:
                worldEvents->erase(World::Events::userCommandShipUp);
                break;
            case om::EventType::down_pressed:
                //                worldEvents->insert({
                //                World::Events::userCommandShipDown, {} });
                break;
            case om::EventType::down_released:
                //                worldEvents->erase(World::Events::userCommandShipDown);
                break;
            case om::EventType::left_pressed:
                worldEvents->insert(
                    { World::Events::userCommandShipRotateLeft, {} });
                break;
            case om::EventType::left_released:
                worldEvents->erase(World::Events::userCommandShipRotateLeft);
                break;
            case om::EventType::right_pressed:
                worldEvents->insert(
                    { World::Events::userCommandShipRotateRight, {} });
                break;
            case om::EventType::right_released:
                worldEvents->erase(World::Events::userCommandShipRotateRight);
                break;
            case om::EventType::button1_pressed:
            {
                worldEvents->insert(
                    { World::Events::userCommandShipAttack, {} });
                //                const double xWorld =
                //                    (mouseParams[0]) /
                //                    Global::getCurrentWorldScaleX();

                //                const double yWorld =
                //                    (mouseParams[1]) /
                //                    Global::getCurrentWorldScaleY();
                //                worldEvents->insert(
                //                    {
                //                    World::Events::userCommandShipTeleporation,
                //                      { xWorld, yWorld } });
                //                std::cerr << "Oh, it`s imposible, man, you`ve
                //                just committed "
                //                             "a teleportation.\n";
                break;
            }
            case om::EventType::button2_pressed:
                environementEvents.insert(event);
                break;
            case om::EventType::cursor_motion:
                environementEvents.insert(event);
                break;
            case om::EventType::wheel_rolled:
                environementEvents.insert(event);
                break;
            case om::EventType::select_pressed:
                environementEvents.insert(event);
                break;
            case om::EventType::select_released:
                environementEvents.insert(event);
                break;
            case om::EventType::start_pressed:
                environementEvents.insert(event);
                break;
            default:
                break;
        }
    }
    return true;
}

bool Environement::handleEnvironementEvents(
    [[maybe_unused]] om::IEngine& engine, eventSet envEvents)
{
    std::stringstream debugStream{};
    for (const auto& event : envEvents)
    {
        switch (event.type)
        {
            case om::EventType::cursor_motion:
                mouseParams[0] = event.parameters.p0;
                mouseParams[1] = event.parameters.p1;
                debugStream << mouseParams[0] << " " << mouseParams[1] << "\n";
                // engine.setUniform("u_mouse", mouseParams, { 1 });
                break;
            case om::EventType::wheel_rolled:
                mouseParams[2] *= (1 + event.parameters.p1);
                mouseParams[2] += event.parameters.p1;
                if (mouseParams[2] > Global::maximalScale)
                {
                    mouseParams[2] = Global::maximalScale;
                }
                else if (mouseParams[2] < Global::minimalScale)
                {
                    mouseParams[2] = Global::minimalScale;
                }

                Global::currentWorldScale = mouseParams[2];
                debugStream << "Scaling: " << mouseParams[2] << "\n";
                break;
            case om::EventType::button2_pressed:
                mouseParams[2]            = Global::initialScale;
                Global::currentWorldScale = mouseParams[2];
                debugStream << "Reset to default scale: " << mouseParams[2]
                            << "\n";
                break;
            case om::EventType::select_pressed:
                m_isPause = !m_isPause;
                debugStream << "pause "
                            << "\n";
                break;
            case om::EventType::start_pressed:
                m_resetGame = true;
                debugStream << "Reset game "
                            << "\n";
                break;
            default:
                break;
        }
    }
#ifdef DEBUG_CONFIGURATION
    std::clog << debugStream.str();
#endif
    return true;
}
