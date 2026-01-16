# Generates version_autogen.h at build time from Git metadata.
# Inputs:
#   - OUT: path to write header to (e.g. <build>/generated/version_autogen.h)
#   - SRC_DIR: repository root for git commands

if(NOT DEFINED OUT)
  message(FATAL_ERROR "GenVersion.cmake: OUT not specified")
endif()
if(NOT DEFINED SRC_DIR)
  message(FATAL_ERROR "GenVersion.cmake: SRC_DIR not specified")
endif()

# Default values
set(HASH "unknown")
set(DATE "unknown")

# Find git
find_package(Git QUIET)
if(Git_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} -C ${SRC_DIR} rev-parse --short=10 HEAD
    OUTPUT_VARIABLE HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} -C ${SRC_DIR} log -1 --format=%cd --date=format:%Y%m%d
    OUTPUT_VARIABLE DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
endif()

file(WRITE ${OUT} "#pragma once\n")
file(APPEND ${OUT} "\n")
file(APPEND ${OUT} "// Auto-generated at build time.\n")
file(APPEND ${OUT} "// SEQEYE_GIT_DATE is the commit date (YYYYMMDD) for SEQEYE_GIT_HASH.\n")
file(APPEND ${OUT} "#define SEQEYE_GIT_HASH \"${HASH}\"\n")
file(APPEND ${OUT} "#define SEQEYE_GIT_DATE \"${DATE}\"\n")

