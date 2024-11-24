from setuptools import setup, Extension
import pybind11
import os
import subprocess
import sysconfig
from conan import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

# Helper function to locate ReaDDy includes and libraries via Conan
class SimpleActionConan(ConanFile):
    name = "simple_action"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = "spdlog/1.10.0", "nlohmann_json/3.10.3", "fmt/8.1.1"
    options = {
        "header_only": [True, False]
    }
    default_options = {
        "header_only": True
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

# Set up environment variables from the bash script
prefix = os.getenv("PREFIX", "/home/earkfeld/miniforge3/envs/readdy2dev-action_reaction")
python_include_dir = subprocess.check_output([f"{prefix}/bin/python", "-c", "import sysconfig; print(sysconfig.get_path('include'))"]).decode().strip()
lib_path = ""
py_ver = os.getenv("PY_VER", "3.9")

if os.uname().sysname == "Darwin":
    lib_path = f"{prefix}/lib/libpython{py_ver}.dylib"
else:
    lib_path = f"{prefix}/lib/libpython{py_ver}.so"

extra_link_args = [lib_path]

# Extension module configuration
simple_action_module = Extension(
    "SimpleAction",
    sources=["/home/earkfeld/CLionProjects/readdy-dev/examples/simple_action/binding.cpp"],
    include_dirs=[
        pybind11.get_include(),
        pybind11.get_include(user=True),
        python_include_dir,
        "/home/earkfeld/CLionProjects/readdy-dev/build/Debug/generators"
    ],
    libraries=["readdy", "readdy_kernel_cpu"],
    library_dirs=["/home/earkfeld/CLionProjects/readdy-dev/build/Debug/generators"],
    extra_compile_args=["-std=c++17"],
    extra_link_args=extra_link_args,
    language="c++"
)

# Setup script
setup(
    name="bindings",
    version="0.1",
    author="Eric Arkfeld",
    description="Bindings for ReaDDy custom user-defined actions",
    ext_modules=[simple_action_module],
    install_requires=["pybind11", "conan"],
    zip_safe=False,
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)


# from setuptools import setup, Extension
# import pybind11
# import os
# import subprocess
# import sysconfig
#
# # Helper function to locate ReaDDy includes and libraries
# def find_readdy():
#     readdy_include_dir = os.getenv("READDY_INCLUDE_DIR")
#     readdy_library = os.getenv("READDY_LIBRARY")
#     readdy_cpu_library = os.getenv("READDY_CPU_LIBRARY")
#
#     if not (readdy_include_dir and readdy_library and readdy_cpu_library):
#         raise RuntimeError("Please set the READDY_INCLUDE_DIR, READDY_LIBRARY, and READDY_CPU_LIBRARY environment variables.")
#
#     return readdy_include_dir, [readdy_library, readdy_cpu_library]
#
# # Find ReaDDy includes and libraries
# readdy_include_dir, readdy_libraries = find_readdy()
#
# # Set C++ standard
# cxx_standard = "-std=c++17"
#
# # Compiler arguments and linking options
# extra_compile_args = [cxx_standard]
# extra_link_args = []
#
# # Set up environment variables from the bash script
# prefix = os.getenv("PREFIX", "/home/earkfeld/miniforge3/envs/readdy2dev-action_reaction")
# python_include_dir = subprocess.check_output([f"{prefix}/bin/python", "-c", "import sysconfig; print(sysconfig.get_path('include'))"]).decode().strip()
# lib_path = ""
# py_ver = os.getenv("PY_VER", "3.9")
#
# if os.uname().sysname == "Darwin":
#     lib_path = f"{prefix}/lib/libpython{py_ver}.dylib"
# else:
#     lib_path = f"{prefix}/lib/libpython{py_ver}.so"
#
# extra_link_args.append(lib_path)
#
# # Extension module configuration
# simple_action_module = Extension(
#     "SimpleAction",
#     sources=["/home/earkfeld/CLionProjects/readdy-dev/examples/simple_action/binding.cpp"],
#     include_dirs=[
#         pybind11.get_include(),
#         pybind11.get_include(user=True),
#         readdy_include_dir,
#         python_include_dir,
#         "/home/earkfeld/CLionProjects/readdy-dev/build/Debug/generators"
#     ],
#     libraries=["readdy", "readdy_kernel_cpu"],
#     library_dirs=[os.path.dirname(lib) for lib in readdy_libraries],
#     extra_compile_args=extra_compile_args,
#     extra_link_args=extra_link_args,
#     language="c++"
# )
#
# # Setup script
# setup(
#     name="bindings",
#     version="0.1",
#     author="Eric Arkfeld",
#     description="Bindings for ReaDDy custom user-defined actions",
#     ext_modules=[simple_action_module],
#     install_requires=["pybind11"],
#     zip_safe=False,
#     classifiers=[
#         "Programming Language :: Python :: 3",
#         "License :: OSI Approved :: MIT License",
#         "Operating System :: OS Independent",
#     ],
#     python_requires='>=3.6',
# )
