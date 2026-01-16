#!/bin/bash
#
# Qt CMake Build Script
#
# Usage:
# ./build.sh <source_dir> <output_dir> <build_type> <qt_install_root> [optional: temp_build_dir]
#
# Example:
# ./build.sh ~/hello_qt_cmake ~/my_bin Release ~/temp_build

# -------------------------------------------------------------------

# Exit immediately if any command fails
set -e

# --- 1. Help and Argument Check ---
if [ "$#" -lt 4 ]; then
    echo "Usage: $0 <source_dir> <output_dir> <build_type (Debug|Release)> <qt_install_root> [optional: temp_build_dir]"
    echo "Example: $0 ~/hello_qt_cmake ~/my_bin Release ~/qt_inst_root"
    exit 1
fi

# --- 2. Set Variables ---
# readlink -f converts relative paths to absolute paths, making the script more robust
SRC_DIR=$(readlink -f "$1")
OUTPUT_DIR=$(readlink -f "$2")
BUILD_TYPE=$3
# Qt install root (REQUIRED)
QT_INSTALL_ROOT=$(readlink -f "$4")

# If the 5th argument (temp build dir) is not specified, use a default
if [ -z "$5" ]; then
    # ${BUILD_TYPE,,} converts "Debug" to "debug"
    TEMP_BUILD_DIR="${SRC_DIR}/build_${BUILD_TYPE,,}" 
    echo "No temp build directory specified, using default: ${TEMP_BUILD_DIR}"
else
    TEMP_BUILD_DIR=$(readlink -f "$5")
fi

# --- 3. Set Your Qt Path (from required argument) ---
# We define the installation root from input, derive prefix path accordingly
QT_PREFIX_PATH="${QT_INSTALL_ROOT}/6.5.3/gcc_64"

# --- 3b. Set CMake Command ---
# Use the CMake bundled with Qt Installer to avoid version conflicts
CMAKE_CMD="${QT_INSTALL_ROOT}/Tools/CMake/bin/cmake"

# --- 4. Check and Create Directories ---
echo "--- Preparing Directories ---"
if [ ! -f "${SRC_DIR}/CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found in source directory: ${SRC_DIR}"
    exit 1
fi

if [ ! -d "${QT_PREFIX_PATH}" ]; then
    echo "Error: Qt prefix path not found: ${QT_PREFIX_PATH}"
    echo "Please edit the QT_PREFIX_PATH variable in this script"
    exit 1
fi

# Also check if the bundled CMake exists
if [ ! -f "${CMAKE_CMD}" ]; then
    echo "Error: Bundled CMake not found at: ${CMAKE_CMD}"
    echo "Did you install 'Developer and Designer Tools' > 'CMake'?"
    echo "Alternatively, edit CMAKE_CMD to point to a valid cmake (version 3.20+)"
    exit 1
fi

# Clean up and create directories
rm -rf "${TEMP_BUILD_DIR}"
mkdir -p "${TEMP_BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

echo "Source Dir:   ${SRC_DIR}"
echo "Output Dir:     ${OUTPUT_DIR}"
echo "Temp Build Dir: ${TEMP_BUILD_DIR}"
echo "Qt Install Root: ${QT_INSTALL_ROOT}"
echo "Qt Path:      ${QT_PREFIX_PATH}"
echo "CMake Cmd:    ${CMAKE_CMD}"

# --- 5. Run CMake Configure ---
echo "--- Configuring CMake (${BUILD_TYPE} mode)... ---"

# -S (Source Dir), -B (Build Dir)
# CMAKE_RUNTIME_OUTPUT_DIRECTORY is the key to a 'clean' output dir
# We now call the specific $CMAKE_CMD
"${CMAKE_CMD}" -S "${SRC_DIR}" \
      -B "${TEMP_BUILD_DIR}" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -DCMAKE_PREFIX_PATH="${QT_PREFIX_PATH}" \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${OUTPUT_DIR}"

# --- 6. Run Make (Build) ---
echo "--- Building... ---"
# -j $(nproc) uses all available CPU cores for parallel compilation
# (Note: 'nproc' might not be available on all systems, replace with -j4, -j8, etc.)
cmake --build "${TEMP_BUILD_DIR}" -- -j $(nproc)

# --- 7. Done ---
echo "--- Build successful! ---"
echo "Final executable is in the (clean) output directory:"
ls -l "${OUTPUT_DIR}"
echo ""
echo "All temporary build files (.o, Makefile) are in: ${TEMP_BUILD_DIR}"