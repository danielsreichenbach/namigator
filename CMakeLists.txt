cmake_minimum_required(VERSION 3.22)

project(namigator
    VERSION 1.0.0
    DESCRIPTION "High-performance pathfinding system for World of Warcraft environments"
    LANGUAGES CXX
)

# namigator depends on a compiler which supports C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# generate position independent code for everything
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Default to release build type if none is specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Default build type")
endif()

# threading library is required
find_package(Threads REQUIRED)

# this is used a lot for serialization
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-multichar)
endif()

option(NAMIGATOR_BUILD_PYTHON "Build Python bindings if Python2/3 is present." TRUE)
option(NAMIGATOR_INSTALL_TESTS "Install tests." TRUE)
option(NAMIGATOR_BUILD_C_API "Build the C API." TRUE)
option(NAMIGATOR_BUILD_EXECUTABLES "Build the MapViewer executable. Windows only." TRUE)

if(NAMIGATOR_BUILD_PYTHON)
    # Modern Python finding
    find_package(Python COMPONENTS Interpreter Development)
    if(Python_FOUND)
        add_subdirectory(pybind11)
    else()
        message(WARNING "Python not found. Python bindings for ${CMAKE_PROJECT_NAME} will not be compiled")
        set(NAMIGATOR_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
    endif()
endif()

# Modern filesystem library detection will be handled per-target with generator expressions

# we want this definition present globally
add_compile_definitions(DT_POLYREF64)

# third party dependencies
set(RECASTNAVIGATION_DEMO OFF CACHE BOOL "Build demo")
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL "Build tests")
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL "Build examples")
add_subdirectory(recastnavigation EXCLUDE_FROM_ALL)

if (NAMIGATOR_BUILD_C_API)
    install(TARGETS Detour Recast ARCHIVE DESTINATION lib)
endif()

# This is just easier than copying over the DLLs
if (MSVC)
    set(STORM_USE_BUNDLED_LIBRARIES ON CACHE BOOL "")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

add_subdirectory(stormlib EXCLUDE_FROM_ALL)
if (NAMIGATOR_BUILD_C_API)
    install(TARGETS storm ARCHIVE DESTINATION lib)
endif()

# namigator libraries
add_subdirectory(utility)

add_subdirectory(parser)
add_subdirectory(pathfind)

# namigator executables
add_subdirectory(MapBuilder)

if(NAMIGATOR_INSTALL_TESTS)
    add_subdirectory(test)
endif()

if (WIN32 AND NAMIGATOR_BUILD_EXECUTABLES)
    add_subdirectory(MapViewer)
endif()

# Installation and packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Create package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/namigatorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Configure package config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/namigatorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/namigatorConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/namigator
)

# Install the config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/namigatorConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/namigatorConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/namigator
)

# Export targets
install(EXPORT namigatorTargets
    FILE namigatorTargets.cmake
    NAMESPACE namigator::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/namigator
)

# Feature summary
include(FeatureSummary)
feature_summary(WHAT ALL)
