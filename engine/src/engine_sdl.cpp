#include "engine_sdl.hpp"
#include "opengl_debug.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <exception>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace om
{

using keyCodeType = SDL_Scancode;

template <typename T>
static void getGlFunctionPointer(const char* func_name, T& result)
{
    void* gl_pointer = SDL_GL_GetProcAddress(func_name);
    if (gl_pointer == nullptr)
    {
        throw std::runtime_error(
            std::string("Can't get pointer to OpenGL function ") + func_name);
    }
    result = reinterpret_cast<T>(gl_pointer);
}

//////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& stream, const EventType e)
{
    std::uint32_t           value = static_cast<std::uint32_t>(e);
    constexpr std::uint32_t minimal =
        static_cast<std::uint32_t>(EventType::left_pressed);

    constexpr std::uint32_t maximal =
        static_cast<std::uint32_t>(EventType::turn_off);
    const auto numberOfEvents = static_cast<size_t>(EventType::max_type);
    static std::array<std::string_view, numberOfEvents> event_names = {
        { /// input events
          "left_pressed", "left_released", "right_pressed", "right_released",
          "up_pressed", "up_released", "down_pressed", "down_released",
          "select_pressed", "select_released", "start_pressed",
          "start_released", "button1_pressed ", " button1_released ",
          "button2_pressed", "button2_released", "cursor_motion",
          "wheel_rolled",
          /// virtual console events
          "turn_off" }
    };

    if (value >= minimal && value <= maximal)
    {
        stream << event_names[value];
        return stream;
    }
    else
    {
        throw std::runtime_error("too big event value");
    }
}

static std::ostream& operator<<(std::ostream& out, const SDL_version& v)
{
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

//////////////////////////////////////////////////////////////////////
struct bind
{
    bind(keyCodeType k, std::string_view s, EventType pressed,
         EventType released)
        : key(k)
        , name(s)
        , event_pressed(pressed)
        , event_released(released)
    {
    }

    keyCodeType      key;
    std::string_view name;
    EventType        event_pressed;
    EventType        event_released;
};

enum class MouseButtons : Uint8
{
    button_left = 1,
    button_middle,
    button_right
};

struct bindMouse
{
    bindMouse(MouseButtons b, std::string_view s, EventType pressed,
              EventType released)
        : button(b)
        , name(s)
        , event_pressed(pressed)
        , event_released(released)
    {
    }

    MouseButtons     button;
    std::string_view name;
    EventType        event_pressed;
    EventType        event_released;
};

static bool check_input(const SDL_Event& e, const bind*& result)
{
    static const std::array<bind, 7 /*8*/> keys{
        { { SDL_SCANCODE_W, "up", EventType::up_pressed,
            EventType::up_released },
          { SDL_SCANCODE_A, "left", EventType::left_pressed,
            EventType::left_released },
          { SDL_SCANCODE_S, "down", EventType::down_pressed,
            EventType::down_released },
          { SDL_SCANCODE_D, "right", EventType::right_pressed,
            EventType::right_released },
          { SDL_SCANCODE_SPACE, "button1", EventType::button1_pressed,
            EventType::button1_released },
          //      { SDLK_SPACE, "button2", event::button2_pressed,
          //        event::button2_released },
          { SDL_SCANCODE_ESCAPE, "select", EventType::select_pressed,
            EventType::select_released },
          { SDL_SCANCODE_RETURN, "start", EventType::start_pressed,
            EventType::start_released } }
    };

    using namespace std;
    const auto& searchingCollection = keys;

    const auto it =
        find_if(begin(searchingCollection), end(searchingCollection),
                [e](const bind& b) { return b.key == e.key.keysym.scancode; });

    if (it != end(searchingCollection))
    {
        result = &(*it);
        return true;
    }
    return false;
}

static bool check_input(const SDL_Event& e, const bindMouse*& result)
{
    static const std::array<bindMouse, 1> mouseButtons{
        { /*{ MouseButtons::button_right, "button1", EventType::button1_pressed,
            EventType::button1_released },*/
          { MouseButtons::button_middle, "button2", EventType::button2_pressed,
            EventType::button2_released } }
    };
    using namespace std;
    const auto& searchingCollection = mouseButtons;
    const auto  it =
        find_if(begin(searchingCollection), end(searchingCollection),
                [e](const bindMouse& b) {
                    return static_cast<Uint8>(b.button) == e.button.button;
                });

    if (it != end(searchingCollection))
    {
        result = &(*it);
        return true;
    }
    return false;
}

/// create main window
/// on success return empty string
std::string EngineSdl::initialize(std::string_view windowName,
                                  std::string_view /*config*/)
{
    std::stringstream serr{};
    auto              isInitSdl = initSdl(serr);
    if (!isInitSdl)
    {
        return serr.str();
    }

    auto isInitSdlWindow = initSdlWindow(windowName, serr);
    if (!isInitSdlWindow)
    {
        return serr.str();
    }

    auto isInitOpenGlContext = initOpenGl(serr);
    if (!isInitOpenGlContext)
    {
        return serr.str();
    }

    // Now load and ES and Core functions
    auto isLoadGLFunctionsPointers = loadGLFunctionsPointers(serr);
    if (!isLoadGLFunctionsPointers)
    {
        return serr.str();
    }

    // Debugging
#ifdef DEBUG_CONFIGURATION
    setDebugOpenGl();
#endif

    // enabling VAO. Renderdoc only can work with VAO enabled
    auto isInitVertexBufferAndVAO = initBuffersAndVAO(serr);
    if (!isInitVertexBufferAndVAO)
    {
        return serr.str();
    }

    setAdditionalGlParameters();

    if (!m_imguiEngine.initialize(m_window, this))
    {
        serr << "Cannot init imgui" << std::endl;
        return serr.str();
    }

    if (!m_audioEngine.initialize())
    {
        serr << "Cannot init audio" << std::endl;
        return serr.str();
    }

    return serr.str();
}

bool EngineSdl::initSdl(std::stringstream& serr)
{
    SDL_version compiled = { 0, 0, 0 };
    SDL_version linked   = { 0, 0, 0 };

    SDL_VERSION(&compiled)
    SDL_GetVersion(&linked);

    if (SDL_COMPILEDVERSION !=
        SDL_VERSIONNUM(linked.major, linked.minor, linked.patch))
    {
        std::cerr << "warning: SDL2 compiled and linked version mismatch: "
                  << compiled << " " << linked << std::endl;
    }

    const int init_result = SDL_Init(SDL_INIT_EVERYTHING);
    if (init_result != 0)
    {
        const char* err_message = SDL_GetError();
        serr << "error: failed call SDL_Init: " << err_message << std::endl;
        return false;
    }
    return true;
}

bool EngineSdl::initSdlWindow(std::string_view   windowName,
                              std::stringstream& serr)
{
    SDL_Window* const window = SDL_CreateWindow(
        windowName.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        initialWpixels, initialHpixels, creatingSdlWindowFlags);

    if (window == nullptr)
    {
        const char* err_message = SDL_GetError();
        serr << "error: failed call SDL_CreateWindow: " << err_message
             << std::endl;
        SDL_Quit();
        return false;
    }

    m_windowId = SDL_GetWindowID(window);
    if (m_windowId == 0)
    {
        serr << "Cannot find sdl window id" << SDL_GetError() << std::endl;
    }

    // hardcode display to initial display
    int displayIndex;
    if (!findDisplayIndex(window, displayIndex, serr))
    {
        return false;
    }

    m_displayIndex = displayIndex;
    m_window       = window;

    if (!findSdlWindowDpi(serr))
    {
        return false;
    }

    if (!findDisplayPixelSizeAndRefreshRate(serr))
    {
        return false;
    }

    return true;
}

bool EngineSdl::findDisplayIndex(SDL_Window* window, int& r_displayIndex,
                                 std::stringstream& serr)
{
    r_displayIndex = SDL_GetWindowDisplayIndex(window);

    if (r_displayIndex < 0)
    {
        serr << "Cannot find display index" << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

/// r_dpis[0] - horyzontal(width) dpi, [1] - vertical (height) dpi, [2] -
/// diagonal dpi
bool EngineSdl::findSdlWindowDpi(std::stringstream& serr)
{
    [[maybe_unused]] float ddpi;
    float                  hdpi;
    float                  vdpi;

    if (SDL_GetDisplayDPI(m_displayIndex, &ddpi, &hdpi, &vdpi))
    {
        serr << "Cant get sdl window DPI" << SDL_GetError() << std::endl;
        return false;
    }

    m_displayWidthDpi  = static_cast<myGlfloat>(hdpi);
    m_displayHeightDpi = static_cast<myGlfloat>(vdpi);

    return true;
}

bool EngineSdl::findDisplayPixelSizeAndRefreshRate(std::stringstream& serr)
{
    SDL_DisplayMode dispMode;
    auto            getDispModeResult =
        SDL_GetCurrentDisplayMode(m_displayIndex, &dispMode);

    if (getDispModeResult != 0)
    {
        serr << "Cannot find sdl window id" << SDL_GetError() << std::endl;
        return false;
    }

    m_displayPixelWidth  = dispMode.w;
    m_displayPixelHeight = dispMode.h;
    m_displayRefreshRate = dispMode.refresh_rate;
    return true;
}

static constexpr std::string_view shaderVersionEsLine{ "#version 300 es\n" };
static constexpr std::string_view shaderVersionCoreLine{
    "#version 330 core\n"
};

bool EngineSdl::initOpenGl(std::stringstream& serr)
{
    // set debug
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // version and profile for openGL ES 3.2. This wont work because Core and ES
    // shaders arent compatible
    int gl_major_ver       = 3;
    int gl_minor_ver       = 0;
    int gl_context_profile = SDL_GL_CONTEXT_PROFILE_ES;

    // checking for supporting platform
    std::string_view platform = SDL_GetPlatform();
    using namespace std::string_view_literals;

    auto list = { "Linux"sv, "Windows"sv, "Mac OS X"sv };
    auto it   = std::find(std::begin(list), std::end(list), platform);
    if (it != std::end(list))
    {
        // We can use RenderDoc + text debug if can init OpenGl Core 4.3.
        // OpenGL ES cant work with RenderDoc so use CORE if possible
        gl_major_ver       = 4;
        gl_minor_ver       = 3;
        gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
    }

    // enabling multisampling for smoothing

    auto isSetAccelerated = SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    //    auto isSetMultiSampleBuffers =
    //        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //    auto isSetMultiSamples =
    //    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    if (/*(isSetMultiSampleBuffers != 0) || (isSetMultiSamples != 0) ||*/
        (isSetAccelerated != 0))
    {
        serr << "Cannot set attribute antialiasing smoothing group "
             << SDL_GetError() << std::endl;
        return false;
    }

    // set version and profile for openGl. If current profile ES (Android, IOS)
    // - we cant use renderdoc
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_ver);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_ver);

    // Try set profile finded profile
    SDL_GLContext gl_context = SDL_GL_CreateContext(m_window);

    // try set core 3.3 if gl_context havent init yet
    if (!gl_context)
    {
        // try to create Core context with lower version. Only Renderdoc is
        // available if Core 3.3 Version
        gl_major_ver       = 3;
        gl_minor_ver       = 3;
        gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_ver);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_ver);
        gl_context = SDL_GL_CreateContext(m_window);
    }

    if (!gl_context)
    {
        serr << "Cannot create context. Maybe current platform is not "
                "supported. Critical error."
             << SDL_GetError() << std::endl;
        return false;
    }

    // check created version
    int gl_major_ver_get;
    int gl_minor_ver_get;
    int gl_context_ver_get;

    int resultMajor =
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major_ver_get);

    int resultMinor =
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor_ver_get);

    int resultProfile =
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &gl_context_ver_get);

    if (resultMinor != 0 || resultMajor != 0 || resultProfile != 0)
    {
        serr << "Cannot get version profile. Critical error.";
        return false;
    }

    if (gl_context_ver_get != gl_context_profile)
    {
        serr << "current context profile: " << gl_context_ver_get
             << "demand context profile: " << gl_context_profile << std::endl
             << "(Core - 1, ES - 4, Compatibility - 2)" << std::flush;
        return false;
    }

    if ((gl_major_ver_get < gl_major_ver) || (gl_minor_ver_get < gl_minor_ver))
    {
        serr << "current context opengl version: " << gl_major_ver_get << '.'
             << gl_minor_ver_get << '\n'
             << "need opengl version at least: " << gl_major_ver << '.'
             << gl_minor_ver_get << '.' << std::flush;
        return false;
    }

    if (gl_context_ver_get == SDL_GL_CONTEXT_PROFILE_ES)
    {
        m_shaderVersionLine = shaderVersionEsLine;
    }
    else
    {
        m_shaderVersionLine = shaderVersionCoreLine;
    }

    // Vertical sync
    const auto swapIntervalResult = SDL_GL_SetSwapInterval(1);

    if (swapIntervalResult != 0)
    {
        serr << "Cannot set mode of swap window" << std::endl;
        return false;
    }

    m_glContext = gl_context;

    return true;
}

