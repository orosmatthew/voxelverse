#include <catch_amalgamated.hpp>

#include "../src/vertex_data.hpp"

TEST_CASE("Test VertexData", "[VertexData]")
{
    auto layout = mve::VertexLayout();
    layout.push_back(mve::VertexAttributeType::e_float);
    layout.push_back(mve::VertexAttributeType::e_vec2);
    layout.push_back(mve::VertexAttributeType::e_vec3);
    layout.push_back(mve::VertexAttributeType::e_vec4);

    REQUIRE(
        mve::get_vertex_layout_bytes(layout)
        == sizeof(float) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec4));

    auto vertex_data = mve::VertexData(layout);

    REQUIRE(vertex_data.layout() == layout);

    REQUIRE(vertex_data.data_count() == 0);

    REQUIRE(vertex_data.vertex_count() == 0);

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::e_float);
    vertex_data.push_back(2.0f);

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::e_vec2);
    vertex_data.push_back(glm::vec2(0, 1));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::e_vec3);
    vertex_data.push_back(glm::vec3(0, 1, 2));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::e_vec4);
    vertex_data.push_back(glm::vec4(0, 1, 2, 3));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::e_float);

    REQUIRE(vertex_data.is_complete() == true);

    REQUIRE(vertex_data.vertex_count() == 1);

    vertex_data.push_back(3.0f);
    REQUIRE(vertex_data.is_complete() == false);

    REQUIRE(vertex_data.data_count() == 5);

    REQUIRE(vertex_data.vertex_count() == 1);

    REQUIRE(vertex_data.data_ptr() != nullptr);
}