#pragma once
#include <array>
#include <glad.h>

bool          isGlResultOk();
void APIENTRY callback_opengl_debug(GLenum source, GLenum type, GLuint id,
                                    GLenum severity, GLsizei length,
                                    const GLchar*                message,
                                    [[maybe_unused]] const void* userParam);
