from conans import ConanFile, CMake, tools
import os


class RaytracingConan(ConanFile):
    name = "raytracing"
    version = "0.1.0"
    license = "GNU GPL v3.0"
    author = "Samuel Dowling <samuel.dowling@protonmail.com>"
    url = "https://github.com/samuel-emrys/finsim"
    description = "Raytracing in a day implementation"
    topics = ("gaming", "raytracing", "graphics")
    test_type = "explicit"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_docs": [True, False],
        "with_tests": [True, False],
        "with_coverage": [True, False],
        "with_clang_tidy": [True, False],
        "with_cppcheck": [True, False],
        "with_include_what_you_use": [True, False],
        "with_ipo": [True, False],
        "with_asan": [True, False],
        "with_ubsan": [True, False],
        "with_leaksan": [True, False],
        "with_memsan": [True, False],
        "with_threadsan": [True, False],
        "with_warnings_as_errors": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_docs": False,
        "with_tests": False,
        "with_coverage": True,
        "with_clang_tidy": False,
        "with_cppcheck": False,
        "with_include_what_you_use": False,
        "with_ipo": False,
        "with_asan": False,
        "with_ubsan": False,
        "with_leaksan": False,
        "with_memsan": False,
        "with_threadsan": False,
        "with_warnings_as_errors": False,
    }
    exports_sources = (
        "src/*",
        "include/*",
        "docs/*",
        "tests/*",
        "cmake/*",
        "LICENSE",
        "CMakeLists.txt",
    )
    generators = (
        "cmake_find_package",
        "ycm",
    )
    _cmake = None

    @property
    def _source_subfolder(self):
        return "."

    @property
    def _build_subfolder(self):
        return "build_subfolder"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def validate(self):
        if self.settings.compiler.cppstd:
            tools.check_min_cppstd(self, "20")

    def build_requirements(self):
        self.tool_requires("cmake/3.22.0")
        if self.options.with_tests:
            self.test_requires("gtest/cci.20210126")
        if self.options.with_docs:
            self.tool_requires("doxygen/1.9.2")

    def requirements(self):
        self.requires("libjpeg/9d")
        self.requires("eigen/3.4.0")
        self.requires("date/3.0.1")
        self.requires("gsl-lite/0.39.0")
        self.requires("fmt/8.1.1")
        self.requires("cli11/2.2.0")
        #self.requires("onetbb/2021.3.0")
        self.requires("pngpp/0.2.10")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.definitions["CMAKE_PREFIX_PATH"] = "{pwd}/venv/bin".format(
            pwd=os.getcwd()
        )
        self._cmake.definitions["BUILD_DOCS"] = self.options.with_docs
        self._cmake.definitions["ENABLE_CLANG_TIDY"] = self.options.with_clang_tidy
        self._cmake.definitions["ENABLE_COVERAGE"] = self.options.with_coverage
        self._cmake.definitions["ENABLE_CPPCHECK"] = self.options.with_cppcheck
        self._cmake.definitions["ENABLE_INCLUDE_WHAT_YOU_USE"] = self.options.with_include_what_you_use
        self._cmake.definitions["ENABLE_IPO"] = self.options.with_ipo
        self._cmake.definitions["ENABLE_SANITIZER_ADDRESS"] = self.options.with_asan
        self._cmake.definitions["ENABLE_SANITIZER_LEAK"] = self.options.with_leaksan
        self._cmake.definitions["ENABLE_SANITIZER_MEMORY"] = self.options.with_memsan
        self._cmake.definitions["ENABLE_SANITIZER_THREAD"] = self.options.with_threadsan
        self._cmake.definitions["ENABLE_SANITIZER_UNDEFINED_BEHAVIOR"] = self.options.with_ubsan
        self._cmake.definitions["ENABLE_TESTING"] = self.options.with_tests
        self._cmake.definitions["WARNINGS_AS_ERRORS"] = self.options.with_warnings_as_errors
        self._cmake.configure(source_folder=self._source_subfolder, build_folder=self._build_subfolder)
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.with_tests:
            os.environ["GTEST_COLOR"] = "1"
            cmake.test(args=["--", "ARGS=-V"], output_on_failure=True)
