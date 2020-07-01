#include "sprite.hpp"
#include <algorithm>
#include <iostream>

om::Matrix<2, 4> Rectangle::getPointsPosCentered() const
{

    om::Matrix<2, 4> pointsPositions{ getPointsPosNormalizedCentered() };

    std::for_each(pointsPositions.begin(), pointsPositions.end(),
                  [this](om::Vector<2>& column) { column = column + pos; });
    return pointsPositions;
}

om::Matrix<2, 4> Rectangle::getPointsPosNormalizedCentered() const
{

    om::Matrix<2, 4> pointsPositions;

    pointsPositions.columns[0] = -(size * 0.5);
    om::Vector<2> sizeXReversed{ -size.elements[0], size.elements[1] };
    pointsPositions.columns[1] = (sizeXReversed * 0.5);
    om::Vector<2> sizeYReversed{ size.elements[0], -size.elements[1] };
    pointsPositions.columns[2] = (sizeYReversed * 0.5);
    pointsPositions.columns[3] = (size * 0.5);
    return pointsPositions;
}

om::Matrix<2, 4> Rectangle::getPointsPosDownLeft() const
{

    om::Matrix<2, 4> pointsPositions;

    pointsPositions.columns[0] = pos + om::Vector<2>{ 0.f, 0.f };
    pointsPositions.columns[1] = pos + om::Vector<2>{ 0.f, size.elements[1] };
    pointsPositions.columns[2] = pos + om::Vector<2>{ size.elements[0], 0.f };
    pointsPositions.columns[3] = pos + size;
    return pointsPositions;
}

Sprite::Sprite()
    : m_id{ "__no__id__error:__" }
{
}

Sprite::Sprite(const std::string_view id,
               const std::string_view textureAttribureName,
               const std::string_view moveMatrixUniformName,
               const om::ProgramId& programId, const om::TextureId& tex,
               const Rectangle& rectangleTexture,
               const Rectangle& rectangleSprite, const float angle,
               const om::Color& mixColor)
    : m_id{ id }
    , m_textureAttribureName{ textureAttribureName }
    , m_moveMatrixUniformName{ moveMatrixUniformName }
    , m_programId{ programId }
    , m_textureId{ tex }
    , m_texCoordinates{ rectangleTexture }
    , m_spriteCoordinates{ rectangleSprite }
    , m_angle(angle)
    , m_mixColor{ mixColor }

{
}

void Sprite::draw(om::IEngine& render, om::Vector<2> basePoint) const
{
    if (!m_textureId.isInit())
    {
        std::cerr << "Texture id has not been initialazed." << std::endl;
        return; // sprite is empty nothing to do
    }

    ///   0            1
    ///   *------------*
    ///   |           /|
    ///   |         /  |
    ///   |      P/    |  // P - pos_ or center of sprite
    ///   |     /      |
    ///   |   /        |
    ///   | /          |
    ///   *------------*
    ///   3            2
    ///

    using namespace om;

    std::vector<VertexTextured> vertexes;

    const auto spritePositions =
        m_spriteCoordinates.getPointsPosNormalizedCentered();
    const auto texturePositions = m_texCoordinates.getPointsPosDownLeft();

    auto formVertex = [](const om::Vector<2>& positionSprite,
                         const om::Vector<2>& positionTexture) {
        VertexTextured newVertex;
        newVertex.position     = { positionSprite.elements[0],
                               positionSprite.elements[1] };
        newVertex.position_tex = { positionTexture.elements[0],
                                   positionTexture.elements[1] };
        return newVertex;
    };

    std::transform(spritePositions.begin(), spritePositions.end(),
                   texturePositions.begin(), std::back_inserter(vertexes),
                   formVertex);

    auto setColor = [this](VertexTextured& vertex) {
        vertex.color = m_mixColor;
    };

    std::for_each_n(vertexes.begin(), vertexes.size(), setColor);

    const auto screen_size = render.getDrawableInchesSize();

    const auto aspect = screen_size[1] / screen_size[0];

    const auto window_aspect = MatrixFunctor::getScaleMatrix({ aspect, 1.0 });

    const auto rotationBase = MatrixFunctor::getRotateMatrix(m_baseAngle);

    const auto rotation = MatrixFunctor::getRotateMatrix(m_angle, basePoint);

    const auto move = MatrixFunctor::getShiftMatrix(m_spriteCoordinates.pos);

    const auto world_transform = window_aspect * move * rotation * rotationBase;

    std::vector<myUint> indices{ 0, 1, 3, 0, 2, 3 };

    render.render(vertexes, indices, { m_textureId },
                  { m_textureAttribureName }, world_transform,
                  m_moveMatrixUniformName, m_programId);
}

om::TextureId Sprite::getTextureId() const
{
    return m_textureId;
}

void Sprite::setTextureId(const om::TextureId& t)
{
    m_textureId = t;
}

const Rectangle& Sprite::getTextureCoord() const
{
    return m_texCoordinates;
}

void Sprite::setTextureCoord(const Rectangle& r)
{
    m_texCoordinates = r;
}

const om::Vector<2>& Sprite::getSpritePos() const
{
    return m_spriteCoordinates.pos;
}

void Sprite::setSpritePos(const om::Vector<2>& r)
{
    m_spriteCoordinates.pos = r;
}

const om::Vector<2>& Sprite::getSpriteSize() const
{
    return m_spriteCoordinates.size;
}

void Sprite::setSpriteSize(const om::Vector<2>& r)
{
    m_spriteCoordinates.size = r;
}

float Sprite::getAngle() const
{
    return m_angle;
}

void Sprite::setAngle(const om::myGlfloat angle)
{
    m_angle = angle;
}

float Sprite::getBaseAngle() const
{
    return m_baseAngle;
}

void Sprite::setBaseAngle(const om::myGlfloat baseAngle)
{
    m_baseAngle = baseAngle;
}

const std::string& Sprite::getId() const
{
    return m_id;
}

void Sprite::setId(std::string_view name)
{
    m_id = name;
}

std::string_view Sprite::getTextureAttributeName() const
{
    return m_textureAttribureName;
}
void Sprite::setTextureAttributeName(std::string_view textureAttributeName)
{
    m_textureAttribureName = textureAttributeName;
}

std::string_view Sprite::getmoveMatrixUniformName() const
{
    return m_moveMatrixUniformName;
}
void Sprite::setmoveMatrixUniformName(std::string_view moveMatrixUniformName)
{
    m_moveMatrixUniformName = moveMatrixUniformName;
}

om::ProgramId Sprite::getProgramId() const
{
    return m_programId;
}
void Sprite::setProgramId(const om::ProgramId& programId)
{
    m_programId = programId;
}

om::Color Sprite::getMixColor() const
{
    return m_mixColor;
}
void Sprite::setMixColor(const om::Color& mixColor)
{
    m_mixColor = mixColor;
}
