from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Recipe(ConanFile):
    
    settings = ( "os", "compiler", "build_type", "arch" )
    generators = ( "CMakeDeps", "CMakeToolchain" )

    def requirements(self):
        self.requires("llhttp/9.3.0")
        self.requires("stduuid/1.2.3")
        self.requires("concurrentqueue/1.0.4")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.18]")

    def layout(self):
        cmake_layout(self)
