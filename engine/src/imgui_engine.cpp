#include "imgui_engine.hpp"
#include "matrix.hpp"

#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
#include <algorithm>
#include <cstring>
#include <glad.h>
#include <imgui.h>
#include <iostream>
#include <vector>

om::IEngine*  ImguiEngine::m_engine{ nullptr };
om::ProgramId ImguiEngine::m_glImguiProgramId{};

const char* ImguiEngine::getClipboardText(void*)
{
    return SDL_GetClipboardText();
}

void ImguiEngine::setClipboardText(void*, const char* text)
{
    SDL_SetClipboardText(text);
}

bool ImguiEngine::initialize(SDL_Window* const  window,
                             om::IEngine* const engine)
{
    m_Window = window;
    m_engine = engine;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
    // values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos
    // requests (optional, rarely used)

    io.BackendPlatformName = "imgui_impl_sdl";
    io.BackendRendererName = "imgui_impl_opengl3";

    // Keyboard mapping. ImGui will use those indices to peek into the
    // io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown]    = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home]        = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]         = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert]      = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]       = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape]      = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
    io.KeyMap[ImGuiKey_A]           = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C]           = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V]           = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X]           = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y]           = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z]           = SDL_SCANCODE_Z;

    io.RenderDrawListsFn  = renderCallback;
    io.SetClipboardTextFn = setClipboardText;
    io.GetClipboardTextFn = getClipboardText;
    io.ClipboardUserData  = nullptr;

    m_MouseCursors.resize(ImGuiMouseCursor_COUNT);
    // Load mouse cursors
    m_MouseCursors[ImGuiMouseCursor_Arrow] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_MouseCursors[ImGuiMouseCursor_TextInput] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    m_MouseCursors[ImGuiMouseCursor_ResizeAll] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    m_MouseCursors[ImGuiMouseCursor_ResizeNS] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    m_MouseCursors[ImGuiMouseCursor_ResizeEW] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    m_MouseCursors[ImGuiMouseCursor_ResizeNESW] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    m_MouseCursors[ImGuiMouseCursor_ResizeNWSE] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    m_MouseCursors[ImGuiMouseCursor_Hand] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    m_MouseCursors[ImGuiMouseCursor_NotAllowed] =
        SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

    // Check and store if we are on Wayland
    m_MouseCanUseGlobalState =
        strncmp(SDL_GetCurrentVideoDriver(), "wayland", 7) != 0;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#endif
    m_Time = clock_t::now();
    return true;
}

