#include "gltexture.hpp"
#include "opengl_debug.hpp"
#include "picopng.hxx"
#include <SDL.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace om
{
static bool loadRawDataFromFile(const std::string_view  filePath,
                                std::vector<std::byte>& rawDataFromFile);

GlTexture::GlTexture(const std::string_view filePath)
{
    if (!loadTexture(filePath))
    {
        std::cerr << "Error loading textrure. " << filePath << std::endl;
        throw std::runtime_error(
            static_cast<std::string>("Error loading textrure. ") +
            static_cast<std::string>(filePath));
    }
}

GlTexture::GlTexture(const uint_least8_t* const pixels, const size_t w,
                     const size_t h)
{
    std::string_view filePath = "::memory::";
    m_w                       = w;
    m_h                       = h;
    if (!generateOpenGlTexture(pixels, m_w, m_h, m_textureGlType,
                               m_textureGlId))
    {
        std::cerr << "Error loading textrure." << filePath << std::endl;
        throw std::runtime_error("Error loading textrure.");
    }
}

GlTexture::~GlTexture()
{
    glDeleteTextures(1, &m_textureGlId);
    isGlResultOk();
}

GlTexture::GlTexture(GlTexture&& srcTexture)
    : m_textureGlId{ srcTexture.m_textureGlId }
    , m_textureGlType{ srcTexture.m_textureGlType }
    , m_w{ srcTexture.m_w }
    , m_h{ srcTexture.m_h }
{
    srcTexture.m_textureGlId   = 0;
    srcTexture.m_textureGlType = 0;
    srcTexture.m_h             = 0;
    srcTexture.m_w             = 0;
}
GlTexture& GlTexture::operator=(GlTexture&& srcTexture)
{
    m_textureGlId              = srcTexture.m_textureGlId;
    m_textureGlType            = srcTexture.m_textureGlType;
    m_h                        = srcTexture.m_h;
    m_w                        = srcTexture.m_w;
    srcTexture.m_textureGlId   = 0;
    srcTexture.m_textureGlType = 0;
    srcTexture.m_h             = 0;
    srcTexture.m_w             = 0;
    return *this;
}

bool GlTexture::use()
{
    if (m_textureGlId == 0)
    {
        std::cerr << "Texture " << m_textureGlId << " is uninitialized"
                  << std::endl;
        return false;
    }
    glBindTexture(m_textureGlType, m_textureGlId);
    const auto isBindOk = isGlResultOk();
    if (!isBindOk)
    {
        std::cerr << "Error when init texture " << m_textureGlId << std::endl;
    }
    return isBindOk;
}

bool GlTexture::loadTexture(const std::string_view filePath)
{
    std::vector<std::byte> rawDataFromFile;
    if (!loadRawDataFromFile(filePath, rawDataFromFile))
    {
        return false;
    }
    std::vector<std::byte> decodedPng;

    auto errorDecodingPng =
        decodePNG(decodedPng, m_w, m_h, rawDataFromFile.data(),
                  rawDataFromFile.size(), false);

    if (errorDecodingPng != 0)
    {
        std::cerr << "When parsing png error N" << errorDecodingPng
                  << " has occured." << std::endl;
        return false;
    }
    const auto decodedDataPtr =
        reinterpret_cast<const uint_least8_t*>(decodedPng.data());

    if (!generateOpenGlTexture(decodedDataPtr, m_w, m_h, m_textureGlType,
                               m_textureGlId))
    {
        return false;
    }

    return true;
}

bool GlTexture::generateOpenGlTexture(
    const uint_least8_t* const textureDecodedData, const unsigned long wTexture,
    const unsigned long hTexture, GLenum& r_textureType, GLuint& r_textureId)
{
    glGenTextures(1, &r_textureId);
    const bool isGenTextureOk = isGlResultOk();
    r_textureType             = hardcodedTextureType;
    glBindTexture(r_textureType, r_textureId);
    const bool isBindTextureOk = isGlResultOk();

    const GLint mipmapLevel =
        0; // should be 0 if we want openGL to generate mipmaps
    const auto  inputTextureFormat     = GL_RGBA;
    const auto  targetFormat           = GL_RGBA;
    const GLint border                 = 0; // should be 0
    const auto  pixelParameterDataType = GL_UNSIGNED_BYTE;
    glTexImage2D(r_textureType, mipmapLevel, inputTextureFormat,
                 static_cast<GLsizei>(wTexture), static_cast<GLsizei>(hTexture),
                 border, targetFormat, pixelParameterDataType,
                 textureDecodedData);
    const bool isTexImageOk = isGlResultOk();
    // Generating mipmap
    glGenerateMipmap(r_textureType);
    const bool isGenMipmapOk = isGlResultOk();

    if (!(isGenTextureOk && isBindTextureOk && isTexImageOk && isGenMipmapOk))
    {
        std::cerr << "Error generating openGl texture." << std::endl;
        return false;
    }

    // use mag and min filters nearest - we should use textures as window size
    // and create mipmaps for small objects
    glTexParameteri(r_textureType, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
    bool isOptionsSetTextureOk = isGlResultOk();
    glTexParameteri(r_textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    isOptionsSetTextureOk |= isGlResultOk();
    // Set texture to repeat if coordinate overwhelmed
    glTexParameteri(r_textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    isOptionsSetTextureOk |= isGlResultOk();
    glTexParameteri(r_textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
    isOptionsSetTextureOk |= isGlResultOk();

    // if GL_TEXTURE_WRAP_ clamp to border we should set border color
    // float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    if (!isOptionsSetTextureOk)
    {
        std::cerr << "Cannot set texture options." << std::endl;
        return false;
    }

    return true;
}

static bool loadRawDataFromFile(const std::string_view  filePath,
                                std::vector<std::byte>& rawDataFromFile)
{
    SDL_RWops* fileSrcTexture = SDL_RWFromFile(filePath.data(), "rb");
    if (fileSrcTexture == nullptr)
    {
        std::cerr << "Can't open texture file: " << filePath << ". "
                  << SDL_GetError();
        return false;
    }

    const auto fileSize = fileSrcTexture->size(fileSrcTexture);

    rawDataFromFile.resize(fileSize);

    const auto numberOfReadenObjects = fileSrcTexture->read(
        fileSrcTexture, rawDataFromFile.data(), fileSize, 1);

    if (numberOfReadenObjects != 1)
    {
        std::cerr << "can't read all content from file: " << filePath << ". "
                  << SDL_GetError();
        fileSrcTexture->close(fileSrcTexture);
        return false;
    }

    fileSrcTexture->close(fileSrcTexture);
    return true;
}
} // namespace om
