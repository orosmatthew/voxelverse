#include "util.hpp"

#include <optional>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace mve {

ModelData load_model(const std::filesystem::path& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    std::string path_str = path.string();
    if (bool result = LoadObj(&attrib, &shapes, &materials, &warning, &error, path_str.c_str()); !result) {
        throw std::runtime_error("[TinyObj] " + warning + " " + error);
    }

    VertexLayout vertex_layout {};
    vertex_layout.push_back(VertexAttributeType::vec3); // 3D position
    vertex_layout.push_back(VertexAttributeType::vec3); // Color
    vertex_layout.push_back(VertexAttributeType::vec2); // Texture coord

    struct Vertex {
        nnm::Vector3f position;
        nnm::Vector3f color;
        nnm::Vector2f texture_coord;

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && texture_coord == other.texture_coord;
        }
    };

    std::vector<Vertex> vertices {};
    std::vector<uint32_t> indices {};

    // ReSharper disable once CppUseStructuredBinding
    for (const auto& shape : shapes) {
        for (const auto& [vertex_index, normal_index, texcoord_index] : shape.mesh.indices) {
            Vertex vertex {};
            vertex.position = { attrib.vertices[3 * vertex_index + 0],
                                attrib.vertices[3 * vertex_index + 1],
                                attrib.vertices[3 * vertex_index + 2] };
            vertex.color = { 1.0f, 1.0f, 1.0f };
            vertex.texture_coord
                = { attrib.texcoords[2 * texcoord_index + 0], 1.0f - attrib.texcoords[2 * texcoord_index + 1] };

            std::optional<size_t> duplicate_index;
            for (size_t i = 0; i < vertices.size(); i++) {
                if (vertex == vertices.at(i)) {
                    duplicate_index = i;
                    break;
                }
            }

            if (duplicate_index.has_value()) {
                indices.push_back(duplicate_index.value());
            }
            else {
                indices.push_back(vertices.size());
                vertices.push_back(vertex);
            }
        }
    }

    VertexData data(vertex_layout);
    for (const auto& [position, color, texture_coord] : vertices) {
        data.push_back(position);
        data.push_back(color);
        data.push_back(texture_coord);
    }
    return { vertex_layout, data, indices };
}

}