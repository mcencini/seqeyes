cmake_minimum_required(VERSION 3.20)

project(seqeyes_python)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress deprecated bind1st warning in ExternalSequence
add_compile_definitions(_SILENCE_CXX17_BIND1ST_DEPRECATION_WARNING)

find_package(pybind11 CONFIG REQUIRED)

# Paths relative to the repo root (CMAKE_SOURCE_DIR is python/, so go up one level)
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PULSEQ_DIR "${REPO_ROOT}/src/external/pulseq")

set(PULSEQ_SOURCES
    "${PULSEQ_DIR}/ExternalSequence.cpp"
    "${PULSEQ_DIR}/v151/ExternalSequence.cpp"
    "${PULSEQ_DIR}/v151/md5.cpp"
)

pybind11_add_module(_seqeyes_core
    "${CMAKE_SOURCE_DIR}/src/_seqeyes_core.cpp"
    ${PULSEQ_SOURCES}
)

target_include_directories(_seqeyes_core PRIVATE
    "${PULSEQ_DIR}"
)

install(TARGETS _seqeyes_core DESTINATION seqeyes)
