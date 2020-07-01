#pragma once
#include "world.hpp"
#include <iengine.hpp>

class ImguiWrapper
{
public:
    explicit ImguiWrapper(om::IEngine& engine);
    void createImguiObjects(Model::World& world);

private:
    om::IEngine& m_engine;
};
