from conan import ConanFile


class App(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    build_policy = "always"
    generators = [
        "CMakeDeps",
        "CMakeToolchain",
    ]
    requires = [
        "glfw/3.3.7",
        "vulkan-headers/1.3.216.0",
        "vulkan-loader/1.3.216.0",
        "vulkan-validationlayers/1.3.216.0",
        "shaderc/2021.1",
        "spdlog/1.10.0",
        ("spirv-tools/1.3.216.0", "override"),
    ]
