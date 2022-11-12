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
        void add_data(float value);

        /**
         * Add vec2 to data
         * @param value - vec2 value
         */
        void add_data(glm::vec2 value);

        /**
         * Add vec3 to data
         * @param value - vec3 value
         */
        void add_data(glm::vec3 value);

        /**
         * Add vec4 to data
         * @param value - vec4 value
         */
        void add_data(glm::vec4 value);

        /**
         * Get the next attribute data type expected
         * @return The vertex attribute data type
         */
        [[nodiscard]] VertexAttributeType get_next_type() const;

        /**
         * Get a pointer to an array of floats of vertex data
         * @return Pointer of array of floats
         */
        [[nodiscard]] const float *get_data_ptr() const;

    private:
        /// Layout for vertex data
        VertexLayout m_layout;

        /// Array of floats for the actual data
        std::vector<float> m_data;

        /// Count of how many values are added
        int m_type_count = 0;
    };
}
