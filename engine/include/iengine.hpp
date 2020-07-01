#pragma once
#include "matrix.hpp"
#include "vertex.hpp"
#include <array>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#ifndef OM_DECLSPEC
#define OM_DECLSPEC
#endif

namespace om
{

using myGlfloat = float;
using myUint    = unsigned int;

struct OM_DECLSPEC ProgramId
{
    myUint      id{};
    bool        isInit() const { return id != 0; }
    friend bool operator==(const ProgramId& id1, const ProgramId& id2)
    {
        return id1.id == id2.id;
    }
    friend bool operator<(const ProgramId& id1, const ProgramId& id2)
    {
        return id1.id < id2.id;
    }
};

struct OM_DECLSPEC TextureId
{
    myUint      id{};
    bool        isInit() const { return id != 0; }
    friend bool operator==(const TextureId& id1, const TextureId& id2)
    {
        return id1.id == id2.id;
    }
    friend bool operator<(const TextureId& id1, const TextureId& id2)
    {
        return id1.id < id2.id;
    }
};

template <typename T>
constexpr bool is_id = std::is_base_of<T, ProgramId>::value ||
                       std::is_base_of<T, TextureId>::value;

template <typename T, typename = std::enable_if_t<is_id<T>>>
class MyIdsHash
{
public:
    size_t operator()(const T& programId) const { return programId.id; }
};

enum class OM_DECLSPEC ShapeType
{
    triangle_strip = 1,
    line           = 2,
    triangle       = 3,

};

/// dendy gamepad emulation events
enum class OM_DECLSPEC EventType : size_t
{
    /// input events
    left_pressed,
    left_released,
    right_pressed,
    right_released,
    up_pressed,
    up_released,
    down_pressed,
    down_released,
    select_pressed,
    select_released,
    start_pressed,
    start_released,
    button1_pressed, // space
    button1_released,
    button2_pressed, // middle key of mouse
    button2_released,

    cursor_motion,
    wheel_rolled,
    /// virtual console events
    turn_off,
    max_type,
};

struct OM_DECLSPEC EventParam
{
    myGlfloat p0{};
    myGlfloat p1{};
    myGlfloat p2{};
};

struct OM_DECLSPEC Event
{
    EventType   type{ EventType::max_type };
    EventParam  parameters{};
    friend bool operator==(const Event& e1, const Event& e2)
    {
        return e1.type == e2.type;
    }
};

class OM_DECLSPEC ISoundTrack
{
public:
    virtual ~ISoundTrack() {}
};

class OM_DECLSPEC ISoundBuffer
{
public:
    enum class properties
    {
        once,
        looped,
        disposable
    };

    virtual ~ISoundBuffer(){};
    virtual void play(const properties)                        = 0;
    virtual void stop()                                        = 0;
    virtual void proceed()                                     = 0;
    virtual void pause()                                       = 0;
    virtual void setVolume(float inVolume)                     = 0;
    virtual void setLeftRightBalance(float inLeftRightBalance) = 0;
    virtual void setStereo(float stereoBalance)                = 0;
};

OM_DECLSPEC std::ostream& operator<<(std::ostream& stream, const EventType e);

class OM_DECLSPEC IEngine
{
public:
    enum class EngineTypes : size_t
    {
        sdl,
        max_types,
    };
    virtual ~IEngine() noexcept {}
    /// create main window
    /// on success return empty string
    virtual std::string initialize(std::string_view windowName,
                                   std::string_view config) = 0;
    /// pool event from input queue
    /// return true if more events in queue
    virtual bool read_input(Event& e) = 0;
    virtual void uninitialize()       = 0;

    virtual ProgramId addProgram(
        const std::string_view& vertexShaderFileName,
        const std::string_view& fragmentShaderFileName,
        const std::vector<std::pair<myUint, std::string_view>>& attributes) = 0;

    virtual bool eraseProgram(ProgramId programId) = 0;

    virtual TextureId addTexture(const std::string_view& pathToTexture) = 0;

    virtual TextureId addTexture(const uint_least8_t* const pixels,
                                 const size_t w, const size_t h) = 0;

    virtual bool eraseTexture(TextureId textureId) = 0;

    virtual bool setCurrentDefaultProgram(ProgramId programId) = 0;

    virtual bool setUniform(std::string_view       uniformName,
                            std::vector<myGlfloat> parameters,
                            ProgramId              programId = ProgramId()) = 0;

