#pragma once
#include "audio_engine.hpp"
#include "glprogram.hpp"
#include "gltexture.hpp"
#include "iengine.hpp"
#include "imgui_engine.hpp"

#ifdef __MINGW32__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <forward_list>
#include <glad.h>
#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>

namespace om
{

class SoundBuffer;
class SoundTrack;

class EngineSdl : public IEngine
{
public:
    std::string initialize(std::string_view windowName,
                           std::string_view /*config*/) override final;
    bool        read_input(Event& e) override final;
    void        uninitialize() override final;
    ~EngineSdl() override final {}

    ProgramId addProgram(const std::string_view& vertexShaderFileName,
                         const std::string_view& fragmentShaderFileName,
                         const std::vector<std::pair<myUint, std::string_view>>&
                             attributes) override;

    bool eraseProgram(ProgramId programId) override;

    TextureId addTexture(const std::string_view& pathToTexture) override;

    TextureId addTexture(const uint_least8_t* const pixels, const size_t w,
                         const size_t h) override;

    bool eraseTexture(TextureId textureId) override;

    bool setCurrentDefaultProgram(ProgramId programId) override;

    bool setUniform(std::string_view       uniformName,
                    std::vector<myGlfloat> parameters,
                    ProgramId              programId = ProgramId()) override;

    bool setUniform(std::string_view uniformName, om::myGlfloat parameters,
                    ProgramId programId = ProgramId()) override;

    bool renderClearWindow(Color color = { 0, 0, 0, 0 }) override;
    ///////////////////////////////////////////////////////////////////////////
    void render(const std::vector<VertexWide>&       vertices,
                const std::vector<myUint>&           indices,
                const std::vector<TextureId>&        textureIds,
                const std::vector<std::string_view>& textureAttributesNames,
                const Matrix<3, 3>&                  moveMatrix,
                const std::string_view moveUnifromName = "u_move_matrix",
                ProgramId              programId       = ProgramId(),
                ShapeType              type = ShapeType::triangle) override;

    void render(const std::vector<VertexTextured>&   vertices,
                const std::vector<myUint>&           indices,
                const std::vector<TextureId>&        textureIds,
                const std::vector<std::string_view>& textureAttributesNames,
                const Matrix<3, 3>&                  moveMatrix,
                const std::string_view moveUnifromName = "u_move_matrix",
                ProgramId              programId       = ProgramId(),
                ShapeType              type = ShapeType::triangle) override;

    void render(const std::vector<VertexMorphed>& vertices,
                const std::vector<myUint>&        indices,
                const Matrix<3, 3>&               moveMatrix,
                const std::string_view moveUnifromName = "u_move_matrix",
                ProgramId              programId       = ProgramId(),
                ShapeType              type = ShapeType::triangle) override;

    void render(const std::vector<Vertex>& vertices,
                const std::vector<myUint>& indices,
                const Matrix<3, 3>&        moveMatrix,
                const std::string_view     moveUnifromName = "u_move_matrix",
                ProgramId                  programId       = ProgramId(),
                ShapeType                  type = ShapeType::triangle) override;
    ///////////////////////////////////////////////////////////////////////////
    void render(const std::vector<VertexWide>&       vertices,
                const std::vector<myUint>&           indices,
                const std::vector<TextureId>&        textureIds,
                const std::vector<std::string_view>& textureAttributesNames,
                ProgramId                            programId = ProgramId(),
                ShapeType type = ShapeType::triangle) override;

    void render(const std::vector<VertexTextured>&   vertices,
                const std::vector<myUint>&           indices,
                const std::vector<TextureId>&        textureIds,
                const std::vector<std::string_view>& textureAttributesNames,
                ProgramId                            programId = ProgramId(),
                ShapeType type = ShapeType::triangle) override;

    void render(const std::vector<VertexMorphed>& vertices,
                const std::vector<myUint>&        indices,
                ProgramId                         programId = ProgramId(),
                ShapeType type = ShapeType::triangle) override;

