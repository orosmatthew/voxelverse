#include "select_box_mesh.hpp"

#include "logger.hpp"
#include "world_renderer.hpp"

SelectBoxMesh::SelectBoxMesh(
    mve::Renderer& renderer,
    mve::GraphicsPipeline& pipeline,
    const mve::ShaderDescriptorSet& set,
    const mve::ShaderDescriptorBinding& uniform_buffer_binding)
    : m_renderer(&renderer)
    , m_descriptor_set(pipeline.create_descriptor_set(set))
    , m_uniform_buffer(renderer.create_uniform_buffer(uniform_buffer_binding))
    , m_model_location(uniform_buffer_binding.member("model").location())
{
    m_descriptor_set.write_binding(uniform_buffer_binding, m_uniform_buffer);

    m_uniform_buffer.update(uniform_buffer_binding.member("fog_influence").location(), 0.0f);

    mve::VertexData data(WorldRenderer::vertex_layout());

    std::vector<std::pair<mve::Vector3, mve::Vector3>> edges;
    edges.push_back({ { -0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, -0.5f } }); // Front left
    edges.push_back({ { 0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, -0.5f } }); // Front right
    edges.push_back({ { -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f } }); // Front top
    edges.push_back({ { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f } }); // Front bottom
    edges.push_back({ { -0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, -0.5f } }); // Back left
    edges.push_back({ { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, -0.5f } }); // Back right
    edges.push_back({ { -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f } }); // Back top
    edges.push_back({ { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f } }); // Back bottom
    edges.push_back({ { -0.5f, -0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f } }); // Left top
    edges.push_back({ { -0.5f, -0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f } }); // Left bottom
    edges.push_back({ { 0.5f, -0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f } }); // Right top
    edges.push_back({ { 0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f } }); // Right bottom

    const float z_offset = 0.005f; // To prevent z-fighting
    const float width = 0.01f;
    mve::Matrix4 scale = mve::Matrix4::from_basis_translation(
        mve::Matrix3::from_scale(mve::Vector3(1.0f - width + z_offset)), { 0, 0, 0 });

    MeshData combined_data;
    for (auto& edge : edges) {
        MeshData rect_mesh = create_rect_mesh(edge.first.transform(scale), edge.second.transform(scale), width);
        combine_mesh_data(combined_data, rect_mesh);
    }

    for (mve::Vector3& vertex : combined_data.vertices) {
        data.push_back(vertex);
        data.push_back({ 0.0f, 0.0f, 0.0f });
        data.push_back({ 0, 0 });
    }

    m_mesh_buffers = { .vertex_buffer = m_renderer->create_vertex_buffer(data),
                       .index_buffer = m_renderer->create_index_buffer(combined_data.indices) };
}
void SelectBoxMesh::draw(const mve::DescriptorSet& global_set)
{
    if (m_mesh_buffers.has_value()) {
        m_renderer->bind_descriptor_sets(global_set, m_descriptor_set);
        m_renderer->bind_vertex_buffer(m_mesh_buffers->vertex_buffer);
        m_renderer->draw_index_buffer(m_mesh_buffers->index_buffer);
    }
}
void SelectBoxMesh::set_position(mve::Vector3 position)
{
    m_uniform_buffer.update(m_model_location, mve::Matrix4::identity().translate(position));
}
SelectBoxMesh::MeshData SelectBoxMesh::create_rect_mesh(float length, float width, const mve::Matrix4& matrix)
{
    MeshData mesh_data;
    float hl = length / 2.0f;
    float hw = width / 2.0f;
    // Front
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, -hw));
    // Back
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, -hw));
    // Left
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, -hw));
    // Right
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, -hw));
    // Top
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, hw));
    // Bottom
    mesh_data.vertices.push_back(mve::Vector3(hl, hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(-hl, -hw, -hw));
    mesh_data.vertices.push_back(mve::Vector3(hl, -hw, -hw));

    for (mve::Vector3& vertex : mesh_data.vertices) {
        vertex = vertex.transform(matrix);
    }

    const std::array<uint32_t, 6> quad_indices = { 0, 3, 2, 0, 2, 1 };
    for (int q = 0; q < 6; q++) {
        for (uint32_t i : quad_indices) {
            mesh_data.indices.push_back(i + (q * 4));
        }
    }
    return mesh_data;
}
SelectBoxMesh::MeshData SelectBoxMesh::create_rect_mesh(mve::Vector3 from, mve::Vector3 to, float width)
{
    mve::Vector3 dir = (to - from).normalize().abs();
    mve::Quaternion quat = mve::Quaternion::from_vector3_to_vector3({ 0, 0, 1 }, dir);
    mve::Matrix3 basis = mve::Matrix3::from_quaternion(quat);

    mve::Matrix4 matrix
        = mve::Matrix4::identity().rotate(basis).rotate({ 0, 1, 0 }, mve::radians(90.0f)).translate((to + from) / 2.0f);
    MeshData rect_mesh = create_rect_mesh(from.distance_to(to) + width, width, matrix);
    return rect_mesh;
}

void SelectBoxMesh::combine_mesh_data(SelectBoxMesh::MeshData& data, const SelectBoxMesh::MeshData& other)
{
    uint32_t indices_offset = data.vertices.size();
    for (const mve::Vector3& vertex : other.vertices) {
        data.vertices.push_back(vertex);
    }
    for (uint32_t index : other.indices) {
        data.indices.push_back(index + indices_offset);
    }
}
