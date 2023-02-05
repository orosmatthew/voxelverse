#pragma once

#include "math/math.hpp"
#include <vector>

namespace mve {

/**
 * @brief Types for vertex attributes
 */
enum class VertexAttributeType {
    scalar,
    vec2,
    vec3,
    vec4,
};

/**
 * @brief List of vertex attribute types to specify the layout for vertex data
 */
using VertexLayout = std::vector<VertexAttributeType>;

/**
 * @brief Calculate number of bytes for each vertex given a layout
 * @param vertex_layout - Vertex layout to calculate bytes
 * @return - Number of bytes for each vertex in the layout
 */
[[nodiscard]] int get_vertex_layout_bytes(const VertexLayout& vertex_layout);

/**
 * @brief Class for holding vertex data
 */
class VertexData {
public:
    /**
     * @brief Construct VertexData
     * @param layout - layout for vertex data
     * @throws std::runtime_error - When layout is empty
     */
    explicit VertexData(VertexLayout layout);

    /**
     * @brief Add float to data
     * @throws std::runtime_error - When invalid type is added
     * @param value - float value
     */
    void push_back(float value);

    /**
     * @brief Add vec2 to data
     * @throws std::runtime_error - When invalid type is added
     * @param value - vec2 value
     */
    void push_back(mve::Vector2 value);

    /**
     * @brief Add vec3 to data
     * @throws std::runtime_error - When invalid type is added
     * @param value - vec3 value
     */
    void push_back(mve::Vector3 value);

    /**
     * @brief Add vec4 to data
     * @throws std::runtime_error - When invalid type is added
     * @param value - vec4 value
     */
    void push_back(mve::Vector4 value);

    /**
     * @brief Get the next attribute data type expected
     * @return The vertex attribute data type
     */
    [[nodiscard]] VertexAttributeType next_type() const noexcept;

    /**
     * @brief Get a pointer to an array of floats of vertex data
     * @return Pointer of array of floats
     */
    [[nodiscard]] const float* data_ptr() const noexcept;

    /**
     * @brief Get number of individual values in vertex data
     * @return - number of individual values in vertex data
     */
    [[nodiscard]] int data_count() const noexcept;

    /**
     * @brief Get number of complete vertices
     * @return - number of complete vertices
     */
    [[nodiscard]] int vertex_count() const noexcept;

    /**
     * @brief Determines if all vertices have all attribute values added
     * @return - true if all values are defined
     */
    [[nodiscard]] bool is_complete() const noexcept;

    /**
     * @brief Get vertex layout of data
     * @return - Vertex layout of data
     */
    [[nodiscard]] VertexLayout layout() const noexcept;

private:
    VertexLayout m_layout;
    std::vector<float> m_data;
    int m_data_count = 0;
};
}
