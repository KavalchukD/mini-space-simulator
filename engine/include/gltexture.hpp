#pragma once
#include "glad.h"
#include <sstream>
#include <string_view>
#include <vector>

namespace om
{
class GlTexture
{
public:
    explicit GlTexture(const std::string_view filePath);

    GlTexture(const uint_least8_t* const pixels, const size_t w,
              const size_t h);

    ~GlTexture();

    GlTexture(const GlTexture&) = delete;

    GlTexture& operator=(const GlTexture&) = delete;

    explicit GlTexture(GlTexture&& srcTexture);

    GlTexture& operator=(GlTexture&& srcTexture);

    bool use();
    bool getW() { return m_w; }
    bool getH() { return m_h; }

private:
    bool loadTexture(const std::string_view filePath);
    bool generateOpenGlTexture(const uint_least8_t* const textureDecodedData,
                               const unsigned long        wTexture,
                               const unsigned long        hTexture,
                               GLenum& r_textureType, GLuint& r_textureId);

    GLuint m_textureGlId{};
    GLenum m_textureGlType{};
    size_t m_w{};
    size_t m_h{};

    static constexpr GLenum hardcodedTextureType{ GL_TEXTURE_2D };
};
} // namespace om
