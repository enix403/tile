#pragma once

#include "tile/gl_wrappers.h"

#include <cstdint>
#include <vector>
#include <memory>

#include <glm/vec3.hpp>

namespace Tile {

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;

        bool operator==(const Vertex& other) const
        {
            return position == other.position && normal == other.normal;
        }
    };

    class Model
    {
    public:

        Model();

        ~Model() = default;

        inline const VertexArray& GetVA()       const { return m_VA;   }
        inline const VertexBuffer& GetVBuf()    const { return m_VBuf; }
        inline const IndexBuffer& GetIBuf()     const { return m_IBuf; }

        inline int GetVertexCount()     const { return m_VertexCount;    }
        inline int GetIndexCount()      const { return m_IndexCount;     }
        inline bool HasIndexBuffer()    const { return m_HasIndexBuffer; }

        void CreateVertexBuffer(const std::vector<Vertex>& vertices);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices);

    private:
        VertexArray m_VA;

        VertexBuffer m_VBuf;
        int m_VertexCount = 0;

        bool m_HasIndexBuffer = false;
        IndexBuffer m_IBuf;
        int m_IndexCount = 0;
    };

    struct SpaceCoordinateSystem
    {   
        // enum classes do not support conversion to integers
        enum AxisLine { AXIS_LINE_X = 0, AXIS_LINE_Y = 1, AXIS_LINE_Z = 2 };

        struct Axis 
        {
            AxisLine line;
            int sign;
        };

        Axis RightDirection;
        Axis UpDirection;
        Axis ForwardDirection;

    public:
        SpaceCoordinateSystem(const SpaceCoordinateSystem& other) = default;
        SpaceCoordinateSystem& operator=(const SpaceCoordinateSystem& other) = default;
    };

    class SpaceConverter
    {
    public:
        SpaceConverter(const SpaceCoordinateSystem& source, const SpaceCoordinateSystem& target);

        void ConvertInPlace(glm::vec3& vec);

    private:
        struct CompMove
        {
            int deltaDest = 0;
            int multiplier = 1;
        };

        CompMove GenAxisMove(SpaceCoordinateSystem::Axis first, SpaceCoordinateSystem::Axis second);

    private:
        CompMove m_MoveX;
        CompMove m_MoveY;
        CompMove m_MoveZ;
    };

    class ModelBuilder
    {
    public:
        ModelBuilder();

        inline std::shared_ptr<Model> LoadObjFromFile(const std::string& filepath)
        {
            return LoadObjFromFile(filepath, "");
        }

        std::shared_ptr<Model> LoadObjFromFile(const std::string& filepath, const std::string& shapeName);

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;    
    };
}