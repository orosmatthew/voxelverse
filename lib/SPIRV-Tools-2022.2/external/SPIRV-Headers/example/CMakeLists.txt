add_library(SPIRV-Headers-example
        example.cpp)
target_include_directories(SPIRV-Headers-example
	PRIVATE ${SPIRV-Headers_SOURCE_DIR}/include)