bool EngineSdl::loadGLFunctionsPointers(std::stringstream& serr)
{
    // get pointers to opengl functions
    try
    {
        getGlFunctionPointer("glGetError", glad_glGetError);
        getGlFunctionPointer("glCreateShader", glad_glCreateShader);
        getGlFunctionPointer("glShaderSource", glad_glShaderSource);
        getGlFunctionPointer("glCompileShader", glad_glCompileShader);
        getGlFunctionPointer("glGetShaderiv", glad_glGetShaderiv);
        getGlFunctionPointer("glGetShaderInfoLog", glad_glGetShaderInfoLog);
        getGlFunctionPointer("glDeleteShader", glad_glDeleteShader);
        getGlFunctionPointer("glCreateProgram", glad_glCreateProgram);
        getGlFunctionPointer("glAttachShader", glad_glAttachShader);
        getGlFunctionPointer("glBindAttribLocation", glad_glBindAttribLocation);
        getGlFunctionPointer("glLinkProgram", glad_glLinkProgram);
        getGlFunctionPointer("glGetProgramiv", glad_glGetProgramiv);
        getGlFunctionPointer("glGetProgramInfoLog", glad_glGetProgramInfoLog);
        getGlFunctionPointer("glDeleteProgram", glad_glDeleteProgram);
        getGlFunctionPointer("glUseProgram", glad_glUseProgram);
        getGlFunctionPointer("glVertexAttribPointer",
                             glad_glVertexAttribPointer);
        getGlFunctionPointer("glEnableVertexAttribArray",
                             glad_glEnableVertexAttribArray);
        getGlFunctionPointer("glDisableVertexAttribArray",
                             glad_glDisableVertexAttribArray);
        getGlFunctionPointer("glValidateProgram", glad_glValidateProgram);
        getGlFunctionPointer("glBindBuffer", glad_glBindBuffer); // for VAO

        getGlFunctionPointer("glGenBuffers", glad_glGenBuffers); // for VAO

        getGlFunctionPointer("glGenVertexArrays",
                             glad_glGenVertexArrays); // for VAO

        getGlFunctionPointer("glBindVertexArray",
                             glad_glBindVertexArray); // for VAO

        getGlFunctionPointer("glBufferData", glad_glBufferData); // for VAO

        getGlFunctionPointer("glDrawArrays", glad_glDrawArrays);
        getGlFunctionPointer("glDrawElements", glad_glDrawElements);
        getGlFunctionPointer("glClear", glad_glClear);
        getGlFunctionPointer("glClearColor", glad_glClearColor);

        getGlFunctionPointer("glGetUniformLocation", glad_glGetUniformLocation);
        getGlFunctionPointer("glUniform4fv", glad_glUniform4fv);
        getGlFunctionPointer("glUniform3fv", glad_glUniform3fv);
        getGlFunctionPointer("glUniform2fv", glad_glUniform2fv);
        getGlFunctionPointer("glUniform1f", glad_glUniform1f);
        getGlFunctionPointer("glUniform1i", glad_glUniform1i);

        getGlFunctionPointer("glUniformMatrix2fv", glad_glUniformMatrix2fv);
        getGlFunctionPointer("glUniformMatrix2x3fv", glad_glUniformMatrix2x3fv);
        getGlFunctionPointer("glUniformMatrix2x3fv", glad_glUniformMatrix2x4fv);
        getGlFunctionPointer("glUniformMatrix3x2fv", glad_glUniformMatrix3x2fv);
        getGlFunctionPointer("glUniformMatrix3fv", glad_glUniformMatrix3fv);
        getGlFunctionPointer("glUniformMatrix3x4fv", glad_glUniformMatrix3x4fv);
        getGlFunctionPointer("glUniformMatrix4x2fv", glad_glUniformMatrix4x2fv);
        getGlFunctionPointer("glUniformMatrix4x3fv", glad_glUniformMatrix4x3fv);
        getGlFunctionPointer("glUniformMatrix4fv", glad_glUniformMatrix4fv);

        getGlFunctionPointer("glLineWidth", glad_glLineWidth);

        getGlFunctionPointer("glLineWidth", glad_glLineWidth);

        getGlFunctionPointer("glEnable", glad_glEnable);

        getGlFunctionPointer("glHint", glad_glHint);

        getGlFunctionPointer("glBlendFunc", glad_glBlendFunc);

        getGlFunctionPointer("glDeleteProgram", glad_glDeleteProgram);

        getGlFunctionPointer("glActiveTexture", glad_glActiveTexture);
        getGlFunctionPointer("glGenTextures", glad_glGenTextures);
        getGlFunctionPointer("glBindTexture", glad_glBindTexture);
        getGlFunctionPointer("glTexImage2D", glad_glTexImage2D);
        getGlFunctionPointer("glGenerateMipmap", glad_glGenerateMipmap);
        getGlFunctionPointer("glTexParameteri", glad_glTexParameteri);
        getGlFunctionPointer("glTexImage2D", glad_glTexImage2D);
        getGlFunctionPointer("glTexImage2D", glad_glTexImage2D);
        getGlFunctionPointer("glDeleteTextures", glad_glDeleteTextures);

        getGlFunctionPointer("glViewport", glad_glViewport);

        getGlFunctionPointer("glGetIntegerv", glad_glGetIntegerv);
    }
    catch (std::runtime_error& ex)
    {
        serr << "Critical exception: " << ex.what() << std::endl;
        return false;
    }
    // For debug
    try
    {
        getGlFunctionPointer("glDebugMessageCallback",
                             glad_glDebugMessageCallback);
        getGlFunctionPointer("glDebugMessageControl",
                             glad_glDebugMessageControl);
    }
    catch (std::runtime_error& ex)
    {
        std::cerr << "Noncritical exception: " << ex.what() << std::endl
                  << "text debug is unable" << std::endl;
    }

    return true;
}

