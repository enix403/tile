#include "tile/Texture.h"

#include "tile/opengl_inc.h"

namespace Tile {
    Texture2D::Texture2D(int width, int height, TexFormat format)
    :   m_Width(width),
        m_Height(height)
    {
        gl::glGenTextures(1, &m_TexId);
        Bind(0);

        gl::GLenum gl_internal_format;

        switch (format) {
            case TexFormat::R_8_I:      gl_internal_format = gl::GL_R8; break;
            case TexFormat::RG_8_I:     gl_internal_format = gl::GL_RG8; break;
            case TexFormat::RGB_8_I:    gl_internal_format = gl::GL_RGB8; break;
            case TexFormat::RGBA_8_I:   gl_internal_format = gl::GL_RGBA8; break;

            case TexFormat::R_8_U:      gl_internal_format = gl::GL_R8UI; break;
            case TexFormat::RG_8_U:     gl_internal_format = gl::GL_RG8UI; break;
            case TexFormat::RGB_8_U:    gl_internal_format = gl::GL_RGB8UI; break;
            case TexFormat::RGBA_8_U:   gl_internal_format = gl::GL_RGBA8UI; break;
        }

        gl::glTexStorage2D(gl::GL_TEXTURE_2D, 1, gl_internal_format, width, height);

        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);


        switch (format) {
            case TexFormat::R_8_I:    { m_FormatComponents = gl::GL_RED; m_FormatTypes = gl::GL_BYTE; break; }
            case TexFormat::RG_8_I:   { m_FormatComponents = gl::GL_RG; m_FormatTypes = gl::GL_BYTE; break; }
            case TexFormat::RGB_8_I:  { m_FormatComponents = gl::GL_RGB; m_FormatTypes = gl::GL_BYTE; break; }
            case TexFormat::RGBA_8_I: { m_FormatComponents = gl::GL_RGBA; m_FormatTypes = gl::GL_BYTE; break; }

            case TexFormat::R_8_U:    { m_FormatComponents = gl::GL_RED; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RG_8_U:   { m_FormatComponents = gl::GL_RG; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RGB_8_U:  { m_FormatComponents = gl::GL_RGB; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RGBA_8_U: { m_FormatComponents = gl::GL_RGBA; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
        }
    }

    Texture2D::~Texture2D()
    {
        gl::glDeleteTextures(1, &m_TexId);
    }

    void Texture2D::Bind(int slot) const
    {
        gl::glActiveTexture(gl::GL_TEXTURE0 + slot);
        gl::glBindTexture(gl::GL_TEXTURE_2D, m_TexId);
    }

    void Texture2D::SetData(int x, int y, int width, int height, const void *data)
    {
        Bind(0);
        gl::glTexSubImage2D(
            gl::GL_TEXTURE_2D, 
            0, 
            x, y, 
            width, height, 
            m_FormatComponents, m_FormatTypes, 
            data
        );
    }

    // std::shared_ptr<Texture2D> Texture2D::CreateFromFile(const std::string& filepath)
    // {

    // }

}