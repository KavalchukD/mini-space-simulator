#include "render_objects.hpp"
#include "global.hpp"
#include "utilities.hpp"
#include <stdexcept>

namespace renderObjects
{

static std::vector<Sprite> loadGridSprites(
    const std::string_view textureAttribureName,
    const std::string_view moveMatrixUniformName,
    const om::ProgramId& programId, const om::TextureId& tex,
    const om::myGlfloat numberOfLines, const om::myGlfloat numberOfColumns,
    const std::string& decriptionLine = "sprite")
{

    const om::myGlfloat textureStepX   = 1.0 / numberOfColumns;
    const om::myGlfloat textureStepY   = 1.0 / numberOfLines;
    const om::myGlfloat renderBaseSize = 0.2;

    const om::Vector<2> textureSpriteSize{ textureStepX, textureStepY };

    std::vector<Sprite> allSprites;
    allSprites.reserve(numberOfLines * numberOfColumns);
    {
        om::myGlfloat textureXCurent = 0.0;
        om::myGlfloat textureYCurent = 1.0 - textureStepY;

        for (int line = 0; line < numberOfLines; ++line)
        {
            for (int column = 0; column < numberOfColumns; ++column)
            {
                allSprites.push_back(
                    { decriptionLine + " " + std::to_string(line) + " " +
                          std::to_string(column),
                      textureAttribureName,
                      moveMatrixUniformName,
                      programId,
                      tex,
                      { { textureXCurent, textureYCurent }, textureSpriteSize },
                      { { 0, 0 }, { renderBaseSize, renderBaseSize } },
                      0.f });
                textureXCurent += textureStepX;
            }
            textureXCurent = 0.0;
            textureYCurent -= textureStepY;
        }
    }
    return allSprites;
}

PhysicalObject::PhysicalObject(const std::string_view textureAttribureName,
                               const std::string_view moveMatrixUniformName,
                               const om::ProgramId&   programId,
                               const om::TextureId& tex, std::string_view id)
{
    const om::Vector<2> textureSpritePos{ 0, 0 };

    const om::Vector<2> textureSpriteSize{ 1, 1 };

    m_sprites.push_back({ id,
                          textureAttribureName,
                          moveMatrixUniformName,
                          programId,
                          tex,
                          { textureSpritePos, textureSpriteSize },
                          { { 0, 0 }, { 1.0, 1.0 } },
                          0.f });
}

const om::Vector<2>& PhysicalObject::getPos() const
{
    return m_pos;
}

void PhysicalObject::setPos(const om::Vector<2>& pos)
{
    m_pos = pos;
}

om::myGlfloat PhysicalObject::getAngle() const
{
    return m_angle;
}

void PhysicalObject::setAngle(om::myGlfloat angle)
{
    m_angle = angle;
}

void PhysicalObject::draw(om::IEngine&                 render,
                          const Model::PhysicalObject& object)
{
    draw(render, object, 1.0);
}

void PhysicalObject::draw(om::IEngine&                 render,
                          const Model::PhysicalObject& object,
                          om::myGlfloat                scaleWorldRelateToRender)
{
    auto rocketNdcX =
        static_cast<om::myGlfloat>(object.x) * Global::getCurrentWorldScaleY();

    auto rocketNdcY =
        static_cast<om::myGlfloat>(object.y) * Global::getCurrentWorldScaleY();

    const auto userPosition = Global::getUserNdcPosition();

    rocketNdcX -= userPosition.elements[0];
    rocketNdcY -= userPosition.elements[1];

    const auto rocketSizeX =
        static_cast<float>(object.width * Global::getCurrentWorldScaleY());

    const auto rocketSizeY =
        static_cast<float>(object.height * Global::getCurrentWorldScaleY());

    m_sprites[0].setSpriteSize({ rocketSizeX * scaleWorldRelateToRender,
                                 rocketSizeY * scaleWorldRelateToRender });
    m_sprites[0].setSpritePos({ rocketNdcX, rocketNdcY });

    m_sprites[0].setAngle(object.angle);
    m_sprites[0].draw(render);
}

///////////////////////////////////////////////////////////////////////////////
Star::Star(const std::string_view textureAttribureName,
           const std::string_view moveMatrixUniformName,
           const om::ProgramId& programId, const om::TextureId& tex)
    : PhysicalObject(textureAttribureName, moveMatrixUniformName, programId,
                     tex, "star")
{
}
void Star::draw(om::IEngine& render, const Model::PhysicalObject& object)
{
    PhysicalObject::draw(render, object, widerNess);
}

RocketMainCorpus::RocketMainCorpus(const std::string_view textureAttribureName,
                                   const std::string_view moveMatrixUniformName,
                                   const om::ProgramId&   programId,
                                   const om::TextureId&   tex)
    : PhysicalObject(textureAttribureName, moveMatrixUniformName, programId,
                     tex, "rocket_corpus")
{
    // TODO! 16/9 screen should be expected
    const om::Vector<2> texturePixcelSize{
        457,
        1024,
    };

    const om::Vector<2> textureSpritePos{
        17 / texturePixcelSize.elements[0],
        28 / texturePixcelSize.elements[1],
    };

    const om::Vector<2> textureSpriteSize{
        1 - textureSpritePos.elements[0] * 2,
        1 - textureSpritePos.elements[1] - 24 / texturePixcelSize.elements[1]
    };

    const auto rocketSizeX = static_cast<float>(
        Model::Rocket::defaultWidth * Global::getCurrentWorldScaleY());

    const auto rocketSizeY = static_cast<float>(
        Model::Rocket::defaultHeight * Global::getCurrentWorldScaleY());

    m_sprites[0].setTextureCoord({ textureSpritePos, textureSpriteSize });
    m_sprites[0].setSpriteSize({ rocketSizeX, rocketSizeY });
}

///////////////////////////////////////////////////////////////////////////////

EngineFire::EngineFire(const std::string_view textureAttribureName,
                       const std::string_view moveMatrixUniformName,
                       const om::ProgramId& programId, const om::TextureId& tex,
                       Type type)
    : m_type{ type }
{
    const auto textureNumberOfLines   = 5;
    const auto textureNumberOfColumns = 6;

    std::vector<Sprite> allSprites = loadGridSprites(
        textureAttribureName, moveMatrixUniformName, programId, tex,
        textureNumberOfLines, textureNumberOfColumns, "engine_fire");

    om::myGlfloat angle;

    switch (m_type)
    {
        case Type::up:
            angle = 0;
            break;
        case Type::down:
            angle = M_PI;
            break;
        case Type::left:
            angle = M_PI_2;
            break;
        case Type::right:
            angle = -M_PI_2;
            break;
        default:
            throw std::runtime_error(
                "Incorrect type of renderObject engineFire");
    }

    std::for_each_n(allSprites.begin(), allSprites.size(),
                    [angle](Sprite& sprite) { sprite.setBaseAngle(angle); });

    std::copy_n(allSprites.begin(), 9, std::back_inserter(m_spritesStart));
    std::copy_n(&allSprites[10], 10, std::back_inserter(m_spritesRun));
    std::copy_n(&allSprites[20], 10, std::back_inserter(m_spritesStop));

    m_animation.sprites(m_spritesStart, m_spritesRun, m_spritesStop);
    m_animation.setFps(20);
}

EngineFire& EngineFire::operator=(const EngineFire& mainEngineFire2)
{
    this->m_pos   = mainEngineFire2.m_pos;
    this->m_angle = mainEngineFire2.m_angle;
    this->m_type  = mainEngineFire2.m_type;

    this->m_spritesStart = mainEngineFire2.m_spritesStart;
    this->m_spritesRun   = mainEngineFire2.m_spritesRun;
    this->m_spritesStop  = mainEngineFire2.m_spritesStop;

    this->m_animation = mainEngineFire2.m_animation;
    this->m_animation.sprites(this->m_spritesStart, this->m_spritesRun,
                              this->m_spritesStop);
    return *this;
}

EngineFire::EngineFire(const EngineFire& mainEngineFire2)
{
    this->m_pos   = mainEngineFire2.m_pos;
    this->m_angle = mainEngineFire2.m_angle;
    this->m_type  = mainEngineFire2.m_type;

    this->m_spritesStart = mainEngineFire2.m_spritesStart;
    this->m_spritesRun   = mainEngineFire2.m_spritesRun;
    this->m_spritesStop  = mainEngineFire2.m_spritesStop;

    this->m_animation = mainEngineFire2.m_animation;
    this->m_animation.sprites(this->m_spritesStart, this->m_spritesRun,
                              this->m_spritesStop);
}

EngineFire& EngineFire::operator=(const EngineFire&& mainEngineFire2)
{
    this->m_pos   = mainEngineFire2.m_pos;
    this->m_angle = mainEngineFire2.m_angle;
    this->m_type  = mainEngineFire2.m_type;

    this->m_spritesStart = std::move(mainEngineFire2.m_spritesStart);
    this->m_spritesRun   = std::move(mainEngineFire2.m_spritesRun);
    this->m_spritesStop  = std::move(mainEngineFire2.m_spritesStop);

    this->m_animation = mainEngineFire2.m_animation;
    this->m_animation.sprites(this->m_spritesStart, this->m_spritesRun,
                              this->m_spritesStop);
    return *this;
}

EngineFire::EngineFire(const EngineFire&& mainEngineFire2)
{
    this->m_pos   = mainEngineFire2.m_pos;
    this->m_angle = mainEngineFire2.m_angle;
    this->m_type  = mainEngineFire2.m_type;

    this->m_spritesStart = std::move(mainEngineFire2.m_spritesStart);
    this->m_spritesRun   = std::move(mainEngineFire2.m_spritesRun);
    this->m_spritesStop  = std::move(mainEngineFire2.m_spritesStop);

    this->m_animation = mainEngineFire2.m_animation;
    this->m_animation.sprites(this->m_spritesStart, this->m_spritesRun,
                              this->m_spritesStop);
}

const om::Vector<2>& EngineFire::getPos() const
{
    return m_pos;
}

void EngineFire::setPos(const om::Vector<2>& pos)
{
    m_pos = pos;
}

const om::Vector<2>& EngineFire::getSize() const
{
    return m_size;
}
void EngineFire::setSize(const om::Vector<2>& size)
{
    m_size = size;
}

om::myGlfloat EngineFire::getAngle() const
{
    return m_angle;
}

void EngineFire::setAngle(om::myGlfloat angle)
{
    m_angle = angle;
}

void EngineFire::draw(om::IEngine&               render,
                      const Model::RocketEngine& rocketEngine,
                      om::Vector<2>              basePoint) const
{
    EngineFire::Event eventEngine = (rocketEngine.getEngineState().getState())
                                        ? EngineFire::Event::Run
                                        : EngineFire::Event::Stop;
    const auto engineEventTime = rocketEngine.getEngineState().getTime();

    auto currentSprite =
        m_animation.getCurrentSprite(eventEngine, engineEventTime);

    if (!currentSprite)
    {
        return;
    }
    currentSprite->setSpriteSize(m_size);
    currentSprite->setSpritePos(m_pos);
    currentSprite->setAngle(m_angle);
    currentSprite->draw(render, basePoint);
}

///////////////////////////////////////////////////////////////////////////////

Collisions::Collisions(const std::string_view textureAttribureName,
                       const std::string_view moveMatrixUniformName,
                       const om::ProgramId&   programId,
                       const om::TextureId&   texExplosion,
                       const om::TextureId&   texHit)
{
    const auto textureNumberOfLinesExplosion   = 5;
    const auto textureNumberOfColumnsExplosion = 8;

    m_spritesExplosion =
        loadGridSprites(textureAttribureName, moveMatrixUniformName, programId,
                        texExplosion, textureNumberOfLinesExplosion,
                        textureNumberOfColumnsExplosion, "collision");

    const auto textureNumberOfLinesHit   = 5;
    const auto textureNumberOfColumnsHit = 6;

    m_spritesHit = loadGridSprites(textureAttribureName, moveMatrixUniformName,
                                   programId, texHit, textureNumberOfLinesHit,
                                   textureNumberOfColumnsHit, "collision");
}

Collisions& Collisions::operator=(const Collisions& collisions2)
{
    this->m_spritesHit       = collisions2.m_spritesHit;
    this->m_spritesExplosion = collisions2.m_spritesExplosion;

    this->collisionsAnimation = collisions2.collisionsAnimation;

    std::for_each(this->collisionsAnimation.begin(),
                  this->collisionsAnimation.end(),
                  [this](AnimationDescriptor& animation) {
                      if (animation.type == Model::OutEvent::Type::hit)
                          animation.animation.sprites(this->m_spritesHit);
                      else
                          animation.animation.sprites(this->m_spritesExplosion);
                  });
    return *this;
}

Collisions::Collisions(const Collisions& collisions2)
{
    this->m_spritesHit       = collisions2.m_spritesHit;
    this->m_spritesExplosion = collisions2.m_spritesExplosion;

    this->collisionsAnimation = collisions2.collisionsAnimation;

    std::for_each(this->collisionsAnimation.begin(),
                  this->collisionsAnimation.end(),
                  [this](AnimationDescriptor& animation) {
                      if (animation.type == Model::OutEvent::Type::hit)
                          animation.animation.sprites(this->m_spritesHit);
                      else
                          animation.animation.sprites(this->m_spritesExplosion);
                  });
}

Collisions& Collisions::operator=(const Collisions&& collisions2)
{
    this->m_spritesHit       = std::move(collisions2.m_spritesHit);
    this->m_spritesExplosion = std::move(collisions2.m_spritesExplosion);

    this->collisionsAnimation = collisions2.collisionsAnimation;

    std::for_each(this->collisionsAnimation.begin(),
                  this->collisionsAnimation.end(),
                  [this](AnimationDescriptor& animation) {
                      if (animation.type == Model::OutEvent::Type::hit)
                          animation.animation.sprites(this->m_spritesHit);
                      else
                          animation.animation.sprites(this->m_spritesExplosion);
                  });
    return *this;
}

Collisions::Collisions(const Collisions&& collisions2)
{
    this->m_spritesHit       = std::move(collisions2.m_spritesHit);
    this->m_spritesExplosion = std::move(collisions2.m_spritesExplosion);

    this->collisionsAnimation = collisions2.collisionsAnimation;

    std::for_each(this->collisionsAnimation.begin(),
                  this->collisionsAnimation.end(),
                  [this](AnimationDescriptor& animation) {
                      if (animation.type == Model::OutEvent::Type::hit)
                          animation.animation.sprites(this->m_spritesHit);
                      else
                          animation.animation.sprites(this->m_spritesExplosion);
                  });
}

void Collisions::addCollision(const Model::OutEvent& collision)
{
    auto xCollision = static_cast<om::myGlfloat>(collision.x);
    auto yCollision = static_cast<om::myGlfloat>(collision.y);

    auto sizeXCollision = static_cast<om::myGlfloat>(collision.sizeX);
    auto sizeYCollision = static_cast<om::myGlfloat>(collision.sizeY);

    collisionsAnimation.push_back({ {},
                                    { collision.time },
                                    { xCollision, yCollision },
                                    { sizeXCollision, sizeYCollision },
                                    collision.type });

    if (collision.type == Model::OutEvent::Type::hit)
    {
        collisionsAnimation.back().animation.sprites(m_spritesHit);
    }
    else
    {
        collisionsAnimation.back().animation.sprites(m_spritesExplosion);
    }
    collisionsAnimation.back().animation.setFps(20);
    collisionsAnimation.back().animation.setMode(Animation2D::Mode::once);
}

void Collisions::addCollisions(const std::list<Model::OutEvent>& collisions)
{
    std::for_each(collisions.begin(), collisions.end(),
                  [this](const Model::OutEvent& collision) {
                      if ((collision.type == Model::OutEvent::Type::hit) ||
                          (collision.type == Model::OutEvent::Type::explosion))
                          this->addCollision(collision);
                  });
}

void Collisions::draw(om::IEngine& render, Timer::time_point_t nowTime)
{
    collisionsAnimation.remove_if(
        [nowTime](const AnimationDescriptor& currentAnimation) {
            const auto time = nowTime - currentAnimation.time;
            return (currentAnimation.animation.getCurrentSprite(time) ==
                    nullptr);
        });

    const auto userPosition = Global::getUserWorldPosition();
    auto       userPositionVector =
        om::Vector<2>{ static_cast<om::myGlfloat>(userPosition[0]),
                       static_cast<om::myGlfloat>(userPosition[1]) };

    for (auto& collisionAnimation : collisionsAnimation)
    {
        const auto time = nowTime - collisionAnimation.time;
        auto       currentSprite =
            collisionAnimation.animation.getCurrentSprite(time);
        const auto position = (collisionAnimation.pos - userPositionVector) *
                              Global::getCurrentWorldScaleForRender();
        currentSprite->setSpritePos(position);
        const auto size =
            (collisionAnimation.size) * Global::getCurrentWorldScaleForRender();
        currentSprite->setSpriteSize(size);
        currentSprite->draw(render);
    }
}

///////////////////////////////////////////////////////////////////////////////

Rocket::Rocket(const std::string_view textureAttribureName,
               const std::string_view moveMatrixUniformName,
               const om::ProgramId&   programId,
               const om::TextureId&   texMainCorpus,
               const om::TextureId&   texMainEngineFire,
               const om::TextureId&   texSideEngineFire,
               const om::TextureId&   texClouds)
    : m_mainCorpusRelativePos{ 0.f, 0.f }

