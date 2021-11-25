#include "tile/Model.h"

#include <iostream>
#include <TinyObjLoader/tiny_obj_loader.h>

#include <glm/matrix.hpp>
#include <glm/mat3x3.hpp>

namespace
{
    using namespace Tile;

    enum class SpaceDirection: uint8_t
    { RIGHT, UP, FORWARD };

    struct AxisDescription
    {
        Axis AxisObj;
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

    inline Axis AxisFromDirection(const CoordinateSystem3D& system, SpaceDirection direction)
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
        auto sourceAxis = AxisFromDirection(source, targetAxisDesc.Direction);

        return {
            static_cast<std::underlying_type_t<AxisLine>>(sourceAxis.Line),
            static_cast<int8_t>(targetAxisDesc.AxisObj.Sign * sourceAxis.Sign)
        };
    }

    // Returns the sign of the given number
    // ( -1 when x < 0 , 0 when x == 0 and +1 when x > 0 )
    int sign(float val) 
    {
        return (0 < val) - (val < 0);
    }


    // Checks if two coordinate systems have the same handedness (both are left handed or both are right handed)
    bool IsSameHandedness(const SpaceConverter& spaceConverter)
    {
        // To check the handedness we first convert the standard basis from the first coordinate
        // space to the second. Then use these (converted) vectors to create an orthonormal matrix representing the
        // linear transformation for the conversion. Transforming a point/vector between two coordinate systems of
        // same handedness requires ONLY a rotation and transforming between systems of different handedness requires
        // a rotation and a reflection. Also the determinant of a pure rotation matix with no reflection is +1. So we can
        // check the determinant to be equal to +1 implying tranformation requires only rotation which would further imply
        // that the two systems are of the same handedness.
        //
        //
        // Phew...
        //
        
        glm::vec3 basisX = { 1.f, 0.f, 0.f };
        glm::vec3 basisY = { 0.f, 1.f, 0.f };
        glm::vec3 basisZ = { 0.f, 0.f, 1.f };

        spaceConverter.ConvertInPlace(basisX);
        spaceConverter.ConvertInPlace(basisY);
        spaceConverter.ConvertInPlace(basisZ);

        // Just in case. Deteminant should always be +1, but you can never know when floating point arithematic
        // causes the value to be slightly different.
        int detSign = sign(glm::determinant(glm::mat3(basisX, basisY, basisZ)));
        return detSign == 1;
    }

    inline bool IsSameHandedness(const CoordinateSystem3D& first, const CoordinateSystem3D& second)
    {
        return IsSameHandedness(SpaceConverter(first, second));
    }
}


namespace Tile {

    Model::Model() {}

    void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
    {
        m_IndexCount = indices.size();
        m_HasIndexBuffer = true;

        m_VA.AddIndexBuffer(m_IBuf);
        m_IBuf.SetIndices(indices.data(), sizeof(uint32_t) * m_IndexCount);
    }

    void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
    {
        m_VertexCount = vertices.size();
        
        m_VA.AddVertexBuffer(m_VBuf, {
            {0, "ia_Pos",       3, VertAttribComponentType::Float, false},
            {1, "ia_Normal",    3, VertAttribComponentType::Float, false},
            {2, "ia_TexCoords", 2, VertAttribComponentType::Float, false},
        });

        m_VBuf.SetData(vertices.data(), sizeof(Vertex) * m_VertexCount);
    }

    /* ============================================================================================================ */
    /* ============================================================================================================ */
    /* ================================================ Space Stuff =============================================== */
    /* ============================================================================================================ */
    /* ============================================================================================================ */

    SpaceConverter::SpaceConverter(const CoordinateSystem3D& source, const CoordinateSystem3D& target)
    {
        m_MoveX = CreateComponentMove(source, target, AxisLine::LINE_X);
        m_MoveY = CreateComponentMove(source, target, AxisLine::LINE_Y);
        m_MoveZ = CreateComponentMove(source, target, AxisLine::LINE_Z);
    }

    void SpaceConverter::ConvertInPlace(glm::vec3& vec) const
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

