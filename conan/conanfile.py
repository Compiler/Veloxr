import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.files import copy


class conanRecipe(ConanFile):
    name = "Veloxr"
    settings = "os", "build_type", "arch"
    generators = "CMakeDeps"

    def configure(self):
        self.options["opencv"].with_jpeg = 'libjpeg-turbo'


    def requirements(self):
        self.requires("glfw/3.4")
        # self.requires("openexr/3.2.3", override=True)
        # self.requires("jasper/4.2.0", override=True)
        # self.requires("libjpeg/9f", override=True)
        self.requires("opencv/4.8.1@josh/veloxr")
        self.requires("openimageio/3.0.4.0@josh/veloxr")
        self.requires("vulkan-loader/1.3.268.0")
        # self.requires("openimageio/2.5.16.0")

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.variables["VERSION"] = self.version
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
        copy(self, "aiengine/*", folder, self.export_sources_folder)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()