#pragma once
#include "iengine.hpp"
#include <memory>
#include <mutex>

namespace om
{
class OM_DECLSPEC EngineHandler
{
public:
    using EngineTypes = IEngine::EngineTypes;
    EngineHandler(EngineTypes             engineType,
                  const std::string_view& windowName = "title",
                  const std::string_view& config     = "");
    ~EngineHandler() noexcept;
    IEngine* operator->() const noexcept { return m_currentEnginePtr.get(); }
    IEngine* get() const noexcept { return m_currentEnginePtr.get(); }
    IEngine& operator*() const noexcept { return *m_currentEnginePtr; }

private:
    bool                    initEnginePtr(EngineTypes engineType) noexcept;
    bool                    initEngine(const std::string_view& windowName,
                                       const std::string_view& config) noexcept;
    static constexpr size_t m_numberOfEngineTypes =
        static_cast<size_t>(EngineTypes::max_types);
    std::shared_ptr<IEngine>      m_currentEnginePtr{};
    static std::weak_ptr<IEngine> m_arrayEnginePtrs[m_numberOfEngineTypes];
    static std::mutex             m_engineHandlerMutex;
};

} // end namespace om
