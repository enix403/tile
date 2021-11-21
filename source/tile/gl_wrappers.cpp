#include "tile/gl_wrappers.h"
#include "tile/opengl_inc.h"

#include <iostream>

using namespace gl;

namespace
{
    using namespace Tile;
    int comp_type_byte_count(VertAttribComponentType type)
    {
        switch(type)
        {
        case VertAttribComponentType::Float:
            return sizeof(float);
        case VertAttribComponentType::Int:
            return sizeof(int);

        default:
            return 0;
        }
    }

    int comp_to_gl_type(VertAttribComponentType type)
    {
        switch(type)
        {
        case VertAttribComponentType::Float:
            return GL_FLOAT;
        case VertAttribComponentType::Int:
            return GL_INT;

        default:
            return -1;
        }
    }
}

namespace Tile
{

    /* ============================================================== */
    /* ============================================================== */
    /* ======================= VERTEX BUFFERS ======================= */
    /* ============================================================== */
    /* ============================================================== */


    VertexBuffer::VertexBuffer()
    {
        glGenBuffers(1, &m_BufId);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_BufId);
        m_BufId = 0;
    }

    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_BufId);
    }
    void VertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::SetData(const void* data, int size)
    {
        SetData(data, size, GL_STATIC_DRAW);
    }

    void VertexBuffer::SetData(const void* data, int size, int usage)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_BufId);
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }


    /* ============================================================= */
    /* ============================================================= */
    /* ======================= INDEX BUFFERS ======================= */
    /* ============================================================= */
    /* ============================================================= */

    IndexBuffer::IndexBuffer()
    {
        glGenBuffers(1, &m_BufId);
    }

    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(1, &m_BufId);
        m_BufId = 0;
    }

    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufId);
    }
    void IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void IndexBuffer::SetIndices(const uint* data, int size)
    {
        SetIndices(data, size, GL_STATIC_DRAW);
    }

    void IndexBuffer::SetIndices(const uint* data, int size, int usage)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }

    /* ============================================================= */
    /* ============================================================= */
    /* ======================= VERTEX ARRAYS ======================= */
    /* ============================================================= */
    /* ============================================================= */

    VertexArray::VertexArray()
    {
        glGenVertexArrays(1, &m_VaoId);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &m_VaoId);
        m_VaoId = 0;
    }

    void VertexArray::Bind() const
    {
        glBindVertexArray(m_VaoId);
    }
    void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void VertexArray::AddVertexBuffer(const VertexBuffer& buffer, const VertexLayout& layout)
    {
        glBindVertexArray(m_VaoId);
        buffer.Bind();

        int stride = 0;

        for(auto& elem : layout)
        {
            stride += elem.VecComponentCount * comp_type_byte_count(elem.ComponentType);
        }

        int currentOffset = 0;

        for(VLayoutElement elem : layout)
        {
            glEnableVertexAttribArray(elem.LayoutIndex);
            glVertexAttribPointer(elem.LayoutIndex,
                                  elem.VecComponentCount,
                                  comp_to_gl_type(elem.ComponentType),
                                  elem.Normalize ? GL_TRUE : GL_FALSE,
                                  stride,

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"
                                  (void*)currentOffset
#pragma clang diagnostic pop

            );
            currentOffset += elem.VecComponentCount * comp_type_byte_count(elem.ComponentType);
        }
    }

    void VertexArray::AddIndexBuffer(const IndexBuffer& buffer)
    {
        glBindVertexArray(m_VaoId);
        buffer.Bind();
    }
}