bool EngineSdl::initBuffersAndVAO(std::stringstream& serr)
{

    // init internal Vertex Buffer
    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    auto isGenVertexBuffer = isGlResultOk();
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    auto isBindVertexBuffer = isGlResultOk();

    // init VAO (vertex array object)
    GLuint vertex_array_object = 0;
    glGenVertexArrays(1, &vertex_array_object);
    auto isGenVertexArrays = isGlResultOk();
    glBindVertexArray(vertex_array_object);
    auto isBindVertexArrays = isGlResultOk();

    auto isInitIndexBuffer = initIndexBuffer(serr);

    if (!(isGenVertexBuffer && isBindVertexBuffer && isInitIndexBuffer &&
          isGenVertexArrays && isBindVertexArrays))
    {
        serr << "Cannot init buffers or VAO." << std::endl;
    }

    return true;
}

bool EngineSdl::initIndexBuffer(std::stringstream& serr)
{
    // init internal Vertex Buffer
    GLuint index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    auto isGenBuffers = isGlResultOk();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    auto isBindBuffers = isGlResultOk();
    if (!(isGenBuffers && isBindBuffers))
    {
        serr << "Cannot create index buffer." << std::endl;
    }
    return true;
}

void EngineSdl::setDebugOpenGl()
{
    if (glEnable && glDebugMessageCallback && glDebugMessageControl)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback_opengl_debug, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              nullptr, GL_TRUE);
    }
    else
    {
        std::cerr << "Warning. Debug functions pointers arent init.";
    }
}

