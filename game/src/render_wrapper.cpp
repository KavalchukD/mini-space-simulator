#include "render_wrapper.hpp"
#include "global.hpp"
#include "utilities.hpp"

#include <algorithm>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

const std::vector<std::pair<om::myUint, std::string_view>>
    RenderWrapper::vertexAttributePositions{
        { om::Vertex::positionAttributeNumber, "a_position" },
        { om::Vertex::colorAttributeNumber, "a_color" }
    };

const std::vector<std::pair<om::myUint, std::string_view>>
    RenderWrapper::vertexMorphedAttributePositions{
        { om::VertexMorphed::positionAttributeNumber, "a_position1" },
        { om::VertexMorphed::colorAttributeNumber, "a_color" },
        { om::VertexMorphed::position2AttributeNumber, "a_position2" },
    };

const std::vector<std::pair<om::myUint, std::string_view>>
    RenderWrapper::vertexTexturedAttributePositions{
        { om::VertexTextured::positionAttributeNumber, "a_position" },
        { om::VertexTextured::colorAttributeNumber, "a_color" },
        { om::VertexTextured::positionTexAttributeNumber, "a_tex_position" },
    };

const std::vector<std::pair<om::myUint, std::string_view>>
    RenderWrapper::vertexWideAttributePositions{
        { om::VertexTextured::positionAttributeNumber, "a_position1" },
        { om::VertexTextured::colorAttributeNumber, "a_color" },
        { om::VertexMorphed::position2AttributeNumber, "a_position2" },
        { om::VertexTextured::positionTexAttributeNumber, "a_tex_position" },
    };

const std::vector<std::string_view> RenderWrapper::textureAttributeNames{
    "s_texture"
};

using namespace renderObjects;

RenderWrapper::RenderWrapper(om::IEngine&                 engine,
                             std::array<om::myGlfloat, 3> color,
                             om::myGlfloat                gridStep)
    : m_engine{ engine }
{
    m_programIdTexturedMoved =
        m_engine.addProgram("res/shaders/game_vertex_shader.vert",
                            "res/shaders/game_fragment_shader.frag",
                            vertexTexturedAttributePositions);

    m_textureIdBackground =
        m_engine.addTexture("res/textures/background_nasa_photo.png");
    m_textureIdBackgroundGameOver =
        m_engine.addTexture("res/textures/background_gameover.png");
    m_textureIdNebulas =
        m_engine.addTexture("res/textures/proc_sheet_nebula_transp.png");

    m_textureIdRocketMainCorpus =
        m_engine.addTexture("res/textures/topdownfighter.png");
    m_textureIdFireMainEngine =
        m_engine.addTexture("res/textures/flame_fire.png");
    m_textureIdFireSideEngine =
        m_engine.addTexture("res/textures/flame_blueish_flame.png");
    m_textureIdTrailCloud = m_engine.addTexture("res/textures/trail_cloud.png");

    m_textureIdPlanet   = m_engine.addTexture("res/textures/planet.png");
    m_textureIdStar     = m_engine.addTexture("res/textures/blue_star.png");
    m_textureIdAsteroid = m_engine.addTexture("res/textures/asteroid.png");
    m_textureIdBullet   = m_engine.addTexture("res/textures/bullet.png");

    m_textureIdExplosion = m_engine.addTexture("res/textures/explosion.png");
    m_textureIdHit       = m_engine.addTexture("res/textures/hit.png");

    m_backgroundSprite =
        Sprite("backgorund", textureAttributeNames[0], moveMatrixUniformName,
               m_programIdTexturedMoved, m_textureIdBackground,
               { { 0.f, 0.f }, { 1.f, 1.f } },
               { { 0.f, 0.f }, { 2.f * Global::baseScaleXtoY, 2.f } }, 0,
               { 1.f, 1.f, 1.f, 1.f });

    m_backgroundGameOverSprite =
        Sprite("backgorundGameOver", textureAttributeNames[0],
               moveMatrixUniformName, m_programIdTexturedMoved,
               m_textureIdBackgroundGameOver, { { 0.f, 0.f }, { 1.f, 1.f } },
               { { 0.f, 0.f }, { 2.f * Global::baseScaleXtoY, 2.f } }, 0,
               { 1.f, 1.f, 1.f, 1.f });

    m_parallaxNebula = renderObjects::ParallaxNebulas(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdNebulas, 0.33f);

    m_rocket = renderObjects::Rocket(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdRocketMainCorpus,
        m_textureIdFireMainEngine, m_textureIdFireSideEngine,
        m_textureIdTrailCloud);

    m_asteroid = renderObjects::PhysicalObject(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdAsteroid);

    m_planet = renderObjects::PhysicalObject(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdPlanet);

    m_bullet = renderObjects::PhysicalObject(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdBullet);

    m_star =
        renderObjects::Star(textureAttributeNames[0], moveMatrixUniformName,
                            m_programIdTexturedMoved, m_textureIdStar);

    m_collisions = renderObjects::Collisions(
        textureAttributeNames[0], moveMatrixUniformName,
        m_programIdTexturedMoved, m_textureIdExplosion, m_textureIdHit);

    m_engine.setCurrentDefaultProgram(m_programIdTexturedMoved);

    m_color    = color;
    m_gridStep = gridStep;
}

bool RenderWrapper::checkColor(std::array<om::myGlfloat, 3> color)
{
    return std::none_of(
        color.begin(), color.end(), [](om::myGlfloat colorComponent) {
            return (colorComponent > 1.f || colorComponent < 0.f);
        });
}

void RenderWrapper::render(const Model::World& world)
{
    m_backgroundSprite.draw(m_engine);

    m_parallaxNebula.draw(m_engine);

    renderWorld(world);
}

void RenderWrapper::renderWorld(const Model::World& world)
{
    m_collisions.addCollisions(world.outEvents);

    for (const auto& currentRocket : world.rockets)
    {
        m_rocket.draw(m_engine, currentRocket);
    }

    for (const auto& planet : world.planets)
    {
        m_planet.draw(m_engine, planet);
    }

    for (const auto& asteroid : world.asteroids)
    {
        m_asteroid.draw(m_engine, asteroid);
    }

    for (const auto& currentBullet : world.bullets)
    {
        m_bullet.draw(m_engine, currentBullet);
    }

    for (const auto& currentRocket : world.rockets)
    {
        m_rocket.drawClouds(m_engine, currentRocket);
    }

    for (const auto& star : world.stars)
    {
        m_star.draw(m_engine, star);
    }
    m_collisions.draw(m_engine, world.lastUpdateTime);
}

void RenderWrapper::renderGameOver()
{
    m_backgroundGameOverSprite.draw(m_engine);
}
