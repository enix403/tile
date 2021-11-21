#pragma once

#include <initializer_list>
#include <string>
#include <vector>

namespace Tile
{

    /* ==================================================================== */
    /* ==================================================================== */
    /* ======================= VERTEX BUFFER LAYOUT ======================= */
    /* ==================================================================== */
    /* ==================================================================== */

    enum class VertAttribComponentType // "vec"3, "ivec"2, etc
    {
        Float,
        Int
    };

    struct VLayoutElement
    {
        int LayoutIndex;
        const std::string Name;
        int VecComponentCount; // number of components (i.e 4 for vec4) or 1 in case of float, int, etc
        VertAttribComponentType ComponentType;
        bool Normalize;
    };
    using VertexLayout = std::vector<VLayoutElement>;


    /* ============================================================== */
    /* ============================================================== */
    /* ======================= VERTEX BUFFERS ======================= */
    /* ============================================================== */
    /* ============================================================== */

    class VertexBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer();

        void Bind() const;
        void Unbind() const;

        void SetData(const void* data, int size);
        void SetData(const void* data, int size, int usage);

    private:
        uint m_BufId;
    };

    /* ============================================================= */
    /* ============================================================= */
    /* ======================= INDEX BUFFERS ======================= */
    /* ============================================================= */
    /* ============================================================= */

    class IndexBuffer
    {
    public:
        IndexBuffer();
        ~IndexBuffer();

        void Bind() const;
        void Unbind() const;

        void SetIndices(const uint* indices, int size);
        void SetIndices(const uint* indices, int size, int usage);

    private:
        uint m_BufId;
    };

    /* ============================================================= */
    /* ============================================================= */
    /* ======================= VERTEX ARRAYS ======================= */
    /* ============================================================= */
    /* ============================================================= */

    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();

        void Bind() const;
        void Unbind() const;

        void AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout);
        void AddIndexBuffer(const IndexBuffer& buffer);

    private:
        uint m_VaoId;
    };

}