#pragma once
#include "gltexture.hpp"
#include "iengine.hpp"
#include "matrix.hpp"
#include "opengl_debug.hpp"
#include <glad.h>
#include <iostream>
#include <string>
#include <vector>
namespace om
{
class GlProgram
{
public:
    explicit GlProgram(
        const std::string_view& vertexShaderFileName,
        const std::string_view& fragmentShaderFileName,
        const std::vector<std::pair<GLuint, std::string_view>>& attributes,
        const std::string_view&                                 versionLine);

    GlProgram(const GlProgram&) = delete;

    GlProgram& operator=(const GlProgram&) = delete;

    explicit GlProgram(GlProgram&& srcProgram);

    ~GlProgram();

    GlProgram& operator=(GlProgram&& srcProgram);

    bool use();
    bool setUniform(std::string_view uniformName, GLint parameters);
    bool setUniform(std::string_view uniformName, GLfloat parameters);
    bool setUniform(std::string_view            uniformName,
                    const std::vector<GLfloat>& parameters);

    bool setTextures(const std::vector<std::string_view>& textureUniformName,
                     const std::vector<GlTexture*>&       texture);
    bool setTexture(std::string_view textureUniformName, GlTexture* texture);

    template <size_t n, size_t m>
    bool setUniform(std::string_view    uniformName,
                    const Matrix<n, m>& parameters);

    void resetTextures() { m_nextTextureUnit = 0; }

    bool validate();

private:
    bool initShader(const std::string_view& shaderFileName, GLenum type,
                    GLuint& r_shader, const std::string_view& versionLine,
                    std::stringstream& serr);

    bool initProgram(
        GLuint vertexShader, GLuint fragmentShader, GLuint& programId,
        const std::vector<std::pair<GLuint, std::string_view>>& attributes,
        std::stringstream&                                      serr);

    bool createProgramAndAttachShaders(GLuint vertexShader,
                                       GLuint fragmentShader, GLuint& programId,
                                       std::stringstream& serr);

    bool bindAttributesLocationAndLinkProgram(
        const GLuint                                            programId,
        const std::vector<std::pair<GLuint, std::string_view>>& attributes,
        std::stringstream&                                      serr);

    bool preSetUniform(std::string_view uniformName, GLint& location);

    friend bool operator==(GlProgram program1, GlProgram program2)
    {
        return program1.m_programId == program2.m_programId;
    }

    friend bool operator<(GlProgram program1, GlProgram program2)
    {
        return program1.m_programId < program2.m_programId;
    }
    GLuint                  m_programId{};
    GLenum                  m_nextTextureUnit{};
    static constexpr GLenum maxNumberOfTexturesForProgram{ 16 };
};

template <size_t n, size_t m>
bool GlProgram::setUniform(std::string_view    uniformName,
                           const Matrix<n, m>& parameters)
{
    GLint location{ -1 };
    if (!preSetUniform(uniformName, location))
    {
        return false;
    }

    const auto isTranspose{ GL_FALSE };

    const myGlfloat* beginMatrixPtr{ parameters.begin()->begin() };

    if constexpr (m == 2)
    {
        if constexpr (n == 2)
        {
            glUniformMatrix2fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 3)
        {
            glUniformMatrix2x3fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 4)
        {
            glUniformMatrix2x4fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else
        {
            std::cerr << "Incorrect number of parameters when set uniform "
                      << uniformName << std::endl;
            return false;
        }
    }
    else if constexpr (m == 3)
    {
        if constexpr (n == 2)
        {
            glUniformMatrix3x2fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 3)
        {
            glUniformMatrix3fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 4)
        {
            glUniformMatrix3x4fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else
        {
            std::cerr << "Incorrect number of parameters when set uniform "
                      << uniformName << std::endl;
            return false;
        }
    }
    else if constexpr (m == 4)
    {
        if constexpr (n == 2)
        {
            glUniformMatrix4x2fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 3)
        {
            glUniformMatrix4x3fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else if constexpr (n == 4)
        {
            glUniformMatrix4fv(location, 1, isTranspose, beginMatrixPtr);
        }
        else
        {
            std::cerr << "Incorrect number of parameters when set uniform "
                      << uniformName << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Incorrect number of parameters when set uniform "
                  << uniformName << std::endl;
        return false;
    }

    return isGlResultOk();
}

} // namespace om