void ImguiEngine::renderCallback(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    ImGuiIO& io        = ImGui::GetIO();
    int      fb_width  = int(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int      fb_height = int(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
    {
        return;
    }
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    const auto aspectMatrix{ om::MatrixFunctor::getScaleMatrix(
        { 2.0f / fb_width, -2.0f / fb_height }) };

    const auto sdlWindowXtoYScale =
        static_cast<double>(io.DisplaySize.x) / io.DisplaySize.y;

    auto xPixelShift =
        ((sdlWindowXtoYScale > om::IEngine::baseScaleXtoY))
            ? static_cast<om::myGlfloat>(io.DisplaySize.x - fb_width) / 2
            : 0.f;
    auto yPixelShift =
        (sdlWindowXtoYScale < om::IEngine::baseScaleXtoY)
            ? static_cast<om::myGlfloat>(io.DisplaySize.y - fb_height)
            : 0.f;

    const auto shiftMatrixResizingWindow{ om::MatrixFunctor::getShiftMatrix(
        { -xPixelShift, -yPixelShift }) };

    const auto shiftMatrix{ om::MatrixFunctor::getShiftMatrix({ -1.f, 1.f }) };

    const auto move_matrix =
        shiftMatrix * aspectMatrix * shiftMatrixResizingWindow;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list          = draw_data->CmdLists[n];
        unsigned int      idx_buffer_offset = 0;

        // om engine vertex format completely the same, prof:
        static_assert(sizeof(om::VertexTextured) == sizeof(ImDrawVert), "");
        static_assert(sizeof(om::VertexTextured::position) ==
                          sizeof(ImDrawVert::pos),
                      "");
        static_assert(sizeof(om::VertexTextured::position_tex) ==
                          sizeof(ImDrawVert::uv),
                      "");

        // there will be copying because my Vertex format is differrent from
        // ImGui
        size_t vert_count = static_cast<size_t>(cmd_list->VtxBuffer.size());

        std::vector<om::VertexTextured> omVertexes;
        omVertexes.reserve(vert_count);

        auto imguiVertexToOmVertex = [](const ImDrawVert& imguiVertex) {
            om::VertexTextured omVertex{
                { imguiVertex.pos.x, imguiVertex.pos.y },
                om::Color{ imguiVertex.col },
                { imguiVertex.uv.x, imguiVertex.uv.y }
            };
            return omVertex;
        };

        std::transform(cmd_list->VtxBuffer.begin(), cmd_list->VtxBuffer.end(),
                       std::back_inserter(omVertexes), imguiVertexToOmVertex);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            assert(pcmd->UserCallback == nullptr); // we not use it

            om::TextureId textureId{ static_cast<om::myUint>(
                reinterpret_cast<uint_least64_t>(pcmd->TextureId)) };

            auto imguiIndexToOmIndex = [](const ImDrawIdx& imguiVertex) {
                return static_cast<om::myUint>(imguiVertex);
            };

            const auto beginIndexBuffer =
                cmd_list->IdxBuffer.begin() + idx_buffer_offset;
            const auto endIndexBuffer = beginIndexBuffer + pcmd->ElemCount;
            std::vector<om::myUint> omIndices;
            omIndices.reserve(pcmd->ElemCount);
            std::transform(beginIndexBuffer, endIndexBuffer,
                           std::back_inserter(omIndices), imguiIndexToOmIndex);

            m_engine->render(omVertexes, omIndices, { textureId },
                             { "Texture" }, move_matrix, moveMatrixUniformName,
                             m_glImguiProgramId);

            idx_buffer_offset += pcmd->ElemCount;
        } // end for cmd_i
    }     // end for n
}
#include <fstream>

void ImguiEngine::createFontsTexture()
{
    // Build texture atlas
    ImGuiIO&       io     = ImGui::GetIO();
    uint_least8_t* pixels = nullptr;
    int            width  = 0;
    int            height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_FontTextureId = m_engine->addTexture(pixels, static_cast<size_t>(width),
                                           static_cast<size_t>(height));
    // Store our identifier
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(m_FontTextureId.id);
}

bool ImguiEngine::createDeviceObjects()
{
    const char* vertex_shader = "res/shaders/imgui_vertex_shader.vert";

    const char* fragment_shader = "res/shaders/imgui_fragment_shader.frag";

    m_glImguiProgramId = m_engine->addProgram(
        vertex_shader, fragment_shader,
        { { om::VertexTextured::positionAttributeNumber, "Position" },
          { om::VertexTextured::colorAttributeNumber, "Color" },
          { om::VertexTextured::positionTexAttributeNumber, "UV" } });

    createFontsTexture();

    return true;
}

void ImguiEngine::destroyDeviceObjects()
{
    ImGui::GetIO().Fonts->TexID    = nullptr;
    const auto isProgramErased     = m_engine->eraseProgram(m_glImguiProgramId);
    const auto isFontTextureErased = m_engine->eraseTexture(m_FontTextureId);

    if (!isProgramErased || !isFontTextureErased)
    {
        std::cerr << "Error when deleting program or texture imgui occured!"
                  << std::endl;
    }

    m_glImguiProgramId.id = 0;
    m_FontTextureId.id    = 0;
}

void ImguiEngine::shutdown()
{
    destroyDeviceObjects();
    m_Window = nullptr;
    m_engine = nullptr;
    // Destroy SDL mouse cursors
    for (const auto currentCursor : m_MouseCursors)
    {
        if (currentCursor)
            SDL_FreeCursor(currentCursor);
    }
    std::fill(m_MouseCursors.begin(), m_MouseCursors.end(), nullptr);
    m_MouseCursors.clear();
    ImGui::DestroyContext();
}

