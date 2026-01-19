set(EXECUTABLE_NAME MapViewerSDL)

set(SRC
    main.cpp
    Renderer.cpp
    Camera.cpp
    DetourDebugDraw.cpp
    gl_core_3_3.cpp
)

set(HEADERS
    Renderer.hpp
    Camera.hpp
    DetourDebugDraw.hpp
)

# Find SDL3
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found, MapViewerSDL will not be built")
    message(STATUS "Install SDL3 or set SDL3_DIR to enable MapViewerSDL")
    return()
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find or fetch Dear ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

# Find or fetch GLM
find_package(glm QUIET)
if(NOT glm_FOUND)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)
endif()

# Simple approach: We'll use SDL_GL functions for OpenGL loading
# No need for GLAD - SDL3 provides GL function loading

# Create ImGui library
add_library(imgui_impl STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_impl PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_impl PUBLIC SDL3::SDL3 OpenGL::GL)

# Create executable
add_executable(${EXECUTABLE_NAME} ${SRC} ${HEADERS})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    namigator::utility
    namigator::parser
    namigator::pathfind
    storm
    RecastNavigation::Detour
    RecastNavigation::DebugUtils
    SDL3::SDL3
    OpenGL::GL
    imgui_impl
    glm::glm
)

# Set C++ standard
target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)

# Platform-specific settings
if(WIN32)
    # On Windows, use main() instead of WinMain
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE SDL_MAIN_HANDLED)
endif()

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
