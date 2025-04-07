#!/usr/bin/env bash

# Move to project root
cd "$( dirname "${BASH_SOURCE[0]}" )"/../..
set -ex

# Package and output directories
pkg_dir="${1:-.}"
out_dir="${2:-dist}"
install_stubs_dir="$3"

# Clean previous build cache
rm -rf "$pkg_dir"/build/{generators,CMakeCache.txt}

# Generate a temporary Conan profile
python_profile="$PWD/tools/ci/profiles/native-conan-python.local.profile"
cat << EOF > "$python_profile"
include(default)
[settings]
os=Linux
build_type=Release
[conf]
tools.build:skip_test=True
tools.cmake.cmaketoolchain:generator=Ninja Multi-Config
EOF

# Install dependencies using Conan
conan install "$pkg_dir" --build=missing -pr "$python_profile"

# Create local py-build-cmake config
pbc_config="$PWD/tools/ci/profiles/native-py-build-cmake.local.pbc"
cat << EOF > "$pbc_config"
[cmake.options]
CMAKE_C_COMPILER_LAUNCHER="sccache"
CMAKE_CXX_COMPILER_LAUNCHER="sccache"

[cmake]
build_args = ["--verbose"]
EOF

# Build the Python wheel
python3 -m build -w "$pkg_dir" -o "$out_dir" -C local="$pbc_config"

# Optional: Install Python stubs to a provided directory
if [ -n "$install_stubs_dir" ]; then
    cd "$pkg_dir"

    py-build-cmake --local="$pbc_config" configure

    py-build-cmake --local="$pbc_config" \
        install --component python_modules -- --prefix "$install_stubs_dir"

    py-build-cmake --local="$pbc_config" \
        install --component python_stubs -- --prefix "$install_stubs_dir"

    # Clean out any compiled binary modules
    while IFS= read -r f || [ -n "$f" ]; do rm -f "$f"; done < build/install_manifest_python_modules.txt
fi
