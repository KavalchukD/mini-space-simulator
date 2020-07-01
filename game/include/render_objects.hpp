#pragma once
#include "animation2d.hpp"
#include "global.hpp"
#include "iengine.hpp"
#include "sprite.hpp"
#include "utilities.hpp"
#include "world.hpp"

namespace renderObjects
{

class PhysicalObject
{
public:
    PhysicalObject() = default;

    virtual ~PhysicalObject() {}

    PhysicalObject(const std::string_view textureAttribureName,
                   const std::string_view moveMatrixUniformName,
                   const om::ProgramId& programId, const om::TextureId& tex,
                   std::string_view id = "");

    const om::Vector<2>& getPos() const;
    void                 setPos(const om::Vector<2>& pos);
    om::myGlfloat        getAngle() const;
    void                 setAngle(om::myGlfloat angle);

    virtual void draw(om::IEngine& render, const Model::PhysicalObject& object);
    void         draw(om::IEngine& render, const Model::PhysicalObject& object,
                      om::myGlfloat scaleWorldRelateToRender);

protected:
    om::Vector<2>       m_pos{};
    om::myGlfloat       m_angle{};
    std::vector<Sprite> m_sprites{};
};

class Star : public PhysicalObject
{
public:
    Star() = default;
    ~Star() override {}
    Star(const std::string_view textureAttribureName,
         const std::string_view moveMatrixUniformName,
         const om::ProgramId& programId, const om::TextureId& tex);
    void draw(om::IEngine&                 render,
              const Model::PhysicalObject& object) override;

private:
    static constexpr om::myGlfloat widerNess = 2.5;
};

class RocketMainCorpus : public PhysicalObject
{
public:
    RocketMainCorpus() = default;
    ~RocketMainCorpus() override {}
    RocketMainCorpus(const std::string_view textureAttribureName,
                     const std::string_view moveMatrixUniformName,
                     const om::ProgramId& programId, const om::TextureId& tex);
};

class EngineFire
{
public:
    enum class Type
    {
        down,
        up,
        left,
        right
    };
    using Event  = Animation2D3Phase::Event;
    EngineFire() = default;

    EngineFire& operator=(const EngineFire& mainEngineFire2);

    EngineFire(const EngineFire& mainEngineFire2);

    EngineFire& operator=(const EngineFire&& mainEngineFire2);

    EngineFire(const EngineFire&& mainEngineFire2);

    EngineFire(const std::string_view textureAttribureName,
               const std::string_view moveMatrixUniformName,
               const om::ProgramId& programId, const om::TextureId& tex,
               Type type = Type::down);

    const om::Vector<2>& getPos() const;
    void                 setPos(const om::Vector<2>& pos);
    const om::Vector<2>& getSize() const;
    void                 setSize(const om::Vector<2>& size);
    om::myGlfloat        getAngle() const;
    void                 setAngle(om::myGlfloat angle);

    void draw(om::IEngine& render, const Model::RocketEngine& rocketEngine,
              om::Vector<2> basePoint = {}) const;

protected:
    om::Vector<2>       m_pos{};
    om::Vector<2>       m_size{};
    om::myGlfloat       m_angle{};
    std::vector<Sprite> m_spritesStart{};
    std::vector<Sprite> m_spritesRun{};
    std::vector<Sprite> m_spritesStop{};
    Animation2D3Phase   m_animation{};
    Type                m_type{};
};

class Collisions
{
public:
    Collisions() = default;

    Collisions& operator=(const Collisions& collisions2);

    Collisions(const Collisions& collisions2);

    Collisions& operator=(const Collisions&& collisions2);

    Collisions(const Collisions&& collisions2);

    Collisions(const std::string_view textureAttribureName,
               const std::string_view moveMatrixUniformName,
               const om::ProgramId&   programId,
               const om::TextureId& texExplosion, const om::TextureId& texHit);

    void addCollision(const Model::OutEvent& collision);
    void addCollisions(const std::list<Model::OutEvent>& collisions);
    void draw(om::IEngine& render, Timer::time_point_t nowTime);

private:
    struct AnimationDescriptor
    {
        Animation2D           animation;
        Timer::time_point_t   time;
        om::Vector<2>         pos;
        om::Vector<2>         size;
        Model::OutEvent::Type type;
    };

