#pragma once

#include <filesystem>

#include <mve/vertex_data.hpp>

namespace mve {

struct ModelData {
    VertexLayout vertex_layout;
    VertexData vertex_data;
    std::vector<uint32_t> indices;
};

ModelData load_model(const std::filesystem::path& path);

}