void EngineSdl::setAdditionalGlParameters()
{
    // set size of the openGL window. It haven't be there, but if the window is
    // float-sized it is necessary.
    glViewport(0, 0, initialWpixels, initialHpixels);
    isGlResultOk();

    // Enabling Z-buffer
    // glEnable(GL_DEPTH_TEST);
    // isGlResultOk();

    // glEnable(GL_MULTISAMPLE);
    // isGlResultOk();

    // endble blending for mix colours between points
    glEnable(GL_BLEND);
    isGlResultOk();
    // GL_SRC_ALPHA - mean that current buffer color will multiply currrent
    // alpha. GL_ONE_MINUS_SRC_ALPHA - koef for already contained for this pixel
    // alpha
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    isGlResultOk();
}

/// pool event from input queue
/// return true if more events in queue
bool EngineSdl::read_input(Event& e)
{
    using namespace std;
    // collect all events from SDL
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        m_imguiEngine.processEvent(&sdl_event);

        if (sdl_event.type == SDL_QUIT)
        {
            e.type = EventType::turn_off;
            return true;
        }
        else if ((sdl_event.type == SDL_KEYDOWN) && (sdl_event.key.repeat == 0))
        {
            const bind* binding = nullptr;
            if (check_input(sdl_event, binding))
            {
                e.type = binding->event_pressed;
                return true;
            }
        }
        else if (sdl_event.type == SDL_KEYUP)
        {
            const bind* binding = nullptr;
            if (check_input(sdl_event, binding))
            {
                e.type = binding->event_released;
                return true;
            }
        }
        // for mouse text mode is not used so we can not check repeat
        else if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
        {
            const bindMouse* binding = nullptr;
            if (check_input(sdl_event, binding))
            {
                e.type = binding->event_pressed;
                return true;
            }
        }
        else if (sdl_event.type == SDL_MOUSEBUTTONUP)
        {
            const bindMouse* binding = nullptr;
            if (check_input(sdl_event, binding))
            {
                e.type = binding->event_released;
                return true;
            }
        }
        else if (sdl_event.type == SDL_MOUSEMOTION)
        {
            int w = 0, h = 0;
            SDL_GetWindowSize(m_window, &w, &h);

            // SDL to OpenGL coordinates
            auto mouse_x =
                static_cast<myGlfloat>(sdl_event.motion.x / (w / 2.0) - 1);
            ;
            auto mouse_y =
                -static_cast<myGlfloat>(sdl_event.motion.y / (h / 2.0) - 1);

            e.type          = EventType::cursor_motion;
            e.parameters.p0 = mouse_x;
            e.parameters.p1 = mouse_y;
            return true;
        }
        else if (sdl_event.type == SDL_MOUSEWHEEL)
        {
            myGlfloat scaleCoef = 100.0;
            // TODO finish implementation
            auto wheel_shift_x =
                static_cast<myGlfloat>(sdl_event.wheel.x / scaleCoef);
            auto wheel_shift_y =
                static_cast<myGlfloat>(sdl_event.wheel.y / scaleCoef);
            e.type          = EventType::wheel_rolled;
            e.parameters.p0 = wheel_shift_x;
            e.parameters.p1 = wheel_shift_y;
            return true;
        }
        else if (sdl_event.type == SDL_WINDOWEVENT)
        {
            if (sdl_event.window.windowID == m_windowId)
                if (sdl_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    GLint width  = sdl_event.window.data1;
                    GLint height = sdl_event.window.data2;

                    resizeSdlGlWindow(width, height);
                }
        }
    }
    return false;
}

