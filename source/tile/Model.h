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
    // Note that coordinate systems only differ by what axis they use to name the physical directions (RIGHT, UP, and FORWARD)
    // Example: OpenGL canonical view volume is defined in a 3d coordinate system where the RIGHT direction is +X axis, UP direction
    // is +Y axis and FORWARD is the -Z axis.
    //
    // Also since the conversion involes only rotation and reflection, normals (and any vector) can also be 
    // safely converted here.
    class SpaceConverter
    {
    public:
        SpaceConverter(const CoordinateSystem3D& source, const CoordinateSystem3D& target);
        void ConvertInPlace(glm::vec3& vec);

        struct CompMove 
        {
            uint8_t SourceCompLocation;
            int8_t Multiplier; // -1 or +1
        };

        inline int FlipParity() const { return m_FlipParity; }

    private:
        CompMove m_MoveX;
        CompMove m_MoveY;
        CompMove m_MoveZ;

        int m_FlipParity;
    };

    /* ========================================================================================================= */
    /* ============================================== SPACE STUFF ============================================== */
    /* ========================================================================================================= */

    class ModelBuilder
    {
    public:
        ModelBuilder();

        inline std::shared_ptr<Model> LoadObjFromFile(const std::string& filepath)
        {
            return LoadObjFromFile(filepath, "");
        }

        std::shared_ptr<Model> LoadObjFromFile(const std::string& filepath, const std::string& shapeName);
        
        inline bool ShouldToggleCullWindingOrder() const { return m_ToggleWindingOrder; }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;    

        bool m_ToggleWindingOrder;
    };
}