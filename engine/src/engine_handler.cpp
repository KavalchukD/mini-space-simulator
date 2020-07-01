#include "engine_handler.hpp"
#include "engine_sdl.hpp"
#include <iostream>
#include <stdexcept>

namespace om
{

std::weak_ptr<IEngine>
           EngineHandler::m_arrayEnginePtrs[m_numberOfEngineTypes]{};
std::mutex EngineHandler::m_engineHandlerMutex{};

EngineHandler::EngineHandler(EngineTypes             engineType,
                             const std::string_view& windowName,
                             const std::string_view& config)
{
    if (!initEnginePtr(engineType))
    {
        throw std::runtime_error("Cannot get handler to engine object.");
    }
    if (!initEngine(windowName, config))
    {
        throw std::runtime_error("Cannot init SDL2.");
    }
}

EngineHandler::~EngineHandler() noexcept
{
    const std::lock_guard guard(m_engineHandlerMutex);
    m_currentEnginePtr->uninitialize();
}

bool EngineHandler::initEnginePtr(EngineTypes engineType) noexcept
{
    const std::lock_guard guard(m_engineHandlerMutex);
    auto                  numEngineType = static_cast<size_t>(engineType);
    if (numEngineType > m_numberOfEngineTypes)
    {
        std::cerr << "Incorrect engine type." << std::endl;
        return false;
    }
    if (m_arrayEnginePtrs[numEngineType].expired())
    {
        switch (engineType)
        {
            case EngineTypes::sdl:
                m_currentEnginePtr = std::make_shared<EngineSdl>();
                break;
            default:
                std::cerr << "Cannot init engine. " << std::endl;
                return false;
        }
    }
    else
    {
        m_currentEnginePtr = m_arrayEnginePtrs[numEngineType].lock();
    }

    if (static_cast<bool>(m_currentEnginePtr))
    {
        m_arrayEnginePtrs[numEngineType] = m_currentEnginePtr;
    }
    return static_cast<bool>(m_currentEnginePtr);
}

bool EngineHandler::initEngine(const std::string_view& windowName,
                               const std::string_view& config) noexcept
{
    auto errStr = m_currentEnginePtr->initialize(windowName, config);
    if (!errStr.empty())
    {
        std::cerr << "Cannot init engine." << errStr << std::endl;
        return false;
    }
    return true;
}

} // end namespace om
