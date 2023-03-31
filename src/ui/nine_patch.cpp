#include "nine_patch.hpp"
#include "../logger.hpp"

NinePatch::NinePatch(
    UIPipeline& ui_pipeline,
    std::shared_ptr<mve::Texture> texture,
    NinePatchMargins margins,
    mve::Vector2i size,
    float scale)
    : m_pipeline(&ui_pipeline)
    , m_uniform_data(ui_pipeline.create_uniform_data())
    , m_scale(scale)
    , m_position(mve::Vector2(0.0f, 0.0f))
    , m_size(size)
    , m_texture(texture)
    , m_model_location(ui_pipeline.model_location())
{
    m_uniform_data.descriptor_set.write_binding(ui_pipeline.texture_binding(), *texture);
    m_uniform_data.buffer.update(ui_pipeline.model_location(), mve::Matrix4::identity().scale(mve::Vector3(scale)));

    mve::Vector2 pixel = { 1.0f / texture->size().x, 1.0f / texture->size().y };

    std::array<float, 4> x_uvs = { 0.0f, pixel.x * margins.left, 1.0f - (pixel.x * margins.right), 1.0f };
    std::array<float, 4> y_uvs = { 0.0f, pixel.y * margins.top, 1.0f - (pixel.y * margins.bottom), 1.0f };

    std::array<mve::Vector2, 16> uvs;
    int uv_index = 0;
    for (float y : y_uvs) {
        for (float x : x_uvs) {
            uvs[uv_index] = { x, y };
            uv_index++;
        }
    }

    std::array<float, 4> x_vertices = {
        0.0f, static_cast<float>(margins.left), static_cast<float>(size.x - margins.right), static_cast<float>(size.x)
    };
    std::array<float, 4> y_vertices = {
        0.0f, static_cast<float>(margins.top), static_cast<float>(size.y - margins.bottom), static_cast<float>(size.y)
    };

    std::array<mve::Vector2, 16> vertices;
    int vertex_count = 0;
    for (float y : y_vertices) {
        for (float x : x_vertices) {
            vertices[vertex_count] = { x, y };
            vertex_count++;
        }
    }

    // clang-format off
    std::vector<uint32_t> indices = {
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
        vertex_data.push_back(mve::Vector3(vertices[i].x, vertices[i].y, 0.0f));
        vertex_data.push_back(mve::Vector3(1.0f, 1.0f, 1.0f));
        vertex_data.push_back(uvs[i]);
    }
    m_vertex_buffer = ui_pipeline.renderer().create_vertex_buffer(vertex_data);
    m_index_buffer = ui_pipeline.renderer().create_index_buffer(indices);
}

void NinePatch::draw() const
{
    m_pipeline->draw(m_uniform_data.descriptor_set, m_vertex_buffer, m_index_buffer);
}

void NinePatch::set_position(const mve::Vector2& pos)
{
    m_position = pos;
    m_uniform_data.buffer.update(
        m_model_location,
        mve::Matrix4::identity()
            .scale(mve::Vector3(m_scale))
            .translate(mve::Vector3(m_position.x, m_position.y, 0.0f)));
}
void NinePatch::set_scale(float scale)
{
    m_scale = scale;
    m_uniform_data.buffer.update(
        m_model_location,
        mve::Matrix4::identity()
            .scale(mve::Vector3(m_scale))
            .translate(mve::Vector3(m_position.x, m_position.y, 0.0f)));
}

void NinePatch::update_texture(const mve::Texture& texture)
{
    // TODO: This texture could be different size from original
    m_uniform_data.descriptor_set.write_binding(m_pipeline->texture_binding(), texture);
}
