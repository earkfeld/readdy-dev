import os
import re
import platform
from pathlib import Path
from subprocess import run

# Set working directory to project root
os.chdir(Path(__file__).resolve().parents[2])

# Detect target architectures from ARCHFLAGS
archflags = os.getenv("ARCHFLAGS")
archs = tuple(sorted(re.findall(r"-arch +(\S+)", archflags))) if archflags else ()

print("ARCHFLAGS:", archs)
print("PLATFORM:", platform.machine())
print("MACOSX_DEPLOYMENT_TARGET:", os.getenv("MACOSX_DEPLOYMENT_TARGET"))
print(flush=True)

if not archs:
    archs = (platform.machine(),)

# Determine deployment target
deployment_target = os.getenv("MACOSX_DEPLOYMENT_TARGET", "11.0")
can_run = platform.machine() in archs

# Architecture settings
conan_arch = {
    ("x86_64",): "x86_64",
    ("arm64",): "armv8",
    ("arm64", "x86_64"): "armv8|x86_64",
}[archs]

conan_arch_custom = {
    ("x86_64",): "x86-64-v2",
    ("arm64",): "apple-m1",
    ("arm64", "x86_64"): "x86-64-v2-apple-m1",
}[archs]

cpu_flags = {
    ("x86_64",): ["-march=x86-64-v2"],
    ("arm64",): ["-mcpu=apple-m1"],
    ("arm64", "x86_64"): [
        "-Xarch_arm64", "-mcpu=apple-m1",
        "-Xarch_x86_64", "-march=x86-64-v2",
    ],
}[archs]

# CMake architecture flags
cmake_opts = {
    "CMAKE_OSX_ARCHITECTURES": ";".join(archs),
    "CMAKE_Fortran_FLAGS_INIT": " ".join(cpu_flags),
}

module_linker_flags = {
    f"CMAKE_MODULE_LINKER_FLAGS{c}_INIT": f"${{CMAKE_SHARED_LINKER_FLAGS{c}_INIT}}"
    for c in ("", "_DEBUG", "_RELEASE", "_RELWITHDEBINFO")
}

# Build the Conan profile string
native_profile = f"""\
include(default)
[settings]
arch={conan_arch}
os.version={deployment_target}
build_type=Release
[conf]
tools.build:skip_test=True
tools.cmake.cmaketoolchain:generator=Ninja Multi-Config
tools.build:cflags+={cpu_flags}
tools.build:cxxflags+={cpu_flags}
tools.cmake.cmaketoolchain:extra_variables*={repr(cmake_opts)}
tools.cmake.cmaketoolchain:extra_variables*={repr(module_linker_flags)}
"""

cross = not can_run
profile = native_profile
if cross:
    profile += f"""\
tools.build.cross_building:can_run={can_run}
tools.cmake.cmaketoolchain:system_name=\"Darwin\"
"""

# Write Conan profile to disk
Path("cibw.profile").write_text(profile)

# Run Conan install with generated profile
run("conan install . -pr:h ./cibw.profile --build=missing", shell=True, check=True)