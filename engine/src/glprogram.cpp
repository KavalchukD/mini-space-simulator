#include "glprogram.hpp"
#include "opengl_debug.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <SDL.h>

namespace om
{

GlProgram::GlProgram(
    const std::string_view& vertexShaderFileName,
    const std::string_view& fragmentShaderFileName,
    const std::vector<std::pair<GLuint, std::string_view>>& attributes,
    const std::string_view&                                 versionLine)
{
    std::stringstream serr;
    GLuint            vertexShader;
    auto              vertexShaderInitResult =
        initShader(vertexShaderFileName, GL_VERTEX_SHADER, vertexShader,
                   versionLine, serr);

    if (!vertexShaderInitResult)
    {
        std::cerr << "Cannot create shaders for program " << serr.str()
                  << std::endl;
        throw std::runtime_error("Cannot create shaders for program " +
                                 serr.str());
    }

    GLuint fragmentShader;
    auto   fragmentShaderInitResult =
        initShader(fragmentShaderFileName, GL_FRAGMENT_SHADER, fragmentShader,
                   versionLine, serr);

    if (!fragmentShaderInitResult)
    {
        std::cerr << "Cannot create shaders for program " << serr.str()
                  << std::endl;
        throw std::runtime_error("Cannot create shaders for program " +
                                 serr.str());
    }

    auto isInitProgram = initProgram(vertexShader, fragmentShader, m_programId,
                                     attributes, serr);

    if (!isInitProgram)
    {
        glDeleteProgram(m_programId);
        isGlResultOk();
        throw std::runtime_error("Cannot create program " + serr.str());
    }
}

GlProgram::GlProgram(GlProgram&& srcProgram)
    : m_programId{ srcProgram.m_programId }
{
    srcProgram.m_programId = 0;
}

GlProgram::~GlProgram()
{
    glDeleteProgram(m_programId);
    isGlResultOk();
}

GlProgram& GlProgram::operator=(GlProgram&& srcProgram)
{
    m_programId            = srcProgram.m_programId;
    srcProgram.m_programId = 0;
    return *this;
}

bool GlProgram::initShader(const std::string_view& shaderFileName, GLenum type,
                           GLuint&                 r_shader,
                           const std::string_view& versionLine,
                           std::stringstream&      serr)
{
    r_shader = glCreateShader(type);
    isGlResultOk();

    std::string shaderSrc;

    {
        SDL_RWops *fileSrcShader = SDL_RWFromFile(shaderFileName.data(), "rb");
        if (fileSrcShader == nullptr) {
            serr << "Unable open file shader: " << shaderFileName << ". "
                 << SDL_GetError() << std::endl;
            return false;
        }

        const auto fileSize = fileSrcShader->size(fileSrcShader);


        shaderSrc.resize(fileSize);

        const auto numberOfReadenObjects = fileSrcShader->read(
                fileSrcShader, shaderSrc.data(), fileSize, 1);

        if (numberOfReadenObjects != 1) {
            serr << "can't read all content from file: " << shaderFileName << ". "
                 << SDL_GetError() << std::endl;
            fileSrcShader->close(fileSrcShader);
            return false;
        }
        fileSrcShader->close(fileSrcShader);
    }

    const auto shaderSrcWithVersion{ static_cast<std::string>(versionLine) +
                                     shaderSrc };

    const auto srcShaderSrcPtr{ shaderSrcWithVersion.data() };

    glShaderSource(r_shader, 1, &srcShaderSrcPtr, nullptr);
    isGlResultOk();

    glCompileShader(r_shader);
    isGlResultOk();

    GLint compiled_status = 0;
    glGetShaderiv(r_shader, GL_COMPILE_STATUS, &compiled_status);
    isGlResultOk();

    if (compiled_status != GL_TRUE)
    {
        GLint info_len = 0;
        glGetShaderiv(r_shader, GL_INFO_LOG_LENGTH, &info_len);
        isGlResultOk();
        std::string info_chars(static_cast<size_t>(info_len), ' ');
        glGetShaderInfoLog(r_shader, info_len, nullptr, info_chars.data());
        isGlResultOk();
        glDeleteShader(r_shader);
        isGlResultOk();

        std::string shader_type_name =
            (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";

        serr << "Error compiling shader(" << shader_type_name << ")\n"
             << shaderSrcWithVersion << "\n"
             << info_chars.data();
        return false;
    }

    return true;
}

bool GlProgram::initProgram(
    GLuint vertexShader, GLuint fragmentShader, GLuint& programId,
    const std::vector<std::pair<GLuint, std::string_view>>& attributes,
    std::stringstream&                                      serr)
{
    auto isCreateProgram = createProgramAndAttachShaders(
        vertexShader, fragmentShader, programId, serr);
    auto isBindAttributes =
        bindAttributesLocationAndLinkProgram(programId, attributes, serr);

    glDeleteShader(vertexShader);
    isGlResultOk();
    glDeleteShader(fragmentShader);
    isGlResultOk();

    return (isCreateProgram && isBindAttributes);
}

bool GlProgram::createProgramAndAttachShaders(GLuint             vertexShader,
                                              GLuint             fragmentShader,
                                              GLuint&            programId,
                                              std::stringstream& serr)
{
    m_programId = glCreateProgram();
    isGlResultOk();

    if (m_programId == 0)
    {
        serr << "failed to create gl program";
        return false;
    }

    glAttachShader(programId, vertexShader);
    isGlResultOk();
    glAttachShader(programId, fragmentShader);
    isGlResultOk();
    return true;
}

bool GlProgram::bindAttributesLocationAndLinkProgram(
    const GLuint                                            programId,
    const std::vector<std::pair<GLuint, std::string_view>>& attributes,
    std::stringstream&                                      serr)
{
    // bind attributes
    for (const auto& attribute : attributes)
    {
        const auto numberOfAttribute = attribute.first;
        const auto nameOfAttribute   = attribute.second.data();
        glBindAttribLocation(programId, numberOfAttribute, nameOfAttribute);
        isGlResultOk();
    }
    // link program after binding attribute locations
    glLinkProgram(programId);
    isGlResultOk();
    // Check the link status
    GLint linked_status = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &linked_status);
    isGlResultOk();
    if (linked_status == 0)
    {
        GLint infoLen = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLen);
        isGlResultOk();
        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetProgramInfoLog(programId, infoLen, nullptr, infoLog.data());
        isGlResultOk();
        serr << "Error linking program:\n" << infoLog.data();
        return false;
    }
    return true;
}