    std::list<AnimationDescriptor> collisionsAnimation;
    std::vector<Sprite>            m_spritesHit{};
    std::vector<Sprite>            m_spritesExplosion{};
};

class Rocket
{
public:
    Rocket() = default;
    Rocket(const std::string_view textureAttribureName,
           const std::string_view moveMatrixUniformName,
           const om::ProgramId& programId, const om::TextureId& texMainCorpus,
           const om::TextureId& texMainEngineFire,
           const om::TextureId& texSideEngineFire,
           const om::TextureId& texClouds);

    void draw(om::IEngine& render, const Model::Rocket& rocket);

    const om::Vector<2>& getPos() const;
    void                 setPos(const om::Vector<2>& pos);
    om::myGlfloat        getAngle() const;
    void                 setAngle(om::myGlfloat angle);
    void drawClouds(om::IEngine& render, const Model::Rocket& rocket);

protected:
    void drawEngineFire(om::IEngine& render, const Model::Rocket& rocket,
                        const Model::RocketEngine& rocketEngine,
                        EngineFire&                engineFire,
                        const om::Vector<2>&       engineFireRelativePos,
                        const om::Vector<2>&       engineFireRelativeSize);

    om::Vector<2>                m_pos{};
    om::Vector<2>                m_mainCorpusRelativePos{};
    om::Vector<2>                m_mainEngineFireRelativePos{};
    om::Vector<2>                m_mainEngineFireRelativeSize{};
    std::array<om::Vector<2>, 4> m_sideEnginesFireRelativePos{};
    std::array<om::Vector<2>, 4> m_sideEnginesFireRelativeSize{};
    om::myGlfloat                m_angle{};
    RocketMainCorpus             mainCorpus{};
    EngineFire                   mainEngineFire{};
    std::array<EngineFire, 4>    sideEnginesFire{};
    Sprite                       trailCloud{};
};

// class MainBackground
//{
// public:
//    MainBackground() = default;

//    MainBackground(const std::string_view textureAttribureName,
//                   const std::string_view moveMatrixUniformName,
//                   const om::ProgramId& programId, const om::TextureId& tex);

//    const om::Vector<2>& getPos() const;
//    void                 setPos(const om::Vector<2>& pos);
//    om::myGlfloat        getAngle() const;
//    void                 setAngle(om::myGlfloat angle);

//    void draw(om::IEngine& render);

// private:
//    om::Vector<2>       m_pos{};
//    om::myGlfloat       m_angle{};
//    std::vector<Sprite> m_sprites{};
//};

class ParallaxNebulas
{
public:
    ParallaxNebulas() = default;

    ParallaxNebulas(const std::string_view textureAttribureName,
                    const std::string_view moveMatrixUniformName,
                    const om::ProgramId& programId, const om::TextureId& tex,
                    om::myGlfloat parallaxCoef);

    om::myGlfloat getAngle() const;
    void          setAngle(om::myGlfloat angle);

    void draw(om::IEngine& render);

private:
    void makeWiderParallax();
    void formGroupOfSprites(
        const std::vector<size_t>&  indices,
        std::vector<Sprite>&        spritesQuadrant,
        std::vector<om::Vector<2>>& spriteBasePositionsQuadrant,
        std::vector<om::Vector<2>>& spriteBaseSizeQuadrant);
    void shiftAndAddToContainerSprites(
        const std::vector<Sprite>&        spritesGroup,
        const std::vector<om::Vector<2>>& spritesPosGroup,
        const std::vector<om::Vector<2>>& spritesSizeGroup,
        const om::Vector<2>&              shift);
    std::vector<om::Vector<2>>     m_spriteBasePositions{};
    std::vector<om::Vector<2>>     m_spriteBaseSize{};
    om::myGlfloat                  m_angle{};
    std::vector<Sprite>            m_sprites{};
    om::myGlfloat                  m_parallaxCoef{ 1.f };
    om::myGlfloat                  sizeY{ defaultSizeY };
    om::myGlfloat                  sizeX{ defaultSizeX };
    static constexpr om::myGlfloat defaultSizeY{ 6.0 };
    static constexpr om::myGlfloat defaultSizeX{ defaultSizeY *
                                                 Global::baseScaleXtoY };
};

} // namespace renderObjects