    virtual bool setUniform(std::string_view uniformName, myGlfloat parameters,
                            ProgramId programId = ProgramId()) = 0;

    virtual bool renderClearWindow(Color color = { 0, 0, 0, 0 }) = 0;

    virtual void render(
        const std::vector<VertexWide>&       vertices,
        const std::vector<myUint>&           indices,
        const std::vector<TextureId>&        textureIds,
        const std::vector<std::string_view>& textureAttributesNames,
        const Matrix<3, 3>&                  moveMatrix,
        const std::string_view               moveUnifromName = "u_move_matrix",
        ProgramId                            programId       = ProgramId(),
        ShapeType                            type = ShapeType::triangle) = 0;

    virtual void render(
        const std::vector<VertexTextured>&   vertices,
        const std::vector<myUint>&           indices,
        const std::vector<TextureId>&        textureIds,
        const std::vector<std::string_view>& textureAttributesNames,
        const Matrix<3, 3>&                  moveMatrix,
        const std::string_view               moveUnifromName = "u_move_matrix",
        ProgramId                            programId       = ProgramId(),
        ShapeType                            type = ShapeType::triangle) = 0;

    virtual void render(
        const std::vector<VertexMorphed>& vertices,
        const std::vector<myUint>& indices, const Matrix<3, 3>& moveMatrix,
        const std::string_view moveUnifromName = "u_move_matrix",
        ProgramId              programId       = ProgramId(),
        ShapeType              type            = ShapeType::triangle) = 0;

    virtual void render(
        const std::vector<Vertex>& vertices, const std::vector<myUint>& indices,
        const Matrix<3, 3>&    moveMatrix,
        const std::string_view moveUnifromName = "u_move_matrix",
        ProgramId              programId       = ProgramId(),
        ShapeType              type            = ShapeType::triangle) = 0;

    virtual void render(
        const std::vector<VertexWide>&       vertices,
        const std::vector<myUint>&           indices,
        const std::vector<TextureId>&        textureIds,
        const std::vector<std::string_view>& textureAttributesNames,
        ProgramId                            programId = ProgramId(),
        ShapeType                            type = ShapeType::triangle) = 0;

    virtual void render(
        const std::vector<VertexTextured>&   vertices,
        const std::vector<myUint>&           indices,
        const std::vector<TextureId>&        textureIds,
        const std::vector<std::string_view>& textureAttributesNames,
        ProgramId                            programId = ProgramId(),
        ShapeType                            type = ShapeType::triangle) = 0;

    virtual void render(const std::vector<VertexMorphed>& vertices,
                        const std::vector<myUint>&        indices,
                        ProgramId programId = ProgramId(),
                        ShapeType type      = ShapeType::triangle) = 0;

    virtual void render(const std::vector<Vertex>& vertices,
                        const std::vector<myUint>& indices,
                        ProgramId                  programId = ProgramId(),
                        ShapeType type = ShapeType::triangle) = 0;

    virtual void renderTriangle(const Triangle<Vertex>& t,
                                ProgramId programId = ProgramId()) = 0;

    virtual bool updateWindow(Color fillingColor = { 0, 0, 0, 0 }) = 0;

    virtual void uiNewFrame() = 0;

    virtual ISoundTrack*  addSoundTrack(std::string_view path)          = 0;
    virtual void          eraseSoundTrack(ISoundTrack* soundTrack)      = 0;
    virtual ISoundBuffer* addSoundBuffer(ISoundTrack* soundTrack)       = 0;
    virtual void          playSoundBufferOnce(ISoundTrack*  soundTrack,
                                              om::myGlfloat volume = 1,
                                              om::myGlfloat leftRightBalance = 0,
                                              om::myGlfloat stereo = 0) = 0;
    virtual void          eraseSoundBuffer(ISoundBuffer* soundBuffer)   = 0;

    virtual std::array<myGlfloat, 2> getDrawableInchesSize() = 0;
    virtual std::array<int, 2>       getDrawablePixelSize()  = 0;

    virtual std::array<int, 2> getDisplayPixelSize()   = 0;
    virtual int                getDisplayRefreshRate() = 0;

    static constexpr myGlfloat minCoordinate{ -1.0 };
    static constexpr myGlfloat maxCoordinate{ 1.0 };
    static constexpr myGlfloat baseScaleXtoY{ 16.0 / 9.0 };
};

} // end namespace om
