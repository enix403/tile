#include "tile/Model.h"

#include <iostream>
#include <unordered_map>
#include <cstdint>

#include <TinyObjLoader/tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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
            combine_hash(seed, vertex.position, vertex.normal);
            return seed;
        }
    };
}


namespace Tile {

    Model::Model() {}

    void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
    {
        m_IndexCount = indices.size();
        m_HasIndexBuffer = true;

        m_VA.AddIndexBuffer(m_IBuf);
        m_IBuf.SetIndices(indices.data(), sizeof(indices[0]) * m_IndexCount);
    }

    void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = vertices.size();
        
        m_VA.AddVertexBuffer(m_VBuf, {
            {0, "ia_Pos",    3, VertAttribComponentType::Float, false},
            {1, "ia_Normal", 3, VertAttribComponentType::Float, false},
        });

        m_VBuf.SetData(vertices.data(), sizeof(vertices[0]) * m_VertexCount);
    }

    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ====================== Space Stuff ====================== */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */
    /* ========================================================= */

    enum class SpaceDirection: uint8_t
    { RIGHT, UP, FORWARD };

    struct AxisDescription
    {
        Axis Axis;
        SpaceDirection Direction;
    };

    inline AxisDescription SystemDirectionFromLine(const CoordinateSystem3D& system, AxisLine line)
    {
        if (system.RightDirection.Line == line)
            return { system.RightDirection, SpaceDirection::RIGHT };

        if (system.ForwardDirection.Line == line)
            return { system.ForwardDirection, SpaceDirection::FORWARD } ;

        return { system.UpDirection, SpaceDirection::UP };
    }

    inline const Axis AxisFromDirection(const CoordinateSystem3D& system, SpaceDirection direction)
    {
        if (direction == SpaceDirection::RIGHT)
            return system.RightDirection;
        
        if (direction == SpaceDirection::FORWARD)
            return system.ForwardDirection;

        return system.UpDirection;
    }

    inline SpaceConverter::CompMove CreateComponentMove(const CoordinateSystem3D& source,
                                                        const CoordinateSystem3D& target,
                                                        AxisLine line)
    {
        auto targetAxisDesc = SystemDirectionFromLine(target, line);
        const auto sourceAxis = AxisFromDirection(source, targetAxisDesc.Direction);

        return {
            static_cast<std::underlying_type_t<AxisLine>>(sourceAxis.Line),
            static_cast<int8_t>(targetAxisDesc.Axis.Sign * sourceAxis.Sign)
        };
    }


    SpaceConverter::SpaceConverter(const CoordinateSystem3D& source, const CoordinateSystem3D& target)
    {
        m_MoveX = CreateComponentMove(source, target, AxisLine::LINE_X);
        m_MoveY = CreateComponentMove(source, target, AxisLine::LINE_Y);
        m_MoveZ = CreateComponentMove(source, target, AxisLine::LINE_Z);

        // FIXME: This calculation, at the moment, is incorrect. Will be fixed later.
        m_FlipParity =    source.RightDirection.Sign
                        * source.UpDirection.Sign
                        * source.ForwardDirection.Sign
                        * target.RightDirection.Sign
                        * target.UpDirection.Sign
                        * target.ForwardDirection.Sign;
    }

    void SpaceConverter::ConvertInPlace(glm::vec3& vec)
    {
        glm::vec3 vecCopy = vec;
        vec[0] = vecCopy[m_MoveX.SourceCompLocation] * m_MoveX.Multiplier;
        vec[1] = vecCopy[m_MoveY.SourceCompLocation] * m_MoveY.Multiplier;
        vec[2] = vecCopy[m_MoveZ.SourceCompLocation] * m_MoveZ.Multiplier;
    }

    /* ============================================================================================================== */
    /* ================================================ ModelBuilder ================================================ */
    /* ============================================================================================================== */

    ModelBuilder::ModelBuilder()
    // Any non-trivial model will have atleast quite a few vertices
    // so it is good to initialize the vector to with preallocated space for
    // some vertices
    :   m_Vertices(32),
        m_Indices(3 * 48)
    {}

    std::shared_ptr<Model> ModelBuilder::LoadObjFromFile(const std::string& filepath, const std::string& shapeName)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> mats;
        std::string warn, err;

        m_Vertices.clear();
        m_Indices.clear();

        if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, filepath.c_str()))
        {
            std::cerr << "[ERROR] Failed to load model: \"" << filepath << "\". "
                    << err << warn << std::endl;


            // Return an empty model so that things do not break due to null pointers
            auto model = std::make_shared<Model>();
            model->CreateVertexBuffer(m_Vertices); // m_Vertices is already empty at this point
            return model;
        }

        // A map from a given (unique) vertex to its index in `m_Vertices`
        std::unordered_map<Vertex, uint32_t> unique_vertices;

        // This should be taken as input instead of being hardcoded here... 
        CoordinateSystem3D source = {
            { AxisLine::LINE_X, +1 },
            { AxisLine::LINE_Y, -1 },
            { AxisLine::LINE_Z, -1 },
        };

        CoordinateSystem3D target = {
            { AxisLine::LINE_X, +1 },
            { AxisLine::LINE_Y, +1 },
            { AxisLine::LINE_Z, -1 },
        };

        SpaceConverter converter(source, target);

        m_ToggleWindingOrder = converter.FlipParity() == -1;

        for (const auto& shape: shapes)
        {

            // Load only the given shape, if specified. If no shape is specified
            // then load all the shapes
            if (shapeName != "" && shape.name != shapeName)
                continue;

            for (const auto& indices: shape.mesh.indices)
            {
                Vertex vertex {};

                if (indices.vertex_index >= 0)
                {
                    vertex.position = {
                        attrib.vertices[3 * indices.vertex_index + 0],
                        attrib.vertices[3 * indices.vertex_index + 1],
                        attrib.vertices[3 * indices.vertex_index + 2],
                    };

                    converter.ConvertInPlace(vertex.position);
                }
                else {
                    break;
                }

                if (indices.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * indices.normal_index + 0],
                        attrib.normals[3 * indices.normal_index + 1],
                        attrib.normals[3 * indices.normal_index + 2],
                    };

                    converter.ConvertInPlace(vertex.normal);

                }
                else {
                    break;
                }

                if (unique_vertices.count(vertex) == 0)
                {
                    unique_vertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }

                m_Indices.push_back(unique_vertices[vertex]);
            }
        }

        auto model = std::make_shared<Model>();
        model->CreateVertexBuffer(m_Vertices);
        model->CreateIndexBuffer(m_Indices);
        return model;
    }
}