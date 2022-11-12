#include <catch_amalgamated.hpp>

#include "../src/vertex_data.hpp"

TEST_CASE("Test VertexData", "[VertexData]")
{
    auto layout = mve::VertexLayout();
    layout.push_back(mve::VertexAttributeType::e_float);
    layout.push_back(mve::VertexAttributeType::e_vec2);
    layout.push_back(mve::VertexAttributeType::e_vec3);
    layout.push_back(mve::VertexAttributeType::e_vec4);

    auto vertex_data = mve::VertexData(layout);

    REQUIRE(vertex_data.get_data_count() == 0);

    REQUIRE(vertex_data.get_next_type() == mve::VertexAttributeType::e_float);
    vertex_data.add_data(2.0f);

    REQUIRE(vertex_data.get_next_type() == mve::VertexAttributeType::e_vec2);
    vertex_data.add_data(glm::vec2(0, 1));

    REQUIRE(vertex_data.get_next_type() == mve::VertexAttributeType::e_vec3);
    vertex_data.add_data(glm::vec3(0, 1, 2));

    REQUIRE(vertex_data.get_next_type() == mve::VertexAttributeType::e_vec4);
    vertex_data.add_data(glm::vec4(0, 1, 2, 3));

    REQUIRE(vertex_data.get_next_type() == mve::VertexAttributeType::e_float);

    REQUIRE(vertex_data.is_complete() == true);

    vertex_data.add_data(3.0f);
    REQUIRE(vertex_data.is_complete() == false);

    REQUIRE(vertex_data.get_data_ptr() != nullptr);
}