    void render(const std::vector<Vertex>& vertices,
                const std::vector<myUint>& indices,
                ProgramId                  programId = ProgramId(),
                ShapeType                  type = ShapeType::triangle) override;
    ///////////////////////////////////////////////////////////////////////////
    void renderTriangle(const Triangle<Vertex>& t,
                        ProgramId programId = ProgramId()) override;

    bool updateWindow(Color fillingColor = { 0, 0, 0, 0 }) override;

    ISoundTrack*  addSoundTrack(std::string_view path) override;
    void          eraseSoundTrack(ISoundTrack* soundTrack) override;
    ISoundBuffer* addSoundBuffer(ISoundTrack* soundTrack) override;
    void playSoundBufferOnce(ISoundTrack* soundTrack, om::myGlfloat volume = 1,
                             om::myGlfloat leftRightBalance = 0,
                             om::myGlfloat stereo           = 0) override;
    void eraseSoundBuffer(ISoundBuffer* soundBuffer) override;

    std::array<myGlfloat, 2> getDrawableInchesSize() override;
    std::array<int, 2>       getDrawablePixelSize() override;

    std::array<int, 2> getDisplayPixelSize() override;
    int                getDisplayRefreshRate() override;

    void uiNewFrame() override;

private:
    bool initSdl(std::stringstream& serr);
    bool initSdlWindow(std::string_view windowName, std::stringstream& serr);
    bool initOpenGl(std::stringstream& serr);

    bool loadGLFunctionsPointers(std::stringstream& serr);

    bool initBuffersAndVAO(std::stringstream& serr);
    bool initIndexBuffer(std::stringstream& serr);

    void       setDebugOpenGl();
    void       setAdditionalGlParameters();
    GlProgram* findProgram(ProgramId programId);
    GlTexture* findTexture(TextureId textureId);

    template <typename T, typename = std::enable_if_t<is_vertex<T>>>
    void renderInternal(const std::vector<T>&      vertices,
                        const std::vector<myUint>& indices, ShapeType type,
                        ProgramId programId);

    template <typename T, typename = std::enable_if_t<is_texture_vertex<T>>>
    void renderTexturedInternal(
        const std::vector<T>& vertices, const std::vector<myUint>& indices,
        std::vector<TextureId>        textureIds,
        std::vector<std::string_view> textureAttributesNames, ShapeType type,
        ProgramId programId);

    template <typename T, typename = std::enable_if_t<is_vertex<T>>>
    void renderTriangleInternal(const Triangle<T>& t, ProgramId programId);

    bool resizeSdlGlWindow(int w, int h);

    bool findDisplayIndex(SDL_Window* window, int& r_displayIndex,
                          std::stringstream& serr);

    bool findSdlWindowDpi(std::stringstream& serr);

    bool findDisplayPixelSizeAndRefreshRate(std::stringstream& serr);

    void uiRender();

    std::string_view m_shaderVersionLine;

    SDL_Window*    m_window{};
    uint_least32_t m_windowId{};

    int       m_displayIndex{};
    int       m_displayPixelWidth{};
    int       m_displayPixelHeight{};
    myGlfloat m_displayWidthDpi{};
    myGlfloat m_displayHeightDpi{};
    int       m_displayRefreshRate{};

    SDL_GLContext m_glContext{};

    GlProgram* m_currentProgram{};
    ProgramId  lastProgramId{};
    std::unordered_map<ProgramId, GlProgram, MyIdsHash<ProgramId>> m_programs;

    GlTexture* m_currentTexture{};
    TextureId  lastTextureId{};
    std::unordered_map<TextureId, GlTexture, MyIdsHash<TextureId>> m_textures;

    static constexpr int creatingSdlWindowFlags{ SDL_WINDOW_OPENGL |
                                                 SDL_WINDOW_RESIZABLE |
                                                 SDL_WINDOW_SHOWN |
                                                 SDL_WINDOW_ALLOW_HIGHDPI };

    static constexpr myGlfloat initialWpixels{ 960 };
    static constexpr myGlfloat initialHpixels{ 540 };

    ImguiEngine m_imguiEngine;
    AudioEngine m_audioEngine;
};
} // end namespace om
