#pragma once

#include "iengine.hpp"

struct Rectangle
{

    om::Vector<2>    pos;
    om::Vector<2>    size;
    om::Matrix<2, 4> getPointsPosCentered() const;
    om::Matrix<2, 4> getPointsPosNormalizedCentered() const;
    om::Matrix<2, 4> getPointsPosDownLeft() const;
};

class Sprite
{
public:
    Sprite();
    Sprite(const Sprite&) = default;
    Sprite(const std::string_view id,
           const std::string_view textureAttribureName,
           const std::string_view moveMatrixUniformName,
           const om::ProgramId& programId, const om::TextureId& tex,
           const Rectangle& rectangleTexture, const Rectangle& rectangleSprite,
           const float angle, const om::Color& mixColor = defaultColor);

    void draw(om::IEngine& render, om::Vector<2> basePoint = {}) const;

    om::TextureId getTextureId() const;
    void          setTextureId(const om::TextureId& t);

    const Rectangle& getTextureCoord() const;
    void             setTextureCoord(const Rectangle& r);

    const om::Vector<2>& getSpritePos() const;
    void                 setSpritePos(const om::Vector<2>& r);

    const om::Vector<2>& getSpriteSize() const;
    void                 setSpriteSize(const om::Vector<2>& r);

    /// angle of sprite in degrees
    om::myGlfloat getAngle() const;
    void          setAngle(const om::myGlfloat angle);

    /// angle of sprite in degrees
    om::myGlfloat getBaseAngle() const;
    void          setBaseAngle(const om::myGlfloat angle);

    const std::string& getId() const;
    void               setId(std::string_view name);

    om::Color getMixColor() const;
    void      setMixColor(const om::Color& mixColor);

    std::string_view getTextureAttributeName() const;
    void setTextureAttributeName(std::string_view textureAttributeName);

    std::string_view getmoveMatrixUniformName() const;
    void setmoveMatrixUniformName(std::string_view moveMatrixUniformName);

    om::ProgramId getProgramId() const;
    void          setProgramId(const om::ProgramId& programId);

private:
    std::string      m_id{};
    std::string_view m_textureAttribureName{};
    std::string_view m_moveMatrixUniformName{};
    om::ProgramId    m_programId{};
    om::TextureId    m_textureId{};
    Rectangle        m_texCoordinates{};
    Rectangle        m_spriteCoordinates{};
    om::myGlfloat    m_baseAngle{}; // rad
    om::myGlfloat    m_angle{};     // rad
    om::Color        m_mixColor{ defaultColor };

    static constexpr om::Color defaultColor{ 1.f, 1.f, 1.f, 1.f };
};

inline bool operator==(const Rectangle& l, const Rectangle& r)
{
    return l.pos == r.pos && l.size == r.size;
}

inline bool operator==(const Sprite& l, const Sprite& r)
{
    return (l.getId() == r.getId()) && (l.getTextureId() == r.getTextureId()) &&
           (l.getTextureCoord() == r.getTextureCoord()) &&
           (l.getSpritePos() == r.getSpritePos()) &&
           (l.getSpriteSize() == r.getSpriteSize()) &&
           (std::abs(l.getAngle() - r.getAngle()) <=
            std::numeric_limits<om::myGlfloat>::epsilon());
}
