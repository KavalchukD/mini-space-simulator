#pragma once
#include "animation2d.hpp"
#include "imgui_wrapper.hpp"
#include "render_objects.hpp"
#include "sprite.hpp"
#include "world.hpp"
#include <iengine.hpp>

#include <array>
#include <vector>

class RenderWrapper
{
public:
    RenderWrapper(om::IEngine&                 engine,
                  std::array<om::myGlfloat, 3> color    = { 0, 1, 0 },
                  om::myGlfloat                gridStep = 0.025);

    void render(const Model::World& world);
    void renderGameOver();

private:
    bool checkColor(std::array<om::myGlfloat, 3> color);
    bool checkStep(om::myGlfloat step);

    void renderWorld(const Model::World& world);

    om::myGlfloat                m_gridStep;
    std::array<om::myGlfloat, 3> m_color;

    om::Vector<2> m_currentShiftRelateToUser{};

    renderObjects::Rocket         m_rocket;
    renderObjects::PhysicalObject m_planet;
    renderObjects::PhysicalObject m_asteroid;
    renderObjects::PhysicalObject m_bullet;
    renderObjects::Star           m_star;
    renderObjects::Collisions     m_collisions;

    Sprite                         m_backgroundSprite;
    Sprite                         m_backgroundGameOverSprite;
    renderObjects::ParallaxNebulas m_parallaxNebula;

    om::ProgramId m_programIdShaderGrid;
    om::ProgramId m_programIdShaderMorph;
    om::ProgramId m_programIdTexturedMorphed;
    om::ProgramId m_programIdTexturedMoved;
    om::ProgramId m_programIdMorphedMoved;
    om::ProgramId m_programIdMoved;

    om::TextureId m_textureIdRocketMainCorpus;
    om::TextureId m_textureIdFireMainEngine;
    om::TextureId m_textureIdFireSideEngine;
    om::TextureId m_textureIdBackground;
    om::TextureId m_textureIdBackgroundGameOver;
    om::TextureId m_textureIdNebulas;
    om::TextureId m_textureIdTrailCloud;
    om::TextureId m_textureIdExplosion;
    om::TextureId m_textureIdHit;

    om::TextureId m_textureIdPlanet;
    om::TextureId m_textureIdStar;
    om::TextureId m_textureIdAsteroid;
    om::TextureId m_textureIdBullet;

    om::IEngine& m_engine;

    static constexpr std::string_view moveMatrixUniformName{ "u_move_matrix" };

    static const std::vector<std::string_view> textureAttributeNames;

    static const std::vector<std::pair<om::myUint, std::string_view>>
        vertexAttributePositions;

    static const std::vector<std::pair<om::myUint, std::string_view>>
        vertexMorphedAttributePositions;

    static const std::vector<std::pair<om::myUint, std::string_view>>
        vertexTexturedAttributePositions;

    static const std::vector<std::pair<om::myUint, std::string_view>>
        vertexWideAttributePositions;
};