void ImguiEngine::newFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    if (io.Fonts->TexID == nullptr)
    {
        createDeviceObjects();
    }

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    // int display_w, display_h;
    SDL_GetWindowSize(m_Window, &w, &h);
    if (SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MINIMIZED)
        w = h = 0;

    GLint viewPort[4];

    glGetIntegerv(GL_VIEWPORT, viewPort);
    // SDL_GL_GetDrawableSize(m_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2(float(w), float(h));
    //        io.DisplayFramebufferScale = ImVec2(w > 0 ? float(display_w / w) :
    //        0.f,
    //                                            h > 0 ? float(display_h / h) :
    //                                            0.f);

    io.DisplayFramebufferScale = ImVec2(w > 0 ? float(viewPort[2]) / w : 0.f,
                                        h > 0 ? float(viewPort[3]) / h : 0.f);

    // Setup time step
    auto time      = clock_t::now();
    auto deltaTime = std::chrono::duration_cast<seconds_t>(time - m_Time);
    // checking if m_Time has been initialized
    io.DeltaTime = m_Time.time_since_epoch() > m_Time.time_since_epoch().zero()
                       ? deltaTime.count()
                       : (float)(1.0f / 60.0f);
    m_Time = time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from
    // SDL_PollEvent())
    updateMousePosAndButtons();

    // Hide OS mouse cursor if ImGui is drawing it
    updateMouseCursor();

    // Start the frame. This call will update the io.WantCaptureMouse,
    // io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not)
    // to your application.
    ImGui::NewFrame();
}

void ImguiEngine::updateMousePosAndButtons()
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when
    // ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
        SDL_WarpMouseInWindow(m_Window, (int)io.MousePos.x, (int)io.MousePos.y);
    else
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    int    mx, my;
    Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
    io.MouseDown[0] =
        m_MousePressed[0] ||
        (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) !=
            0; // If a mouse press event came, always pass it as "mouse held
    // this frame", so we don't miss click-release events that are
    // shorter than 1 frame.
    io.MouseDown[1] = m_MousePressed[1] ||
                      (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = m_MousePressed[2] ||
                      (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_MousePressed[0] = m_MousePressed[1] = m_MousePressed[2] = false;

    SDL_Window* focused_window = SDL_GetKeyboardFocus();
    if (m_Window == focused_window)
    {
        if (m_MouseCanUseGlobalState)
        {
            // SDL_GetMouseState() gives mouse position seemingly based on the
            // last window entered/focused(?) The creation of a new windows at
            // runtime and SDL_CaptureMouse both seems to severely mess up with
            // that, so we retrieve that position globally. Won't use this
            // workaround when on Wayland, as there is no global mouse position.
            int wx, wy;
            SDL_GetWindowPosition(focused_window, &wx, &wy);
            SDL_GetGlobalMouseState(&mx, &my);
            mx -= wx;
            my -= wy;
        }
        io.MousePos = ImVec2((float)mx, (float)my);
    }

    // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the
    // SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
    // The function is only supported from SDL 2.0.4 (released Jan 2016)
    bool any_mouse_button_down = ImGui::IsAnyMouseDown();
    SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
}

void ImguiEngine::updateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        SDL_ShowCursor(SDL_FALSE);
    }
    else
    {
        // Show OS mouse cursor
        SDL_SetCursor(m_MouseCursors[imgui_cursor]
                          ? m_MouseCursors[imgui_cursor]
                          : m_MouseCursors[ImGuiMouseCursor_Arrow]);
        SDL_ShowCursor(SDL_TRUE);
    }
}

bool ImguiEngine::processEvent(const SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
        case SDL_MOUSEWHEEL:
        {
            if (event->wheel.x > 0)
                io.MouseWheelH += 1;
            if (event->wheel.x < 0)
                io.MouseWheelH -= 1;
            if (event->wheel.y > 0)
                io.MouseWheel += 1;
            if (event->wheel.y < 0)
                io.MouseWheel -= 1;
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (event->button.button == SDL_BUTTON_LEFT)
                m_MousePressed[0] = true;
            if (event->button.button == SDL_BUTTON_RIGHT)
                m_MousePressed[1] = true;
            if (event->button.button == SDL_BUTTON_MIDDLE)
                m_MousePressed[2] = true;
            return true;
        }
        case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            int key = event->key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift      = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl       = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt        = ((SDL_GetModState() & KMOD_ALT) != 0);
#ifdef _WIN32
            io.KeySuper = false;
#else
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
#endif
            return true;
        }
    }
    return false;
}
