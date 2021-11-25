#include "tile/Texture.h"
#include "tile/opengl_inc.h"

#include <iostream>

#include <STB/stb_image.h>


namespace Tile {
    Texture2D::Texture2D(int width, int height, TexFormat format, int levelCount)
    :   m_Width(width),
        m_Height(height)
    {
        gl::glGenTextures(1, &m_TexId);
        Bind(0);

        gl::GLenum gl_internal_format;

        switch (format) {
            case TexFormat::R_8:      gl_internal_format = gl::GL_R8; break;
            case TexFormat::RG_8:     gl_internal_format = gl::GL_RG8; break;
            case TexFormat::RGB_8:    gl_internal_format = gl::GL_RGB8; break;
            case TexFormat::RGBA_8:   gl_internal_format = gl::GL_RGBA8; break;
        }

        // gl_internal_format = gl::GL_RGB8;

        gl::glTexStorage2D(gl::GL_TEXTURE_2D, levelCount, gl_internal_format, width, height);

        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
        gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);


        switch (format) {
            case TexFormat::R_8:    { m_FormatComponents = gl::GL_RED; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RG_8:   { m_FormatComponents = gl::GL_RG; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RGB_8:  { m_FormatComponents = gl::GL_RGB; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
            case TexFormat::RGBA_8: { m_FormatComponents = gl::GL_RGBA; m_FormatTypes = gl::GL_UNSIGNED_BYTE; break; }
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

    void Texture2D::Unbind() const
    {
        gl::glBindTexture(gl::GL_TEXTURE_2D, 0);
    }

    void Texture2D::SetData(int level, int x, int y, int width, int height, const void *data)
    {
        Bind(0);
        gl::glTexSubImage2D(
            gl::GL_TEXTURE_2D, 
            level, 
            x, y, 
            width, height, 
            m_FormatComponents, m_FormatTypes, 
            data
        );
    }

    std::shared_ptr<Texture2D> Texture2D::ImageFromFile(const std::string& filepath)
    {
        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;

        stbi_uc* image_data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

        if (image_data == nullptr)
        {
            std::cerr << "[ERROR] Failed to load texture \"" << filepath << "\"" << std::endl;
            return std::make_shared<Texture2D>(1, 1, TexFormat::R_8);
        }

        TexFormat format;
        switch (channels) {
            case 1: format = TexFormat::R_8;      break;
            case 2: format = TexFormat::RG_8;     break;
            case 3: format = TexFormat::RGB_8;    break;
            case 4: format = TexFormat::RGBA_8;   break;

            default:
            {
                std::cerr << "[ERROR] Unsuppported channel count (= " << channels << ") in texture \"" 
                        << filepath << "\"" 
                        << std::endl;
                return std::make_shared<Texture2D>(1, 1, TexFormat::R_8);
            }
        }

        auto texture = std::make_shared<Texture2D>(width, height, format);
        texture->SetData(0, 0, 0, width, height, image_data);

        stbi_image_free(image_data);
        return texture;
    }

}