void EngineSdl::uninitialize()
{
    m_audioEngine.shutdown();
    m_imguiEngine.shutdown();
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

ProgramId EngineSdl::addProgram(
    const std::string_view& vertexShaderFileName,
    const std::string_view& fragmentShaderFileName,
    const std::vector<std::pair<myUint, std::string_view>>& attributes)
{

    const ProgramId thisId{ lastProgramId.id + 1 };

    GlProgram  tempGlProg{ vertexShaderFileName, fragmentShaderFileName,
                          attributes, m_shaderVersionLine };
    const auto constructionResult =
        m_programs.emplace(thisId, std::move(tempGlProg));
    if (constructionResult.second)
        ++lastProgramId.id;
    return (constructionResult.second) ? thisId : ProgramId{};
}

bool EngineSdl::eraseProgram(ProgramId programId)
{
    const auto numberOfDeleted = m_programs.erase(programId);
    return numberOfDeleted == 1;
}

TextureId EngineSdl::addTexture(const std::string_view& pathToTexture)
{
    ++lastTextureId.id;
    const auto thisId{ lastTextureId };

    GlTexture  tempGlTexture{ pathToTexture };
    const auto constructionResult =
        m_textures.emplace(thisId, std::move(tempGlTexture));

    return (constructionResult.second) ? thisId : TextureId{};
}

TextureId EngineSdl::addTexture(const uint_least8_t* const pixels,
                                const size_t w, const size_t h)
{
    ++lastTextureId.id;
    const auto thisId{ lastTextureId };

    GlTexture  tempGlTexture{ pixels, w, h };
    const auto constructionResult =
        m_textures.emplace(thisId, std::move(tempGlTexture));

    return (constructionResult.second) ? thisId : TextureId{};
}

bool EngineSdl::eraseTexture(TextureId textureId)
{
    const auto numberOfDeleted = m_textures.erase(textureId);
    return numberOfDeleted == 1;
}

bool EngineSdl::setCurrentDefaultProgram(ProgramId programId)
{
    m_currentProgram = findProgram(programId);
    if (!m_currentProgram)
    {
        return false;
    }
    return true;
}

bool EngineSdl::setUniform(std::string_view uniformName, myGlfloat parameters,
                           ProgramId programId)
{
    auto glprogram = findProgram(programId);
    if (!glprogram)
    {
        return false;
    }
    return glprogram->setUniform(uniformName, parameters);
}

bool EngineSdl::setUniform(std::string_view       uniformName,
                           std::vector<myGlfloat> parameters,
                           ProgramId              programId)
{
    auto glprogram = findProgram(programId);
    if (!glprogram)
    {
        return false;
    }
    return glprogram->setUniform(uniformName, parameters);
}

bool EngineSdl::renderClearWindow(Color color)
{
    glClearColor(color.get_r(), color.get_g(), color.get_b(), color.get_a());
    auto clearColorResult = isGlResultOk();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto clearResult = isGlResultOk();
    return clearColorResult && clearResult;
}

void EngineSdl::render(
    const std::vector<VertexWide>& vertices, const std::vector<myUint>& indices,
    const std::vector<TextureId>&        textureIds,
    const std::vector<std::string_view>& textureAttributesNames,
    const Matrix<3, 3>& moveMatrix, const std::string_view moveUnifromName,
    ProgramId programId, ShapeType type)
{
    auto glprogram = findProgram(programId);
    glprogram->setUniform(moveUnifromName, moveMatrix);
    renderTexturedInternal(vertices, indices, textureIds,
                           textureAttributesNames, type, programId);
}

void EngineSdl::render(
    const std::vector<VertexTextured>&   vertices,
    const std::vector<myUint>&           indices,
    const std::vector<TextureId>&        textureIds,
    const std::vector<std::string_view>& textureAttributesNames,
    const Matrix<3, 3>& moveMatrix, const std::string_view moveUnifromName,
    ProgramId programId, ShapeType type)
{
    auto glprogram = findProgram(programId);
    glprogram->setUniform(moveUnifromName, moveMatrix);
    renderTexturedInternal(vertices, indices, textureIds,
                           textureAttributesNames, type, programId);
}

void EngineSdl::render(const std::vector<VertexMorphed>& vertices,
                       const std::vector<myUint>&        indices,
                       const Matrix<3, 3>&               moveMatrix,
                       const std::string_view            moveUnifromName,
                       ProgramId programId, ShapeType type)
{
    auto glprogram = findProgram(programId);
    glprogram->setUniform(moveUnifromName, moveMatrix);
    renderInternal(vertices, indices, type, programId);
}

void EngineSdl::render(const std::vector<Vertex>& vertices,
                       const std::vector<myUint>& indices,
                       const Matrix<3, 3>&        moveMatrix,
                       const std::string_view     moveUnifromName,
                       ProgramId programId, ShapeType type)
{
    auto glprogram = findProgram(programId);
    glprogram->setUniform(moveUnifromName, moveMatrix);
    renderInternal(vertices, indices, type, programId);
}

void EngineSdl::render(
    const std::vector<VertexWide>& vertices, const std::vector<myUint>& indices,
    const std::vector<TextureId>&        textureIds,
    const std::vector<std::string_view>& textureAttributesNames,
    ProgramId programId, ShapeType type)
{
    renderTexturedInternal(vertices, indices, textureIds,
                           textureAttributesNames, type, programId);
}

void EngineSdl::render(
    const std::vector<VertexTextured>&   vertices,
    const std::vector<myUint>&           indices,
    const std::vector<TextureId>&        textureIds,
    const std::vector<std::string_view>& textureAttributesNames,
    ProgramId programId, ShapeType type)
{
    renderTexturedInternal(vertices, indices, textureIds,
                           textureAttributesNames, type, programId);
}

void EngineSdl::render(const std::vector<VertexMorphed>& vertices,
                       const std::vector<myUint>& indices, ProgramId programId,
                       ShapeType type)
{
    renderInternal(vertices, indices, type, programId);
}

void EngineSdl::render(const std::vector<Vertex>& vertices,
                       const std::vector<myUint>& indices, ProgramId programId,
                       ShapeType type)
{
    renderInternal(vertices, indices, type, programId);
}

void EngineSdl::renderTriangle(const Triangle<Vertex>& t, ProgramId programId)
{
    renderTriangleInternal(t, programId);
}

static bool isUiNewFrameEvoked{};

bool EngineSdl::updateWindow(Color fillingColor)
{
    if (isUiNewFrameEvoked)
        uiRender();
    isUiNewFrameEvoked = false;
    SDL_GL_SwapWindow(m_window);
    renderClearWindow(fillingColor);
    return isGlResultOk();
}

ISoundTrack* EngineSdl::addSoundTrack(std::string_view path)
{
    return m_audioEngine.addSoundTrack(path);
}

void EngineSdl::eraseSoundTrack(ISoundTrack* soundTrack)
{
    m_audioEngine.eraseSoundTrack(soundTrack);
}

ISoundBuffer* EngineSdl::addSoundBuffer(ISoundTrack* soundTrack)
{
    return m_audioEngine.addSoundBuffer(soundTrack);
}

void EngineSdl::playSoundBufferOnce(ISoundTrack*  soundTrack,
                                    om::myGlfloat volume,
                                    om::myGlfloat leftRightBalance,
                                    om::myGlfloat stereo)
{
    m_audioEngine.playSoundBufferOnce(soundTrack, volume, leftRightBalance,
                                      stereo);
}

void EngineSdl::eraseSoundBuffer(ISoundBuffer* soundBuffer)
{
    m_audioEngine.eraseSoundBuffer(soundBuffer);
}

void EngineSdl::uiNewFrame()
{
    m_imguiEngine.newFrame();
    isUiNewFrameEvoked = true;
}

void EngineSdl::uiRender()
{
    ImGui::Render();
}

GlProgram* EngineSdl::findProgram(ProgramId programId)
{
    auto outProgramPtr{ m_currentProgram };
    auto programIt = m_programs.find(programId);
    if (programIt != m_programs.end())
    {
        outProgramPtr = &programIt->second;
    }
    if (!outProgramPtr)
    {
        std::cerr << "Attempt to find program " << programId.id << " failed";
    }
    return outProgramPtr;
}

GlTexture* EngineSdl::findTexture(TextureId textureId)
{
    auto outProgramPtr{ m_currentTexture };
    auto textureIt = m_textures.find(textureId);
    if (textureIt != m_textures.end())
    {
        outProgramPtr = &textureIt->second;
    }
    if (!outProgramPtr)
    {
        std::cerr << "Attempt to find texture " << textureId.id << " failed";
    }
    return outProgramPtr;
}

template <typename T, typename>
void EngineSdl::renderTexturedInternal(
    const std::vector<T>& vertices, const std::vector<myUint>& indices,
    std::vector<TextureId>        textureIds,
    std::vector<std::string_view> textureAttributesNames, ShapeType type,
    ProgramId programId)
{

    auto                    glprogram = findProgram(programId);
    std::vector<GlTexture*> textures;
    textures.reserve(textureIds.size());

    auto findTextureWrap = [this](const TextureId& textureId) {
        return findTexture(textureId);
    };

    std::transform(std::begin(textureIds), std::end(textureIds),
                   std::back_inserter(textures), findTextureWrap);

    glprogram->setTextures(textureAttributesNames, textures);
    renderInternal(vertices, indices, type, programId);
    glprogram->resetTextures();
}

template <typename T, typename>
void EngineSdl::renderTriangleInternal(const Triangle<T>& t,
                                       ProgramId          programId)
{
    const std::vector<T>      vertices{ &t.v[0], &t.v[3] };
    const std::vector<myUint> indices{ 0, 1, 2 };
    const auto                type = ShapeType::triangle;
    renderInternal(vertices, indices, type, programId);
}

template <typename T, typename>
void EngineSdl::renderInternal(const std::vector<T>&      vertices,
                               const std::vector<myUint>& indices,
                               ShapeType type, ProgramId programId)
{

    auto glprogram = findProgram(programId);
    if (!glprogram)
    {
        return;
    }
    glprogram->use();
    // Only for Core profile setting to draw only skeletons of polygones
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // That unefficient because we should save our all our vertexes once and
    // after only use indeces buffer

    const auto vertexNumber    = static_cast<int>(type);
    const auto numberOfIndeces = indices.size();
    if ((numberOfIndeces % vertexNumber) != 0)
    {
        std::cerr
            << "Error draw triangles. Number of indices should multiply 3."
            << std::endl;
    }

    // Fill internal OpenGL Vertices buffer with triangle
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(),
                 GL_STATIC_DRAW);
    isGlResultOk();

    // Fill internal OpenGL indices buffer with triangle
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numberOfIndeces * sizeof(myUint),
                 indices.data(), GL_STATIC_DRAW);

    isGlResultOk();

    for (size_t count{ 0 }; count < T::attributesNumbers.size(); ++count)
    {
        const auto currentAttributeNumber     = T::attributesNumbers[count];
        const auto currentAttributeByteOffset = T::attributesByteOffsets[count];
        const auto currentAttributeAmount     = T::attributesAmounts[count];

        const auto currentAttributeGlType =
            (currentAttributeNumber != om::Vertex::colorAttributeNumber)
                ? GL_FLOAT
                : GL_UNSIGNED_BYTE;
        const auto isNeedNormalization =
            (currentAttributeNumber != om::Vertex::colorAttributeNumber)
                ? GL_FALSE
                : GL_TRUE;

        glEnableVertexAttribArray(currentAttributeNumber);
        isGlResultOk();

        glVertexAttribPointer(
            currentAttributeNumber, currentAttributeAmount,
            currentAttributeGlType, isNeedNormalization, sizeof(T),
            reinterpret_cast<const void*>(currentAttributeByteOffset));

        isGlResultOk();
    }

    if (!glprogram->validate())
    {
        throw std::runtime_error("error");
    }

    GLenum shapeGlType = 0;
    switch (type)
    {
        case ShapeType::line:
            shapeGlType = GL_LINES;
            glLineWidth(1);
            break;
        case ShapeType::triangle:
            shapeGlType = GL_TRIANGLES;
            glLineWidth(1);
            break;
        case ShapeType::triangle_strip:
            shapeGlType = GL_TRIANGLE_STRIP;
            glLineWidth(1);
            break;
    }
    const GLsizei indicesDataType = GL_UNSIGNED_INT;
    glDrawElements(shapeGlType, numberOfIndeces, indicesDataType, nullptr);
    isGlResultOk();

    auto disable_gl_attribute = [](myUint attributeNumber) {
        glDisableVertexAttribArray(attributeNumber);
        isGlResultOk();
    };

    std::for_each_n(std::begin(T::attributesNumbers),
                    T::attributesNumbers.size(), disable_gl_attribute);
}

