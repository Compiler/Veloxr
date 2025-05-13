import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.files import copy


class conanRecipe(ConanFile):
    name = "veloxr"
    settings = "os", "build_type", "arch"
    generators = "CMakeDeps"

    options = {
        "validation_layers": [True, False],
    }
    default_options = {
        "validation_layers": False,
    }

    def configure(self):
        self.options["opencv"].with_jpeg = "libjpeg-turbo"

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("opencv/4.8.1@josh/veloxr")
        self.requires("openimageio/3.0.4.0@josh/veloxr")
        # self.requires("vulkan-loader/1.3.268.0")
        self.requires("glm/1.0.1")
    
    def build_requirements(self):
        self.requires("vulkan-loader/1.3.268.0")

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.variables["VERSION"] = self.version
        tc.variables["VALIDATION_LAYERS"] = self.options.validation_layers
        tc.generate()

        for dep in self.dependencies.values():
            if not dep.package_folder: continue
            copy(self, "*", src=os.path.join(dep.package_folder, "bin"), dst="bin")
            copy(self, "*", src=os.path.join(dep.package_folder, "lib"), dst="lib")

    def layout(self):
        cmake_layout(self)

    def export_sources(self):
        folder = os.path.join(self.recipe_folder, "..")
        copy(self, "CMakeLists.txt", folder, self.export_sources_folder)
        copy(self, "src/*", folder, self.export_sources_folder)
        copy(self, "include/*", folder, self.export_sources_folder)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        # Copy the executable to the package bin directory
        copy(self, "vulkanrenderer*", src="build", dst="bin", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["veloxr_lib"]
        self.cpp_info.set_property("cmake_target_name", "veloxr::veloxr_lib")
        # Add the executable to the package info
        self.cpp_info.bindirs = ["bin"]