from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Recipe(ConanFile):
    
    settings = ( "os", "compiler", "build_type", "arch" )
    generators = ( "CMakeDeps", "CMakeToolchain" )

    def requirements(self):
        self.requires("concurrentqueue/1.0.4")
        self.requires("gtest/1.17.0")
        self.requires("re2/20251105")
        self.requires("libidn2/2.3.8")
        self.requires("libpsl/0.21.5")
        self.requires("libcurl/8.18.0")
        self.requires("lexbor/2.6.0")
        self.requires("nlohmann_json/3.12.0")
        self.requires("boost/1.90.0")
        self.requires("libpqxx/8.0.1")
        self.requires("sqlite3/3.53.1")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.18]")

    def configure(self):
        self.settings.compiler.cppstd = "23"

    def layout(self):
        cmake_layout(self)
