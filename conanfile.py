from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Recipe(ConanFile):
    
    settings = ( "os", "compiler", "build_type", "arch" )
    generators = ( "CMakeDeps", "CMakeToolchain" )

    def requirements(self):
        self.requires("llhttp/9.3.0")
        self.requires("stduuid/1.2.3")
        self.requires("concurrentqueue/1.0.4")
        self.requires("gtest/1.17.0")
        self.requires("re2/20251105")
        self.requires("libidn2/2.3.8")
        self.requires("libpsl/0.21.5")
        self.requires("libcurl/8.18.0")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.18]")

    def layout(self):
        cmake_layout(self)
