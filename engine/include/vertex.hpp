#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <iosfwd>
#ifndef OM_DECLSPEC
#define OM_DECLSPEC
#endif

namespace om
{
using myGlfloat = float;
using myUint    = unsigned int;

class OM_DECLSPEC Color
{
public:
    constexpr Color() = default;
    constexpr explicit Color(uint_least32_t rgba_);
    constexpr Color(myGlfloat r, myGlfloat g, myGlfloat b, myGlfloat a);

    float get_r() const;
    float get_g() const;
    float get_b() const;
    float get_a() const;

    void set_r(const myGlfloat r);
    void set_g(const myGlfloat g);
    void set_b(const myGlfloat b);
    void set_a(const myGlfloat a);

private:
    uint_least32_t rgba{};
};

constexpr Color::Color(uint_least32_t rgba_)
    : rgba(rgba_)
{
}
constexpr Color::Color(myGlfloat r, myGlfloat g, myGlfloat b, myGlfloat a)
{
    assert(r <= 1 && r >= 0);
    assert(g <= 1 && g >= 0);
    assert(b <= 1 && b >= 0);
    assert(a <= 1 && a >= 0);

    uint_least32_t r_ = static_cast<uint_least32_t>(r * 255);
    uint_least32_t g_ = static_cast<uint_least32_t>(g * 255);
    uint_least32_t b_ = static_cast<uint_least32_t>(b * 255);
    uint_least32_t a_ = static_cast<uint_least32_t>(a * 255);

    rgba = a_ << 24 | b_ << 16 | g_ << 8 | r_;
}

struct OM_DECLSPEC Position3
{
    myGlfloat x{};
    myGlfloat y{};
    myGlfloat z{};
};

struct OM_DECLSPEC Position2
{
    myGlfloat x{};
    myGlfloat y{};
};

#pragma pack(push, 1)
struct OM_DECLSPEC Vertex
{
    Vertex(myGlfloat xin, myGlfloat yin, myGlfloat rin, myGlfloat gin,
           myGlfloat bin, myGlfloat ain = 1)
        : position{ xin, yin }
        , color{ rin, gin, bin, ain }
    {
    }

    Vertex(Position2 pos, Color col)
        : position{ pos }
        , color{ col }
    {
    }

    Vertex()
        : position{}
        , color{}
    {
    }

    Position2 position;
    Color     color;

public:
    static constexpr myUint positionByteOffset = 0;
    static constexpr myUint colorByteOffset =
        positionByteOffset + sizeof(position);
    static constexpr myUint positionAttributeNumber = 0;
    static constexpr myUint colorAttributeNumber    = 1;
    static constexpr myUint positionAttributeAmount =
        sizeof(position) / sizeof(position.x);
    static constexpr myUint colorAttributeAmount = sizeof(color);

    static constexpr std::array<myUint, 2> attributesNumbers{
        positionAttributeNumber, colorAttributeNumber
    };
    static constexpr std::array<myUint, 2> attributesByteOffsets{
        positionByteOffset, colorByteOffset
    };
    static constexpr std::array<myUint, 2> attributesAmounts{
        positionAttributeAmount, colorAttributeAmount
    };
};

struct OM_DECLSPEC VertexMorphed : public Vertex
{
    VertexMorphed(myGlfloat xin1, myGlfloat yin1, myGlfloat rin, myGlfloat gin,
                  myGlfloat bin, myGlfloat ain, myGlfloat xin2, myGlfloat yin2)
        : Vertex(xin1, yin1, rin, gin, bin, ain)
        , position2{ xin2, yin2 }
    {
    }

    VertexMorphed(myGlfloat xin, myGlfloat yin, myGlfloat rin, myGlfloat gin,
                  myGlfloat bin, myGlfloat ain = 1)
        : VertexMorphed(xin, yin, rin, gin, bin, ain, xin, yin)
    {
    }

    VertexMorphed(Position2 posStart, Color col, Position2 posFinish)
        : Vertex(posStart, col)
        , position2{ posFinish }
    {
    }

    VertexMorphed()
        : position2{}
    {
    }

    Position2 position2;

    static constexpr myUint position2ByteOffset =
        colorByteOffset + sizeof(color);
    static constexpr myUint position2AttributeNumber = 2;