std::array<int, 2> EngineSdl::getDrawablePixelSize()
{

    GLint viewPort[4];

    glGetIntegerv(GL_VIEWPORT, viewPort);

    //    int wPixels;
    //    int hPixels;
    //    SDL_GetWindowSize(m_window, &wPixels, &hPixels);

    return { viewPort[2], viewPort[3] };
}

std::array<myGlfloat, 2> EngineSdl::getDrawableInchesSize()
{
    auto pixelSize = getDrawablePixelSize();

    std::array<myGlfloat, 2> windowSize{
        static_cast<myGlfloat>(pixelSize[0] * m_displayWidthDpi),
        static_cast<myGlfloat>(pixelSize[1] * m_displayHeightDpi)
    };
    return windowSize;
}

std::array<int, 2> EngineSdl::getDisplayPixelSize()
{
    return { m_displayPixelWidth, m_displayPixelHeight };
}

int EngineSdl::getDisplayRefreshRate()
{
    return m_displayRefreshRate;
}

bool EngineSdl::resizeSdlGlWindow(const int w, const int h)
{
    const auto sdlWindowXtoYScale = static_cast<double>(w) / h;

    if (sdlWindowXtoYScale < baseScaleXtoY)
    {
        GLint newH   = w / baseScaleXtoY;
        GLint yShift = (h - newH) / 2;
        glViewport(0, yShift, w, newH);
    }
    else
    {
        GLint newW   = h * baseScaleXtoY;
        GLint xShift = (w - newW) / 2;
        glViewport(xShift, 0, newW, h);
    }

    return isGlResultOk();
}

} // namespace om
