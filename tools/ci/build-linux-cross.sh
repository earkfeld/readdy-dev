#!/usr/bin/env bash

cd "$( dirname "${BASH_SOURCE[0]}" )"/../..
set -ex

# Detect host Python version
build_python_version="$(python3 --version | cut -d' ' -f2)"
python_version="${1:-${build_python_version}}"
python_majmin="$(echo "$python_version" | cut -d'.' -f1,2)"
python_majmin_nodot="${python_majmin//./}"

# Target platform triple (e.g. x86_64-bionic-linux-gnu)
triple="${2:-x86_64-bionic-linux-gnu}"
case "$triple" in
    x86_64-bionic-*) plat_tag=manylinux_2_27_x86_64 ;;
    aarch64-rpi3-*) plat_tag=manylinux_2_27_aarch64 ;;
    armv7-neon-*) plat_tag=manylinux_2_27_armv7l ;;
    armv6-rpi-*) plat_tag=linux_armv6l ;;
    *) echo "Unknown platform ${triple}"; exit 1 ;;
esac

# Package and output directories
pkg_dir="${3:-.}"
out_dir="${4:-dist}"

# Clean build artifacts
rm -rf "$pkg_dir"/build/{generators,CMakeCache.txt}

# Conan profile with Python dev dependencies
python_profile="$PWD/tools/ci/profiles/conan-python.cross.profile"
cat << EOF > "$python_profile"
include($PWD/tools/ci/profiles/$triple.profile)
[conf]
tools.build:skip_test=True
[options]
&:with_conan_python=True
[replace_requires]
tttapa-python-dev/*: tttapa-python-dev/[~$python_majmin]
EOF

# Install dependencies with Conan
conan install "$pkg_dir" --build=missing -pr "$python_profile"

# py-build-cmake cross config file
pbc_config="$PWD/tools/ci/profiles/$triple.py-build-cmake.cross.pbc"
cat << EOF > "$pbc_config"
os = "linux"
implementation = "cp"
version = "$python_majmin_nodot"
abi = "cp$python_majmin_nodot"
arch = "$plat_tag"

[cmake.options]
CMAKE_C_COMPILER_LAUNCHER = "sccache"
CMAKE_CXX_COMPILER_LAUNCHER = "sccache"

[cmake]
build_args = ["--verbose"]
EOF

# Build the wheel
python3 -m build -w "$pkg_dir" -o "$out_dir" -C cross="$pbc_config"
