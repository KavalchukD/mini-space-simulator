#pragma once
#include "iengine.hpp"
#include <chrono>

struct ImDrawData;
struct SDL_Window;
struct SDL_Cursor;
union SDL_Event;

class ImguiEngine
{
public:
    bool initialize(SDL_Window* const window, om::IEngine* const engine);
    bool processEvent(const SDL_Event* event);
    void newFrame();
    bool isInit() { return isImguiInit; }
    void shutdown();

private:
    using clock_t   = std::chrono::steady_clock;
    using seconds_t = std::chrono::duration<float, std::ratio<1>>;

    static void        renderCallback(ImDrawData* draw_data);
    void               createFontsTexture();
    static const char* getClipboardText(void*);
    static void        setClipboardText(void*, const char* text);
    bool               createDeviceObjects();
    void               destroyDeviceObjects();

    void updateMousePosAndButtons();
    void updateMouseCursor();

    bool                             isImguiInit{ false };
    static om::IEngine*              m_engine;
    SDL_Window*                      m_Window{ nullptr };
    std::chrono::time_point<clock_t> m_Time{};
    bool                             m_MousePressed[3]{ false, false, false };
    float                            m_MouseWheel{ 0.0f };
    std::vector<SDL_Cursor*>         m_MouseCursors{};
    bool                             m_MouseCanUseGlobalState{ true };

    om::TextureId        m_FontTextureId{};
    static om::ProgramId m_glImguiProgramId;

    static constexpr std::string_view moveMatrixUniformName{ "u_move_matrix" };
};