    static constexpr std::array<myUint, 3> attributesNumbers{
        positionAttributeNumber, colorAttributeNumber, position2AttributeNumber
    };
    static constexpr std::array<myUint, 3> attributesByteOffsets{
        positionByteOffset, colorByteOffset, position2ByteOffset
    };
    static constexpr std::array<myUint, 3> attributesAmounts{
        positionAttributeAmount,
        colorAttributeAmount,
        positionAttributeAmount,
    };
};

struct OM_DECLSPEC VertexTextured : public Vertex
{
    VertexTextured(myGlfloat xin, myGlfloat yin, myGlfloat, myGlfloat rin,
                   myGlfloat gin, myGlfloat bin, myGlfloat ain,
                   myGlfloat xin_tex, myGlfloat yin_tex)
        : Vertex(xin, yin, rin, gin, bin, ain)
        , position_tex{ xin_tex, yin_tex }
    {
    }

    VertexTextured(Position2 pos, Color col, Position2 pos_tex)
        : Vertex(pos, col)
        , position_tex{ pos_tex }
    {
    }

    VertexTextured()
        : position_tex{}
    {
    }

    Position2 position_tex;

public:
    static constexpr myUint positionTexByteOffset =
        colorByteOffset + sizeof(color);
    static constexpr myUint positionTexAttributeNumber = 3;

    static constexpr myUint positionTexAttributeAmount =
        sizeof(position_tex) / sizeof(position_tex.x);

    static constexpr std::array<myUint, 3> attributesNumbers{
        positionAttributeNumber, colorAttributeNumber,
        positionTexAttributeNumber
    };
    static constexpr std::array<myUint, 3> attributesByteOffsets{
        positionByteOffset, colorByteOffset, positionTexByteOffset
    };
    static constexpr std::array<myUint, 3> attributesAmounts{
        positionAttributeAmount,
        colorAttributeAmount,
        positionTexAttributeAmount,
    };
};

struct OM_DECLSPEC VertexWide : public VertexMorphed
{
    VertexWide(myGlfloat xinStart, myGlfloat yinStart, myGlfloat rin,
               myGlfloat gin, myGlfloat bin, myGlfloat ain, myGlfloat xinFinish,
               myGlfloat yinFinish, myGlfloat xin_tex, myGlfloat yin_tex)
        : VertexMorphed(xinStart, yinStart, rin, gin, bin, ain, xinFinish,
                        yinFinish)
        , position_tex{ xin_tex, yin_tex }
    {
    }

    VertexWide(Position2 posStart, Color col, Position2 posFinish,
               Position2 positionTex)
        : VertexMorphed(posStart, col, posFinish)
        , position_tex{ positionTex }
    {
    }

    VertexWide() {}

    Position2 position_tex;

public:
    static constexpr myUint positionTexAttributeNumber = 3;
    static constexpr myUint positionTexByteOffset =
        position2ByteOffset + sizeof(position2);
    static constexpr myUint positionTexAttributeAmount =
        sizeof(position_tex) / sizeof(position_tex.x);

    static constexpr std::array<myUint, 4> attributesNumbers{
        positionAttributeNumber, colorAttributeNumber, position2AttributeNumber,
        positionTexAttributeNumber
    };
    static constexpr std::array<myUint, 4> attributesByteOffsets{
        positionByteOffset, colorByteOffset, position2ByteOffset,
        positionTexByteOffset
    };
    static constexpr std::array<myUint, 4> attributesAmounts{
        positionAttributeAmount,
        colorAttributeAmount,
        positionAttributeAmount,
        positionTexAttributeAmount,
    };
};
#pragma pack(pop)

template <typename T>
constexpr bool is_vertex = std::is_base_of<Vertex, T>::value;

template <typename T>
constexpr bool is_texture_vertex = std::is_base_of<VertexTextured, T>::value ||
                                   std::is_base_of<VertexWide, T>::value;

template <typename T, typename = std::enable_if_t<is_vertex<T>>>
struct OM_DECLSPEC Triangle
{
    T v[3]{};
};

OM_DECLSPEC std::istream& operator>>(std::istream& in, Vertex& vertex);

OM_DECLSPEC std::istream& operator>>(std::istream&     in,
                                     Triangle<Vertex>& triangle);
} // namespace om
