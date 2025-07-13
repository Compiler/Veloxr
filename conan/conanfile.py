import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.files import copy


class conanRecipe(ConanFile):
    name = "veloxr"
    settings = "os", "build_type", "arch"
    generators = "CMakeDeps"
    settings = "os", "arch", "compiler", "build_type"

    options = {
        "validation_layers": [True, False],
        "build_video": [True, False],
        "build_photo": [True, False],
    }
    default_options = {
        "validation_layers": False,
        "build_video": False,
        "build_photo": False,
    }

    def configure(self):
        # if self.options.build_photo:
        self.options["opencv"].with_jpeg = False
        self.options["opencv"].with_png = False
        self.options["opencv"].with_tiff = False
        self.options["opencv"].with_jpeg2000 = False
        self.options["opencv"].with_openexr = False
        self.options["opencv"].with_eigen = False
        self.options["opencv"].with_webp = False
        self.options["opencv"].with_quirc = False

        self.options["libtiff"].jpeg = "libjpeg-turbo"
        self.options["openimageio"].shared = True  # Build OIIO as a shared library
        self.options["openimageio"].with_libjpeg = "libjpeg-turbo"

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("opencv/4.8.1-topaz")
        self.requires("openimageio/3.0.4.0-topaz")
        self.requires("glm/1.0.1")
        if self.settings.os == "Macos":
            self.requires("moltenvk/1.2.2")

    def build_requirements(self):
        self.requires("vulkan-loader/1.3.268.0")

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.variables["VERSION"] = self.version
        print(f"[Conan] Setting VALIDATION_LAYERS to: {self.options.validation_layers}")
        tc.variables["VALIDATION_LAYERS"] = self.options.validation_layers
        tc.generate()

        for dep in self.dependencies.values():
            if not dep.package_folder:
                continue
            copy(self, "*", src=os.path.join(dep.package_folder, "bin"), dst="bin")
            copy(self, "*", src=os.path.join(dep.package_folder, "lib"), dst="lib")

    def layout(self):
        cmake_layout(self)

    def export_sources(self):
        folder = os.path.join(self.recipe_folder, "..")
        copy(self, "CMakeLists.txt", folder, self.export_sources_folder)
        copy(self, "src/*", folder, self.export_sources_folder)
        copy(self, "include/*", folder, self.export_sources_folder)
        copy(self, "spirv/*", folder, self.export_sources_folder)
        copy(self, "conan/fix_mac_libs.cmake", folder, self.export_sources_folder)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        if self.settings.os == "Windows":
            copy(
                self,
                "vulkan-1.dll",
                os.path.join(self.build_folder),
                os.path.join(self.package_folder, "bin"),
                keep_path=False,
            )

        copy(
            self,
            "vulkanrenderer.exe",
            os.path.join(self.build_folder),
            os.path.join(self.package_folder, "bin"),
            keep_path=False,
        )
        copy(
            self,
            "spirv/*.spv",
            self.source_folder,
            os.path.join(self.package_folder, "spirv"),
            keep_path=False,
        )
        copy(
            self,
            "spirv/glslc",
            self.source_folder,
            os.path.join(self.package_folder, "bin"),
            keep_path=False,
        )

    def package_info(self):
        self.cpp_info.libs = ["veloxr_lib"]
        self.cpp_info.set_property("cmake_target_name", "veloxr::veloxr")
        # Add the executable to the package info
        self.cpp_info.bindirs = ["bin"]
