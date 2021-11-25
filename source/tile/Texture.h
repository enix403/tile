#pragma once

#include <memory>

namespace Tile
{
    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual unsigned int GetID() const = 0;
        virtual void Bind(int slot) const = 0;
    };

    enum class TexFormat 
    {   
        // Unsigned
        R_8_U, RG_8_U, RGB_8_U, RGBA_8_U,

        // Signed
        R_8_I, RG_8_I, RGB_8_I, RGBA_8_I
    };

    class Texture2D: public Texture
    {
    public:
        Texture2D(int width, int height, TexFormat format);
        ~Texture2D();

        unsigned int GetID() const override { return m_TexId; }
        void Bind(int slot) const override;

        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }

        void SetData(int x, int y, int width, int height, const void* data);

        // static std::shared_ptr<Texture2D> CreateFromFile(const std::string& filepath);

    private:
        unsigned int m_TexId;
        int m_Width, m_Height;

        // used for (weird) opengl functions
        unsigned int m_FormatComponents, m_FormatTypes;    
    };
}