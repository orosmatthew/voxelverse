#include "wire_box_mesh.hpp"

// ReSharper disable once CppUnusedIncludeDirective
// Needed for GCC std::as_const
#include <utility>

#include "world_renderer.hpp"

WireBoxMesh::WireBoxMesh(
    mve::Renderer& renderer,
    mve::GraphicsPipeline& pipeline,
    const mve::ShaderDescriptorSet& set,
    const mve::ShaderDescriptorBinding& uniform_buffer_binding,
    const BoundingBox& box,
    float width,
    nnm::Vector3f color)
    : m_renderer(&renderer)
    , m_descriptor_set(pipeline.create_descriptor_set(set))
    , m_uniform_buffer(renderer.create_uniform_buffer(uniform_buffer_binding))
    , m_model_location(uniform_buffer_binding.member("model").location())
{
    m_descriptor_set.write_binding(uniform_buffer_binding, m_uniform_buffer);

    m_uniform_buffer.update(uniform_buffer_binding.member("fog_influence").location(), 0.0f);

    mve::VertexData data(WorldRenderer::vertex_layout());

    Rect3 rect = bounding_box_to_rect3(box);

    std::vector<std::pair<nnm::Vector3f, nnm::Vector3f>> edges = rect3_to_edges(rect);

    constexpr float z_offset = 0.005f; // To prevent z-fighting
    auto scale = nnm::Transform3f::from_basis_translation(
        nnm::Basis3f::from_scale(nnm::Vector3f::all(1.0f - width + z_offset)), { 0, 0, 0 });

    MeshData combined_data;
    for (auto& [from, to] : edges) {
        MeshData rect_mesh = create_rect_mesh(from.transform(scale), to.transform(scale), width);
        combine_mesh_data(combined_data, rect_mesh);
    }

    for (const nnm::Vector3f& vertex : combined_data.vertices) {
        data.push_back(vertex);
        data.push_back(color); // TODO: Fix broken color
        data.push_back({ 0, 0 });
        data.push_back(1.0f);
    }

    m_uniform_buffer.update(m_model_location, nnm::Matrix4f::identity());

    m_mesh_buffers = { .vertex_buffer = renderer.create_vertex_buffer(data),
                       .index_buffer = renderer.create_index_buffer(combined_data.indices) };
}
void WireBoxMesh::draw(const mve::DescriptorSet& global_set) const
{
    if (m_mesh_buffers.has_value()) {
        m_renderer->bind_descriptor_sets(global_set, m_descriptor_set);
        m_renderer->bind_vertex_buffer(m_mesh_buffers->vertex_buffer);
        m_renderer->draw_index_buffer(m_mesh_buffers->index_buffer);
    }
}
void WireBoxMesh::set_position(const nnm::Vector3f position)
{
    m_uniform_buffer.update(m_model_location, nnm::Transform3f().translate(position).matrix);
}
WireBoxMesh::MeshData WireBoxMesh::create_rect_mesh(
    const float length, const float width, const nnm::Transform3f& transform)
{
    MeshData mesh_data;
    float hl = length / 2.0f;
    float hw = width / 2.0f;
    // Front
    mesh_data.vertices.emplace_back(-hl, -hw, hw);
    mesh_data.vertices.emplace_back(hl, -hw, hw);
    mesh_data.vertices.emplace_back(hl, -hw, -hw);
    mesh_data.vertices.emplace_back(-hl, -hw, -hw);
    // Back
    mesh_data.vertices.emplace_back(hl, hw, hw);
    mesh_data.vertices.emplace_back(-hl, hw, hw);
    mesh_data.vertices.emplace_back(-hl, hw, -hw);
    mesh_data.vertices.emplace_back(hl, hw, -hw);
    // Left
    mesh_data.vertices.emplace_back(-hl, hw, hw);
    mesh_data.vertices.emplace_back(-hl, -hw, hw);
    mesh_data.vertices.emplace_back(-hl, -hw, -hw);
    mesh_data.vertices.emplace_back(-hl, hw, -hw);
    // Right
    mesh_data.vertices.emplace_back(hl, -hw, hw);
    mesh_data.vertices.emplace_back(hl, hw, hw);
    mesh_data.vertices.emplace_back(hl, hw, -hw);
    mesh_data.vertices.emplace_back(hl, -hw, -hw);
    // Top
    mesh_data.vertices.emplace_back(-hl, hw, hw);
    mesh_data.vertices.emplace_back(hl, hw, hw);
    mesh_data.vertices.emplace_back(hl, -hw, hw);
    mesh_data.vertices.emplace_back(-hl, -hw, hw);
    // Bottom
    mesh_data.vertices.emplace_back(hl, hw, -hw);
    mesh_data.vertices.emplace_back(-hl, hw, -hw);
    mesh_data.vertices.emplace_back(-hl, -hw, -hw);
    mesh_data.vertices.emplace_back(hl, -hw, -hw);

    std::ranges::transform(
        std::as_const(mesh_data.vertices), mesh_data.vertices.begin(), [&](const nnm::Vector3f vertex) {
            return vertex.transform(transform);
        });

    // const std::array<uint32_t, 6> quad_indices = { 0, 3, 2, 0, 2, 1 };
    const std::array<uint32_t, 6> quad_indices = { 0, 2, 3, 0, 1, 2 };
    for (int q = 0; q < 6; q++) {
        for (const uint32_t i : quad_indices) {
            mesh_data.indices.push_back(i + q * 4);
        }
    }
    return mesh_data;
}
WireBoxMesh::MeshData WireBoxMesh::create_rect_mesh(const nnm::Vector3f from, const nnm::Vector3f to, const float width)
{
    const nnm::Vector3f dir = (to - from).normalize().abs();
    const nnm::QuaternionF quat = nnm::QuaternionF::from_vector_to_vector({ 0, 0, 1 }, dir);
    const auto basis = nnm::Basis3f::from_rotation_quaternion(quat);

    const auto matrix
        = nnm::Transform3f()
              .transform(nnm::Transform3f::from_basis(basis))
              .rotate_axis_angle({ 0, 1, 0 }, nnm::radians(90.0f))
              .translate((to + from) / 2.0f);
    MeshData rect_mesh = create_rect_mesh(from.distance(to) + width, width, matrix);
    return rect_mesh;
}

void WireBoxMesh::combine_mesh_data(MeshData& data, const MeshData& other)
{
    const uint32_t indices_offset = data.vertices.size();
    for (const nnm::Vector3f& vertex : other.vertices) {
        data.vertices.push_back(vertex);
    }
    for (const uint32_t index : other.indices) {
        data.indices.push_back(index + indices_offset);
    }
}