    , m_mainEngineFireRelativePos{ 0.f, 0.40f }
    , m_mainEngineFireRelativeSize{ 1.0f, 1.0f }

    // engine 1 - left side down, engine 2 - left side uppper, engine 3 - rigth
    // side down, engine 4 - rigth side upper

    , m_sideEnginesFireRelativePos{ { { 0.47f, 0.16f },
                                      { 0.5f, -0.1f },
                                      { -0.47f, 0.16f },
                                      { -0.5f, -0.1f } } }
    , m_sideEnginesFireRelativeSize{ { { 0.5f, 0.4f },
                                       { 0.5f, 0.4f },
                                       { 0.5f, 0.4f },
                                       { 0.5f, 0.4f } } }

    , mainCorpus{ textureAttribureName, moveMatrixUniformName, programId,
                  texMainCorpus }
    , mainEngineFire{ textureAttribureName, moveMatrixUniformName, programId,
                      texMainEngineFire, EngineFire::Type::down }
    , sideEnginesFire{ { { textureAttribureName, moveMatrixUniformName,
                           programId, texSideEngineFire,
                           EngineFire::Type::left },
                         { textureAttribureName, moveMatrixUniformName,
                           programId, texSideEngineFire,
                           EngineFire::Type::left },
                         { textureAttribureName, moveMatrixUniformName,
                           programId, texSideEngineFire,
                           EngineFire::Type::right },
                         { textureAttribureName, moveMatrixUniformName,
                           programId, texSideEngineFire,
                           EngineFire::Type::right } } }
    , trailCloud{
        "cloud",
        textureAttribureName,
        moveMatrixUniformName,
        programId,
        texClouds,
        { { 0, 0 }, { 1, 1 } },
        { { 0, 0 },
          { static_cast<om::myGlfloat>(Model::TrailCloud::widthDefault) *
                Global::getCurrentScale(),
            static_cast<om::myGlfloat>(Model::TrailCloud::heightDefault) *
                Global::getCurrentScale() } },
        0
    }
{
}

const om::Vector<2>& Rocket::getPos() const
{
    return m_pos;
}
void Rocket::setPos(const om::Vector<2>& pos)
{
    m_pos = pos;
}
om::myGlfloat Rocket::getAngle() const
{
    return m_angle;
}
void Rocket::setAngle(om::myGlfloat angle)
{
    m_angle = angle;
}

void Rocket::drawEngineFire(om::IEngine& render, const Model::Rocket& rocket,
                            const Model::RocketEngine& rocketEngine,
                            EngineFire&                engineFire,
                            const om::Vector<2>&       engineFireRelativePos,
                            const om::Vector<2>&       engineFireRelativeSize)
{
    auto ndcXShip =
        static_cast<om::myGlfloat>(rocket.x * Global::getCurrentWorldScaleY());
    auto ndcYShip =
        static_cast<om::myGlfloat>(rocket.y * Global::getCurrentWorldScaleY());

    const auto userPosition = Global::getUserNdcPosition();

    ndcXShip -= userPosition.elements[0];
    ndcYShip -= userPosition.elements[1];

    const om::Vector<2> ndcCoordShip{ ndcXShip, ndcYShip };

    const auto rocketNdcSizeX = static_cast<om::myGlfloat>(
        rocket.width * Global::getCurrentWorldScaleForRender());

    const auto rocketNdcSizeY = static_cast<om::myGlfloat>(
        rocket.height * Global::getCurrentWorldScaleForRender());

    const om::Vector<2> rocketNdcSize{ rocketNdcSizeX, rocketNdcSizeY };

    const om::Vector<2> engineFireNdcSize =
        engineFireRelativeSize * rocketNdcSize *
        static_cast<om::myGlfloat>(rocketEngine.enginePercentThrust / 100.0);

    const auto ndcEngineFireShift =
        engineFireRelativePos * (engineFireNdcSize + rocketNdcSize);

    const auto ndcEngineFire = ndcCoordShip - ndcEngineFireShift;

    engineFire.setAngle(rocket.angle);

    engineFire.setSize(engineFireNdcSize);

    engineFire.setPos(ndcEngineFire);

    engineFire.draw(render, rocketEngine, ndcEngineFireShift);
}

void Rocket::draw(om::IEngine& render, const Model::Rocket& rocket)
{
    drawEngineFire(render, rocket, rocket.mainEngine, mainEngineFire,
                   m_mainEngineFireRelativePos, m_mainEngineFireRelativeSize);

    for (size_t i = 0; i < sideEnginesFire.size(); ++i)
    {
        drawEngineFire(render, rocket, rocket.sideEngines[i],
                       sideEnginesFire[i], m_sideEnginesFireRelativePos[i],
                       m_sideEnginesFireRelativeSize[i]);
    }

    mainCorpus.draw(render, rocket);
}

void Rocket::drawClouds(om::IEngine& render, const Model::Rocket& rocket)
{
    const auto userPosition      = Global::getUserNdcPosition();
    const auto currentWorldScale = Global::getCurrentWorldScaleForRender();

    for (const auto& cloud : rocket.clouds)
    {
        auto ndcXCloud =
            static_cast<om::myGlfloat>(cloud.x * currentWorldScale);
        auto ndcYCloud =
            static_cast<om::myGlfloat>(cloud.y * currentWorldScale);
        ndcXCloud -= userPosition.elements[0];
        ndcYCloud -= userPosition.elements[1];

        trailCloud.setSpritePos({ ndcXCloud, ndcYCloud });
        trailCloud.setAngle(cloud.angle);
        trailCloud.setSpriteSize(
            { static_cast<om::myGlfloat>(cloud.width) * currentWorldScale,
              static_cast<om::myGlfloat>(cloud.height) * currentWorldScale });
        const auto powerOfCloud =
            static_cast<om::myGlfloat>(cloud.getCurrentPower());
        trailCloud.setMixColor({ 1, 1, 1, powerOfCloud });
        trailCloud.draw(render);
    }
}

// MainBackground::MainBackground(const std::string_view
// textureAttribureName,
//                               const std::string_view
//                               moveMatrixUniformName, const om::ProgramId&
//                               programId, const om::TextureId&   tex)
//{
//    // TODO! 16/9 screen should be expected
//    const om::Vector<2> texturePixcelSize{
//        457,
//        1024,
//    };

//    const om::Vector<2> textureSpritePos{
//        17 / texturePixcelSize.elements[0],
//        28 / texturePixcelSize.elements[1],
//    };

//    const om::Vector<2> textureSpriteSize{
//        1 - textureSpritePos.elements[0] * 2,
//        1 - textureSpritePos.elements[1] - 24 /
//        texturePixcelSize.elements[1]
//    };

//    const auto rocketSizeX = static_cast<float>(
//        Model::Rocket::width * Global::getCurrentWorldScaleY());

//    const auto rocketSizeY = static_cast<float>(
//        Model::Rocket::height * Global::getCurrentWorldScaleY());

//    m_sprites.push_back({ std::string{ "rocket_corpus" },
//                          textureAttribureName,
//                          moveMatrixUniformName,
//                          programId,
//                          tex,
//                          { textureSpritePos, textureSpriteSize },
//                          { { 0, 0 }, { rocketSizeX, rocketSizeY } },
//                          0.f });
//}

// const om::Vector<2>& MainBackground::getPos() const
//{
//    return m_pos;
//}
// void MainBackground::setPos(const om::Vector<2>& pos)
//{
//    m_pos = pos;
//}
// om::myGlfloat MainBackground::getAngle() const
//{
//    return m_angle;
//}
// void MainBackground::setAngle(om::myGlfloat angle)
//{
//    m_angle = angle;
//}

// void MainBackground::draw(om::IEngine& render)
//{
//    const auto rocketNdcX =
//        static_cast<om::myGlfloat>(rocket.x) *
//        Global::getCurrentWorldScaleY();

//    const auto rocketNdcY =
//        static_cast<om::myGlfloat>(rocket.y) *
//        Global::getCurrentWorldScaleY();

//    const auto rocketSizeX = static_cast<float>(
//        Model::Rocket::width * Global::getCurrentWorldScaleY());

//    const auto rocketSizeY = static_cast<float>(
//        Model::Rocket::height * Global::getCurrentWorldScaleY());

//    m_sprites[0].setSpriteSize({ rocketSizeX, rocketSizeY });
//    m_sprites[0].setSpritePos({ rocketNdcX, rocketNdcY });

//    m_sprites[0].setAngle(rocket.angle);
//    m_sprites[0].draw(render);
//}

void ParallaxNebulas::formGroupOfSprites(
    const std::vector<size_t>& indices, std::vector<Sprite>& spritesQuadrant,
    std::vector<om::Vector<2>>& spriteBasePositionsQuadrant,
    std::vector<om::Vector<2>>& spriteBaseSizeQuadrant)
{
    spritesQuadrant.clear();
    spriteBasePositionsQuadrant.clear();
    for (const auto index : indices)
    {
        spritesQuadrant.push_back(m_sprites[index]);
        spriteBasePositionsQuadrant.push_back(m_spriteBasePositions[index]);
        spriteBaseSizeQuadrant.push_back(m_spriteBaseSize[index]);
    }
}

static std::vector<om::Vector<2>> addShiftToGroup(
    const std::vector<om::Vector<2>>& spriteBasePositionsQuadrant,
    const om::Vector<2>&              shift)
{
    std::vector<om::Vector<2>> newVector;
    std::transform(spriteBasePositionsQuadrant.begin(),
                   spriteBasePositionsQuadrant.end(),
                   std::back_inserter(newVector),
                   [&shift](const om::Vector<2>& currentPos) {
                       return currentPos + shift;
                   });
    return newVector;
}

void ParallaxNebulas::shiftAndAddToContainerSprites(
    const std::vector<Sprite>&        spritesGroup,
    const std::vector<om::Vector<2>>& spritesPosGroup,
    const std::vector<om::Vector<2>>& spritesSizeGroup,
    const om::Vector<2>&              shift)
{
    const auto shiftedGroup1 = addShiftToGroup(spritesPosGroup, shift);
    std::copy(spritesGroup.begin(), spritesGroup.end(),
              std::back_inserter(m_sprites));
    std::copy(shiftedGroup1.begin(), shiftedGroup1.end(),
              std::back_inserter(m_spriteBasePositions));
    std::copy(spritesSizeGroup.begin(), spritesSizeGroup.end(),
              std::back_inserter(m_spriteBaseSize));
}

void ParallaxNebulas::makeWiderParallax()
{
    std::vector<Sprite>        spritesQuadrant1{};
    std::vector<om::Vector<2>> spriteBasePositionsQuadrant1{};
    std::vector<om::Vector<2>> spriteBaseSizeQuadrant1{};
    formGroupOfSprites({ 0, 1, 4, 5 }, spritesQuadrant1,
                       spriteBasePositionsQuadrant1, spriteBaseSizeQuadrant1);
    std::vector<Sprite>        spritesQuadrant2;
    std::vector<om::Vector<2>> spriteBasePositionsQuadrant2{};
    std::vector<om::Vector<2>> spriteBaseSizeQuadrant2{};
    formGroupOfSprites({ 2, 3, 6, 7 }, spritesQuadrant2,
                       spriteBasePositionsQuadrant2, spriteBaseSizeQuadrant2);
    std::vector<Sprite>        spritesQuadrant3;
    std::vector<om::Vector<2>> spriteBasePositionsQuadrant3{};
    std::vector<om::Vector<2>> spriteBaseSizeQuadrant3{};
    formGroupOfSprites({ 8, 9, 12, 13 }, spritesQuadrant3,
                       spriteBasePositionsQuadrant3, spriteBaseSizeQuadrant3);

    std::vector<Sprite>        spritesQuadrant4;
    std::vector<om::Vector<2>> spriteBasePositionsQuadrant4{};
    std::vector<om::Vector<2>> spriteBaseSizeQuadrant4{};
    formGroupOfSprites({ 10, 11, 14, 15 }, spritesQuadrant4,
                       spriteBasePositionsQuadrant4, spriteBaseSizeQuadrant4);

    {
        const om::Vector<2> shift{ 0, sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant1,
                                      spriteBasePositionsQuadrant1,
                                      spriteBaseSizeQuadrant1, shift);
        shiftAndAddToContainerSprites(spritesQuadrant2,
                                      spriteBasePositionsQuadrant2,
                                      spriteBaseSizeQuadrant2, shift);
    }

    {
        const om::Vector<2> shift{ 0, -sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant3,
                                      spriteBasePositionsQuadrant3,
                                      spriteBaseSizeQuadrant3, shift);
        shiftAndAddToContainerSprites(spritesQuadrant4,
                                      spriteBasePositionsQuadrant4,
                                      spriteBaseSizeQuadrant4, shift);
    }
    {
        const om::Vector<2> shift{ sizeX, 0 };
        shiftAndAddToContainerSprites(spritesQuadrant1,
                                      spriteBasePositionsQuadrant1,
                                      spriteBaseSizeQuadrant1, shift);
        shiftAndAddToContainerSprites(spritesQuadrant3,
                                      spriteBasePositionsQuadrant3,
                                      spriteBaseSizeQuadrant3, shift);
    }
    {
        const om::Vector<2> shift{ -sizeX, 0 };
        shiftAndAddToContainerSprites(spritesQuadrant2,
                                      spriteBasePositionsQuadrant2,
                                      spriteBaseSizeQuadrant2, shift);
        shiftAndAddToContainerSprites(spritesQuadrant4,
                                      spriteBasePositionsQuadrant4,
                                      spriteBaseSizeQuadrant4, shift);
    }

    {
        const om::Vector<2> shift{ sizeX, sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant1,
                                      spriteBasePositionsQuadrant1,
                                      spriteBaseSizeQuadrant1, shift);
    }
    {
        const om::Vector<2> shift{ -sizeX, sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant2,
                                      spriteBasePositionsQuadrant2,
                                      spriteBaseSizeQuadrant2, shift);
    }
    {
        const om::Vector<2> shift{ sizeX, -sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant3,
                                      spriteBasePositionsQuadrant3,
                                      spriteBaseSizeQuadrant3, shift);
    }
    {
        const om::Vector<2> shift{ -sizeX, -sizeY };
        shiftAndAddToContainerSprites(spritesQuadrant4,
                                      spriteBasePositionsQuadrant4,
                                      spriteBaseSizeQuadrant4, shift);
    }
}

ParallaxNebulas::ParallaxNebulas(const std::string_view textureAttribureName,
                                 const std::string_view moveMatrixUniformName,
                                 const om::ProgramId&   programId,
                                 const om::TextureId&   tex,
                                 om::myGlfloat          parallaxCoef)
    : m_parallaxCoef{ parallaxCoef }
    , sizeY{ defaultSizeY / parallaxCoef }
    , sizeX{ defaultSizeX / parallaxCoef }

{
    const auto textureNumberOfLines   = 4;
    const auto textureNumberOfColumns = 4;

    const om::myGlfloat textureStepX = 1.0 / textureNumberOfColumns;
    const om::myGlfloat textureStepY = 1.0 / textureNumberOfLines;

    const om::myGlfloat renderStepX    = sizeX / textureNumberOfColumns;
    const om::myGlfloat renderStepY    = sizeY / textureNumberOfLines;
    const om::myGlfloat renderBaseSize = 0.7 / parallaxCoef;

    const om::Vector<2> textureSpriteSize{ textureStepX, textureStepY };

    const om::myGlfloat initSpriteXPos = -sizeX / 2 + renderStepX / 2;
    const om::myGlfloat initSpriteYPos = -sizeY / 2 + renderStepY / 2;

    m_sprites.reserve(textureNumberOfLines * textureNumberOfColumns);
    {
        om::myGlfloat textureXCurent = 0.0;
        om::myGlfloat textureYCurent = 1.0 - textureStepY;

        om::myGlfloat spriteXCurent = initSpriteXPos;
        om::myGlfloat spriteYCurent = initSpriteYPos;

        for (int line = 0; line < textureNumberOfLines; ++line)
        {
            for (int column = 0; column < textureNumberOfColumns; ++column)
            {
                m_spriteBasePositions.push_back(
                    { spriteXCurent, spriteYCurent });
                m_spriteBaseSize.push_back({ renderBaseSize, renderBaseSize });
                m_sprites.push_back(
                    { std::string{ "nebula" } + std::to_string(line) + " " +
                          std::to_string(column),
                      textureAttribureName,
                      moveMatrixUniformName,
                      programId,
                      tex,
                      { { textureXCurent, textureYCurent }, textureSpriteSize },
                      { { 0, 0 }, { renderBaseSize, renderBaseSize } },
                      0.f });
                textureXCurent += textureStepX;
                spriteXCurent += renderStepX;
            }
            textureXCurent = 0.0;
            textureYCurent -= textureStepY;
            spriteXCurent = initSpriteXPos;
            spriteYCurent += renderStepY;
        }
        makeWiderParallax();
    }
} // namespace renderObjects

om::myGlfloat ParallaxNebulas::getAngle() const
{
    return m_angle;
}
void ParallaxNebulas::setAngle(om::myGlfloat angle)
{
    m_angle = angle;
}

void ParallaxNebulas::draw(om::IEngine& render)
{
    const auto parallaxDist = (1 - m_parallaxCoef) / m_parallaxCoef;

    const auto scale = 1 / (1 / Global::getCurrentScale() + parallaxDist);

    auto userPosUnscaled =
        Global::getUserNdcPosition() * (1 / Global::getCurrentScale());
    // size must be bigger than 1
    if (userPosUnscaled.elements[0] > (sizeX / 2))
    {
        userPosUnscaled.elements[0] -=
            sizeX *
            (static_cast<int>(userPosUnscaled.elements[0] + (sizeX / 2)) /
             static_cast<int>(sizeX));
    }
    else if (userPosUnscaled.elements[0] < (-sizeX / 2))
    {
        userPosUnscaled.elements[0] -=
            sizeX *
            (static_cast<int>(userPosUnscaled.elements[0] - (sizeX / 2)) /
             static_cast<int>(sizeX));
    }

    if (userPosUnscaled.elements[1] > (sizeY / 2))
    {
        userPosUnscaled.elements[1] -=
            sizeY *
            (static_cast<int>(userPosUnscaled.elements[1] + (sizeY / 2)) /
             static_cast<int>(sizeY));
    }
    else if (userPosUnscaled.elements[1] < (-sizeY / 2))
    {
        userPosUnscaled.elements[1] -=
            sizeY *
            (static_cast<int>(userPosUnscaled.elements[1] - (sizeY / 2)) /
             static_cast<int>(sizeY));
    }

    auto userPosScaled = userPosUnscaled * scale;

    for (size_t i = 0; i < m_sprites.size(); ++i)
    {
        auto spritePosition = m_spriteBasePositions[i] * scale - userPosScaled;

        m_sprites[i].setSpritePos(spritePosition);

        const auto spriteSize = m_spriteBaseSize[i] * scale;
        m_sprites[i].setSpriteSize({ spriteSize });

        m_sprites[i].draw(render);
    }
}

} // namespace renderObjects
