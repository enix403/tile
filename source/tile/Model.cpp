#include "tile/Model.h"

#include <iostream>
#include <unordered_map>

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
    /* ====================== Space Sutff ====================== */
    /* ========================================================= */

    SpaceConverter::SpaceConverter(const SpaceCoordinateSystem& source, const SpaceCoordinateSystem& target)
    {
        m_MoveX = GenAxisMove(source.RightDirection, target.RightDirection);
        m_MoveY = GenAxisMove(source.UpDirection, target.UpDirection);
        m_MoveZ = GenAxisMove(source.ForwardDirection, target.ForwardDirection);
    }

    SpaceConverter::CompMove SpaceConverter::GenAxisMove(   SpaceCoordinateSystem::Axis first, 
                                                            SpaceCoordinateSystem::Axis second)
    {
        return { second.line - first.line, first.sign * second.sign };
    }

    void SpaceConverter::ConvertInPlace(glm::vec3& vec)
    {
        glm::vec3 vecCopy = vec;
        vec[(0 + m_MoveX.deltaDest) % 3] = vecCopy[0] * m_MoveX.multiplier;
        vec[(1 + m_MoveY.deltaDest) % 3] = vecCopy[1] * m_MoveY.multiplier;
        vec[(2 + m_MoveZ.deltaDest) % 3] = vecCopy[2] * m_MoveZ.multiplier;
    }

    /* ========================================================== */
    /* ====================== ModelBuilder ====================== */
    /* ========================================================== */

    ModelBuilder::ModelBuilder()
    // Any non-trivial model will have atleast quite a few vertices
    // so it is good to initialize the vector to with preallocated space for
    // some vertices
    :   m_Vertices(32),
        m_Indices(3 * 48)
    {}

    std::shared_ptr<Model> ModelBuilder::LoadObjFromFile(const std::string &filepath, const std::string& shapeName)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> mats;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, filepath.c_str()))
        {
            std::cerr << "[ERROR] Failed to load model: \"" << filepath << "\". "
                    << err << warn << std::endl;
            return nullptr;
        }

        m_Vertices.clear();
        m_Indices.clear();

        // A map from a given (unique) vertex to its index in `m_Vertices`
        std::unordered_map<Vertex, uint32_t> unique_vertices;

        // float y_multiplier = -1.0f;

        SpaceCoordinateSystem source = {
            { SpaceCoordinateSystem::AXIS_LINE_X,  1 },
            { SpaceCoordinateSystem::AXIS_LINE_Y,  1 },
            { SpaceCoordinateSystem::AXIS_LINE_Z,  1 },
        };

        SpaceCoordinateSystem target = {
            { SpaceCoordinateSystem::AXIS_LINE_X,  1 },
            { SpaceCoordinateSystem::AXIS_LINE_Y,  1 },
            { SpaceCoordinateSystem::AXIS_LINE_Z,  1 },
        };

        SpaceConverter converter(source, target);

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