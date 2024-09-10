#include "nine_patch.hpp"

NinePatch::NinePatch(
    UIPipeline& ui_pipeline,
    const std::shared_ptr<mve::Texture>& texture,
    const NinePatchMargins margins,
    const nnm::Vector2i size,
    const float scale)
    : m_pipeline(&ui_pipeline)
    , m_uniform_data(ui_pipeline.create_uniform_data())
    , m_texture(texture)
    , m_model_location(ui_pipeline.model_location())
    , m_position(nnm::Vector2(0.0f, 0.0f))
    , m_scale(scale)
    , m_size(size)
{
    m_uniform_data.descriptor_set.write_binding(ui_pipeline.texture_binding(), *texture);
    m_uniform_data.buffer.update(
        ui_pipeline.model_location(), nnm::Transform3f().scale(nnm::Vector3f::all(scale)).matrix);

    const nnm::Vector2 pixel
        = { 1.0f / static_cast<float>(texture->size().x), 1.0f / static_cast<float>(texture->size().y) };

    const std::array x_uvs = {
        0.0f, pixel.x * static_cast<float>(margins.left), 1.0f - pixel.x * static_cast<float>(margins.right), 1.0f
    };
    const std::array y_uvs = {
        0.0f, pixel.y * static_cast<float>(margins.top), 1.0f - pixel.y * static_cast<float>(margins.bottom), 1.0f
    };

    std::array<nnm::Vector2f, 16> uvs;
    int uv_index = 0;
    for (float y : y_uvs) {
        for (float x : x_uvs) {
            uvs[uv_index] = { x, y };
            uv_index++;
        }
    }

    const std::array x_vertices = {
        0.0f, static_cast<float>(margins.left), static_cast<float>(size.x - margins.right), static_cast<float>(size.x)
    };
    const std::array y_vertices = {
        0.0f, static_cast<float>(margins.top), static_cast<float>(size.y - margins.bottom), static_cast<float>(size.y)
    };

    std::array<nnm::Vector2f, 16> vertices;
    int vertex_count = 0;
    for (float y : y_vertices) {
        for (float x : x_vertices) {
            vertices[vertex_count] = { x, y };
            vertex_count++;
        }
    }

    // clang-format off
    const std::vector<uint32_t> indices = {
        0,   5,  1,  0,  4,  5,
        1,   6,  2,  1,  5,  6,
        2,   7,  3,  2,  6,  7,
        4,   9,  5,  4,  8,  9,
        5,  10,  6,  5,  9, 10,
        6,  11,  7,  6, 10, 11,
        8,  13,  9,  8, 12, 13,
        9,  14, 10,  9, 13, 14,
        10, 15, 11, 10, 14, 15
    };
    // clang-format on

    mve::VertexData vertex_data(UIPipeline::vertex_layout());
    for (int i = 0; i < 16; i++) {
        vertex_data.push(nnm::Vector3(vertices[i].x, vertices[i].y, 0.0f));
        vertex_data.push(nnm::Vector3(1.0f, 1.0f, 1.0f));
        vertex_data.push(uvs[i]);
    }
    m_vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(vertex_data);
    m_index_buffer = ui_pipeline.renderer().create_index_buffer(indices);
}

void NinePatch::draw() const
{
    m_pipeline->draw(m_uniform_data.descriptor_set, m_vertex_buffer, m_index_buffer);
}

void NinePatch::set_position(const nnm::Vector2f& pos)
{
    m_position = pos;
    m_uniform_data.buffer.update(
        m_model_location,
        nnm::Transform3f()
            .scale(nnm::Vector3f::all(m_scale))
            .translate(nnm::Vector3(m_position.x, m_position.y, 0.0f))
            .matrix);
}
void NinePatch::set_scale(const float scale)
{
    m_scale = scale;
    m_uniform_data.buffer.update(
        m_model_location,
        nnm::Transform3f()
            .scale(nnm::Vector3f::all(m_scale))
            .translate(nnm::Vector3(m_position.x, m_position.y, 0.0f))
            .matrix);
}

void NinePatch::update_texture(const mve::Texture& texture)
{
    // TODO: This texture could be different size from original
    m_uniform_data.descriptor_set.write_binding(m_pipeline->texture_binding(), texture);
}
