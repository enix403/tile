
#include "tile/Model.h"

#include <TinyObjLoader/tiny_obj_loader.h>
#include <vector>
#include <iostream>

using namespace Tile;

void load_model_test_main()
{
    std::string filepath = "assets/models/cube.obj";

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> mats;
    std::string warn, err;

    bool res = tinyobj::LoadObj(&attrib, &shapes, &mats, &warn, &err, filepath.c_str());
    if (!res)
    {
        std::cerr << "Failed to load model: \"" << filepath << "\"" << std::endl;
        std::cerr << "    " << err << warn;

        return;
    }

    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;

    for (const auto& shape : shapes)
    {
        std::cout << "Shape boundry [" << shape.name << "]" << std::endl;

        const auto& mesh = shape.mesh;

        std::cout << "[ " << attrib.normals.size() << " ]" << "    ";
        for (const auto entt: attrib.vertices)
        {   
            std::cout << entt << ", ";
        }

        // std::cout << "    ";
        // for (const auto& index: shape.mesh.indices)
        // {
        //     std::cout << ", " << index.normal_index + 1;
        // }

        // std::cout << std::endl;
    }
    
}