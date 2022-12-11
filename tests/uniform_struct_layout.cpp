#include <catch_amalgamated.hpp>

#include "../src/uniform_struct_layout.hpp"

TEST_CASE("Test UniformStructLayout", "[UniformStructLayout]")
{
    auto struct_layout = mve::UniformStructLayout("MyStruct");

    REQUIRE(struct_layout.name() == "MyStruct");

    struct_layout.push_back("my_float", mve::UniformType::e_float);

    REQUIRE(struct_layout.size_bytes() == 4);

    REQUIRE(struct_layout.location_of("my_float") == mve::UniformLocation(0));

    struct_layout.push_back("my_mat4", mve::UniformType::e_mat4);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64));

    REQUIRE(struct_layout.location_of("my_mat4") == mve::UniformLocation(16));

    struct_layout.push_back("my_vec3", mve::UniformType::e_vec3);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64 + 12));

    REQUIRE(struct_layout.location_of("my_vec3") == mve::UniformLocation(4 + 12 + 64));

    struct_layout.push_back("my_vec2", mve::UniformType::e_vec2);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64 + 12 + 4 + 8));

    REQUIRE(struct_layout.location_of("my_vec2") == mve::UniformLocation(4 + 12 + 64 + 12 + 4));

    struct_layout.push_back("my_vec4", mve::UniformType::e_vec4);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64 + 12 + 4 + 8 + 8 + 16));

    REQUIRE(struct_layout.location_of("my_vec4") == mve::UniformLocation(4 + 12 + 64 + 12 + 4 + 8 + 8));

    struct_layout.push_back("my_mat2", mve::UniformType::e_mat2);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64 + 12 + 4 + 8 + 8 + 16 + 16));

    REQUIRE(struct_layout.location_of("my_mat2") == mve::UniformLocation(4 + 12 + 64 + 12 + 4 + 8 + 8 + 16));

    struct_layout.push_back("my_mat3", mve::UniformType::e_mat3);

    REQUIRE(struct_layout.size_bytes() == (4 + 12 + 64 + 12 + 4 + 8 + 8 + 16 + 16 + 36));

    REQUIRE(struct_layout.location_of("my_mat3") == mve::UniformLocation(4 + 12 + 64 + 12 + 4 + 8 + 8 + 16 + 16));
}