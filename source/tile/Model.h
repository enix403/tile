#pragma once

#include "TinyObjLoader/tiny_obj_loader.h"
#include "tile/gl_wrappers.h"

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// forward declaration
namespace tinyobj 
{
    struct attrib_t;
}

namespace Tile {
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoords;

        bool operator==(const Vertex& other) const
        {
            return  position == other.position && 
                    normal == other.normal &&
                    textureCoords == other.textureCoords;
        }
    };
}

namespace 
{
    // @See https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void combine_hash(std::size_t& seed, const T& v, const Rest&... rest) 
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (combine_hash(seed, rest), ...);
    };
}

namespace std {
    using namespace Tile;
    template<> struct hash<Vertex> {
        size_t operator()(const Vertex& vertex) const 
        {
            size_t seed = 0;
            combine_hash(seed, vertex.position, vertex.normal, vertex.textureCoords);
            return seed;
        }
    };
}

namespace Tile
{

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

    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ====================== SPACE STUFF ====================== */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */

    enum class AxisLine : uint8_t { LINE_X = 0, LINE_Y = 1, LINE_Z = 2 };
    struct Axis
    {
        AxisLine Line;
        int8_t Sign; // -1 or +1
    };

    struct CoordinateSystem3D
    {
        // This struct only hold the "name"s given to the physical directions right, up and forward
        // (e.g the standard OpenGL Clip space has right direction being +X, up is +Y and foward is -Z)
        //
        // The directions below are relative to an imaginary camera placed at the origin
        // and looking towards the forward direction
        //
        // The description of the physical directions are always fixed relative to the camera. The 'RIGHT'
        // direction will always be the physical right (horizontally going right from the camera origin) regardless of what it
        // is named (+X, -X, -Z or anything). 'UP' is always, well, up and 'FORWARD' is the direction the camera is looking at,
        // i.e into the computer screen/camera viewport

        Axis RightDirection;
        Axis UpDirection;
        Axis ForwardDirection;

    public:
        CoordinateSystem3D(const CoordinateSystem3D& other) = default;
        CoordinateSystem3D& operator=(const CoordinateSystem3D& other) = default;
    };

    // This class converts a vec3 represented in one CoordinateSystem3D (source) to the SAME vec3, but represented
    // using a new CoordinateSystem3D (target).
    // Note that coordinate systems here only differ by what axis they use to name the physical directions (RIGHT, UP, and FORWARD)
    //
    // Also since the conversion involes only rotation and reflection, normals (and any vector) can also be 
    // safely converted here.
    class SpaceConverter
    {
    public:
        SpaceConverter(const CoordinateSystem3D& source, const CoordinateSystem3D& target);

        SpaceConverter(const SpaceConverter& other) = default;
        SpaceConverter& operator=(const SpaceConverter& other) = default;

        void ConvertInPlace(glm::vec3& vec) const;

        struct CompMove 
        {
            uint8_t SourceCompLocation;
            int8_t Multiplier; // -1 or +1
        };

    private:
        CompMove m_MoveX;
        CompMove m_MoveY;
        CompMove m_MoveZ;
    };

    /* ========================================================================================================= */
    /* ============================================== SPACE STUFF ============================================== */
    /* ========================================================================================================= */
    class ModelBuilder
    {
    public:
        ModelBuilder();

        inline std::shared_ptr<Model> LoadWavefrontObj(const std::string& filepath)
        {
            return LoadWavefrontObj(filepath, "");
        }

        std::shared_ptr<Model> LoadWavefrontObj(const std::string& filepath, const std::string& shapeName);

    private:
        void AddVertex(int vertex_index, int normal_index, int texcoord_index);

    private:

        std::unique_ptr<SpaceConverter> converter;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;

        // A map from a given (unique) vertex to its index in `m_Vertices`
        std::unordered_map<Vertex, uint32_t> m_UniqueVertices; 

        std::unique_ptr<tinyobj::attrib_t> attrib;
    };
}