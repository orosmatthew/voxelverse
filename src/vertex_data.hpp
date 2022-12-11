#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace mve {

/**
 * Types for vertex attributes
 */
enum class VertexAttributeType {
    e_float,
    e_vec2,
    e_vec3,
    e_vec4,
};

/**
 * List of vertex attribute types to specify the layout for vertex data
 */
using VertexLayout = std::vector<VertexAttributeType>;

/**
 * Calculate number of bytes for each vertex given a layout
 * @param vertex_layout - Vertex layout to calculate bytes
 * @return - Number of bytes for each vertex in the layout
 */
[[nodiscard]] int get_vertex_layout_bytes(const VertexLayout& vertex_layout);

/**
 * Class for holding vertex data
 */
class VertexData {
public:
    /**
     * Construct VertexData
     * @param layout - layout for vertex data
     */
    explicit VertexData(VertexLayout layout);

    /**
     * Add float to data
     * @param value - float value
     */
    void push_back(float value);

    /**
     * Add vec2 to data
     * @param value - vec2 value
     */
    void push_back(glm::vec2 value);

    /**
     * Add vec3 to data
     * @param value - vec3 value
     */
    void push_back(glm::vec3 value);

    /**
     * Add vec4 to data
     * @param value - vec4 value
     */
    void push_back(glm::vec4 value);

    /**
     * Get the next attribute data type expected
     * @return The vertex attribute data type
     */
    [[nodiscard]] VertexAttributeType get_next_type() const;

    /**
     * Get a pointer to an array of floats of vertex data
     * @return Pointer of array of floats
     */
    [[nodiscard]] const float* get_data_ptr() const;

    /**
     * Get number of values in vertex data
     * @return - number of values in vertex data
     */
    [[nodiscard]] int get_data_count() const;

    /**
     * Get number of complete vertices
     * @return - number of complete vertices
     */
    [[nodiscard]] int get_vertex_count() const;

    /**
     * Determines if all vertices have all attribute values added
     * @return - true if all values are defined
     */
    [[nodiscard]] bool is_complete() const;

    /**
     * Get vertex layout of data
     * @return - Vertex layout of data
     */
    [[nodiscard]] VertexLayout get_layout() const;

private:
    VertexLayout m_layout;
    std::vector<float> m_data;
    int m_data_count = 0;
};
}
