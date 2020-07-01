#include "opengl_debug.hpp"
#include <iostream>

static constexpr bool enableNotifications{ false };

bool isGlResultOk()
{
    const int err = static_cast<int>(glGetError());
    if (err != GL_NO_ERROR)
    {
        switch (err)
        {
            case GL_INVALID_ENUM:
                std::cerr << "GL_INVALID_ENUM" << std::endl;
                break;
            case GL_INVALID_VALUE:
                std::cerr << "GL_INVALID_VALUE" << std::endl;
                break;
            case GL_INVALID_OPERATION:
                std::cerr << "GL_INVALID_OPERATION" << std::endl;
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
                break;
            case GL_OUT_OF_MEMORY:
                std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
                break;
        }
        return false;
    }
    return true;
}

// enum GL_DEBUG printing
static const char* source_to_strv(GLenum source)
{
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "SHADER_COMPILER";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:
            return "OTHER";
    }
    return "unknown";
}
// enum GL_DEBUG printing
static const char* type_to_strv(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "PERFORMANCE";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "PORTABILITY";
        case GL_DEBUG_TYPE_MARKER:
            return "MARKER";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "PUSH_GROUP";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "POP_GROUP";
        case GL_DEBUG_TYPE_OTHER:
            return "OTHER";
    }
    return "unknown";
}
// enum GL_DEBUG printing
static const char* severity_to_strv(GLenum severity)
{
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:
            return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return "NOTIFICATION";
    }
    return "unknown";
}

// 30Kb on my system, too much for stack
static std::array<char, GL_MAX_DEBUG_MESSAGE_LENGTH> local_log_buff;

// When error openGl call this funtion and we get detailed messages
void APIENTRY callback_opengl_debug(GLenum source, GLenum type, GLuint id,
                                    GLenum severity, GLsizei length,
                                    const GLchar*                message,
                                    [[maybe_unused]] const void* userParam)
{
    // The memory formessageis owned and managed by the GL, and should
    // onlybe considered valid for the duration of the function call.The
    // behavior of calling any GL or window system function from within
    // thecallback function is undefined and may lead to program
    // termination.Care must also be taken in securing debug callbacks for
    // use with asynchronousdebug output by multi-threaded GL
    // implementations.  Section 18.8 describes thisin further detail.

    if (enableNotifications || severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        auto& buff{ local_log_buff };
        int   num_chars =
            std::snprintf(buff.data(), buff.size(), "%s %s %d %s %.*s\n",
                          source_to_strv(source), type_to_strv(type), id,
                          severity_to_strv(severity), length, message);

        if (num_chars > 0)
        {
            // TODO use https://en.cppreference.com/w/cpp/io/basic_osyncstream
            // to fix possible data races
            // now we use GL_DEBUG_OUTPUT_SYNCHRONOUS to garantie call in main
            // thread
            std::cerr.write(buff.data(), num_chars);
        }
    }
}