bool GlProgram::use()
{
    if (m_programId == 0)
    {
        return false;
    }
    glUseProgram(m_programId);
    return isGlResultOk();
}

bool GlProgram::preSetUniform(std::string_view uniformName, GLint& location)
{
    if (m_programId == 0)
    {
        return false;
    }
    glUseProgram(m_programId);
    isGlResultOk();
    location = glGetUniformLocation(m_programId, uniformName.data());
    if (location < 0)
    {
        std::cerr
            << "Error. Can't get uniform location from shader. Uniform name - "
            << uniformName << ". ProgramId - " << m_programId << std::endl;
        return false;
        // throw std::runtime_error("can't get uniform location");
    }
    return isGlResultOk();
}

bool GlProgram::setUniform(std::string_view uniformName, int parameters)
{
    GLint location{ -1 };
    if (!preSetUniform(uniformName, location))
    {
        return false;
    }

    glUniform1i(location, parameters);

    return isGlResultOk();
}

bool GlProgram::setUniform(std::string_view uniformName, GLfloat parameters)
{
    GLint location{ -1 };
    if (!preSetUniform(uniformName, location))
    {
        return false;
    }

    glUniform1f(location, parameters);

    return isGlResultOk();
}

bool GlProgram::setUniform(std::string_view            uniformName,
                           const std::vector<GLfloat>& parameters)
{
    GLint location{ -1 };
    if (!preSetUniform(uniformName, location))
    {
        return false;
    }
    switch (parameters.size())
    {
        case 1:
            glUniform1fv(location, 1, parameters.data());
            break;
        case 2:
            glUniform2fv(location, 1, parameters.data());
            break;
        case 3:
            glUniform3fv(location, 1, parameters.data());
            break;
        case 4:
            glUniform4fv(location, 1, parameters.data());
            break;
        default:
            std::cerr << "Incorrect number of parameters when set uniform"
                      << uniformName << std::endl;
            return false;
    }

    return isGlResultOk();
}

bool GlProgram::setTexture(std::string_view textureUniformName,
                           GlTexture*       texture)
{
    const auto textureUnit = m_nextTextureUnit;
    if (textureUnit >= maxNumberOfTexturesForProgram)
    {
        std::cerr
            << "Max number of textures for 1 program is 16. Cannot set texture."
            << std::endl;
        return false;
    }
    ++m_nextTextureUnit;
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    isGlResultOk();

    if (!texture->use())
    {
        return false;
    }

    GLint location{ -1 };
    if (!preSetUniform(textureUniformName, location))
    {
        return false;
    }

    glUniform1i(location, textureUnit);
    isGlResultOk();
    return true;
}

bool GlProgram::setTextures(
    const std::vector<std::string_view>& textureUniformNames,
    const std::vector<GlTexture*>&       textures)
{
    if (textureUniformNames.size() != textures.size())
    {
        std::cerr << "Size of uniform names array and pointer to textures "
                     "doesnt match."
                  << std::endl;
        return false;
    }

    auto isSettingOk{ true };
    auto iteratorTextureUniformNames = std::begin(textureUniformNames);
    auto set_texture_wrap = [&iteratorTextureUniformNames, &isSettingOk,
                             this](GlTexture* texture) {
        const auto isSetTexture =
            setTexture(*iteratorTextureUniformNames, texture);
        if (!isSetTexture)
            isSettingOk = false;
        ++iteratorTextureUniformNames;
    };
    std::for_each(std::begin(textures), std::end(textures), set_texture_wrap);

    return isSettingOk;
}

bool GlProgram::validate()
{
    glValidateProgram(m_programId);
    isGlResultOk();
    // Check the validate status
    GLint validateStatus = 0;
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &validateStatus);
    isGlResultOk();
    if (validateStatus != GL_TRUE)
    {
        GLint infoLen = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLen);
        isGlResultOk();
        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetProgramInfoLog(m_programId, infoLen, nullptr, infoLog.data());
        isGlResultOk();
        std::cerr << "Error validating program:\n" << infoLog.data();
        return false;
    }
    return true;
}

} // namespace om
