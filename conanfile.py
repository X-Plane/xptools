from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout
class XPToolsRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    # Use force, because for some reason override causes the include directories to not be set
    # properly on linux only.
    def requirements(self):
        self.requires("opengl/system")
        self.requires("glu/system")
        self.requires("bzip2/1.0.8")
        self.requires("expat/2.7.1")
        self.requires("libsquish/1.15")
        self.requires("libgeotiff/1.7.4")
        self.requires("proj/9.6.0", force=True)
        self.requires("freetype/2.13.3", force=True)
        self.requires("libtiff/4.7.0", force=True)
        self.requires("libjpeg/9f", force=True)
        self.requires("zlib/1.3.1")
        self.requires("libpng/1.6.50")
        self.requires("shapelib/1.6.1")
        self.requires("libcurl/8.15.0")
        self.requires("glew/2.2.0")
        self.requires("jsoncpp/1.9.6")
        if self.settings.os == "Linux":
            self.requires("fltk/1.3.9")
            self.requires("egl/system")

    def configure(self):
        self.options["libpng"].shared = False
        self.options["zlib"].shared = False
        self.options["libpng"].with_zlib = True
        self.options["glew"].shared = False
        self.options["glew"].with_glu = "system"
        self.options["proj"].build_executables = False

        if self.settings.os == "Windows":
            self.options["libcurl"].with_ssl = "schannel"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