    std::shared_ptr<Model> ModelBuilder::LoadWavefrontObj(const std::string& filepath, const std::string& shapeName)
    {
        attrib = std::make_unique<tinyobj::attrib_t>();
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> mats;
        std::string warn, err;

        m_Vertices.clear();
        m_Indices.clear();
        m_UniqueVertices.clear();

        if (!tinyobj::LoadObj(attrib.get(), &shapes, &mats, &warn, &err, filepath.c_str()))
        {
            std::cerr << "[ERROR] Failed to load model: \"" << filepath << "\". "
                    << err << warn << std::endl;


            // Return an empty model so that things do not break due to null pointers
            auto model = std::make_shared<Model>();
            model->CreateVertexBuffer(m_Vertices); // m_Vertices is already empty at this point
            return model;
        }

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

        converter = std::make_unique<SpaceConverter>(source, target);
        bool toggleWindingOrder = !IsSameHandedness(*converter);

        for (const auto& shape: shapes)
        {
            // Load only the given shape, if specified. If no shape is specified
            // then load all the shapes
            if (shapeName != "" && shape.name != shapeName)
                continue;

            const auto& mesh = shape.mesh;
            
            int face_index = 0;
            int mesh_indicies_index = 0; // Index into shape.mesh.indices

            while (mesh_indicies_index < mesh.indices.size())
            {
                auto face_vertex_count = mesh.num_face_vertices[face_index];

                // Face triangulation
                // 
                // This code currently only successfully triangulates Simple Convex Polygons
                // Concave and/or Complex polygons will give incorrect visual results
                for (int i = 0; i < face_vertex_count - 2; i ++)
                {  
                    // form a triangle with vertices (0, i + 1, i + 2)
                    const auto& face_index_elem_a = mesh.indices[mesh_indicies_index + 0 + 0];
                    const auto& face_index_elem_b = mesh.indices[mesh_indicies_index + i + 1];
                    const auto& face_index_elem_c = mesh.indices[mesh_indicies_index + i + 2];

                    // flip the first and third vertices for back-face culling
                    // if model has been reflected during the coordinate system conversion 
                    if (toggleWindingOrder) 
                    {
                        AddVertex(face_index_elem_c);
                        AddVertex(face_index_elem_b);
                        AddVertex(face_index_elem_a);
                    }
                    else
                    {
                        AddVertex(face_index_elem_a);
                        AddVertex(face_index_elem_b);
                        AddVertex(face_index_elem_c);
                    }
                }

                mesh_indicies_index += face_vertex_count;
                face_index++;
            }
        }

        auto model = std::make_shared<Model>();
        model->CreateVertexBuffer(m_Vertices);
        model->CreateIndexBuffer(m_Indices);
        return model;
    }

    void ModelBuilder::AddVertex(const tinyobj::index_t& index_elem)
    {
        Vertex vertex;

        vertex.position = {
            attrib->vertices[3 * index_elem.vertex_index + 0],
            attrib->vertices[3 * index_elem.vertex_index + 1],
            attrib->vertices[3 * index_elem.vertex_index + 2],
        };

        converter->ConvertInPlace(vertex.position);
        
        // Normals are optional in obj files
        if (index_elem.normal_index >= 0)
        {
            vertex.normal = {
                attrib->normals[3 * index_elem.normal_index + 0],
                attrib->normals[3 * index_elem.normal_index + 1],
                attrib->normals[3 * index_elem.normal_index + 2],
            };

            converter->ConvertInPlace(vertex.normal);
        }
        else {
            // Maybe change the default...
            vertex.normal = { 0.f, 0.f , 0.f};
        }

        if (index_elem.texcoord_index >= 0)
        {
            vertex.textureCoords = {
                attrib->texcoords[2 * index_elem.texcoord_index + 0],
                attrib->texcoords[2 * index_elem.texcoord_index + 1],
            };
        }
        else
        {
            vertex.textureCoords = { 0.f, 0.f };
        }

        if (m_UniqueVertices.count(vertex) == 0)
        {
            m_UniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
            m_Vertices.push_back(vertex);
        }

        m_Indices.push_back(m_UniqueVertices[vertex]);
    }
}