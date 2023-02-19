#include <catch_amalgamated.hpp>

#include "../src/mve/math/math.hpp"
#include "../src/mve/vertex_data.hpp"

TEST_CASE("Test VertexData", "[VertexData]")
{
    auto layout = mve::VertexLayout();
    layout.push_back(mve::VertexAttributeType::scalar);
    layout.push_back(mve::VertexAttributeType::vec2);
    layout.push_back(mve::VertexAttributeType::vec3);
    layout.push_back(mve::VertexAttributeType::vec4);

    REQUIRE(
        mve::get_vertex_layout_bytes(layout)
        == sizeof(float) + sizeof(mve::Vector2) + sizeof(mve::Vector3) + sizeof(mve::Vector4));

    auto vertex_data = mve::VertexData(layout);

    REQUIRE(vertex_data.layout() == layout);

    REQUIRE(vertex_data.data_count() == 0);

    REQUIRE(vertex_data.vertex_count() == 0);

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::scalar);
    vertex_data.push_back(2.0f);

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::vec2);
    vertex_data.push_back(mve::Vector2(0, 1));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::vec3);
    vertex_data.push_back(mve::Vector3(0, 1, 2));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::vec4);
    vertex_data.push_back(mve::Vector4(0, 1, 2, 3));

    REQUIRE(vertex_data.next_type() == mve::VertexAttributeType::scalar);

    REQUIRE(vertex_data.is_complete() == true);

    REQUIRE(vertex_data.vertex_count() == 1);

    vertex_data.push_back(3.0f);
    REQUIRE(vertex_data.is_complete() == false);

    REQUIRE(vertex_data.data_count() == 5);

    REQUIRE(vertex_data.vertex_count() == 1);

    REQUIRE(vertex_data.data_ptr() != nullptr);
}