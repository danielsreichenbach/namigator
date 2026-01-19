set(EXECUTABLE_NAME MapViewer)

set(SRC
    Camera.cpp
    CommonControl.cpp
    DetourDebugDraw.cpp
    main.cpp
    Renderer.cpp
)

# Modern shader compilation using custom commands
set(PIXEL_SHADER_HEADER ${CMAKE_CURRENT_BINARY_DIR}/pixelShader.hpp)
set(VERTEX_SHADER_HEADER ${CMAKE_CURRENT_BINARY_DIR}/VertexShader.hpp)

# Find fxc.exe (DirectX shader compiler)
find_program(FXC fxc.exe
    HINTS "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
          "$ENV{DXSDK_DIR}/Utilities/bin/x64"
    DOC "Path to fxc.exe (DirectX shader compiler)"
)

if(FXC)
    # Compile pixel shader
    add_custom_command(
        OUTPUT ${PIXEL_SHADER_HEADER}
        COMMAND ${FXC} /T ps_4_0 /E PShader /Fh ${PIXEL_SHADER_HEADER} ${CMAKE_CURRENT_SOURCE_DIR}/pixelShader.hlsl
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/pixelShader.hlsl
        COMMENT "Compiling pixel shader to header"
        VERBATIM
    )
    
    # Compile vertex shader
    add_custom_command(
        OUTPUT ${VERTEX_SHADER_HEADER}
        COMMAND ${FXC} /T vs_4_0 /E VShader /Fh ${VERTEX_SHADER_HEADER} ${CMAKE_CURRENT_SOURCE_DIR}/VertexShader.hlsl
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/VertexShader.hlsl
        COMMENT "Compiling vertex shader to header"
        VERBATIM
    )
    
    set(SHADER_HEADERS ${PIXEL_SHADER_HEADER} ${VERTEX_SHADER_HEADER})
else()
    # Fallback to VS properties if fxc.exe not found
    set(SHADERS
        pixelShader.hlsl
        VertexShader.hlsl
    )
    
    set_source_files_properties(pixelShader.hlsl PROPERTIES
        VS_SHADER_TYPE Pixel
        VS_SHADER_MODEL 4.0
        VS_SHADER_ENTRYPOINT PShader
        VS_SHADER_FLAGS "/Fh \"${CMAKE_CURRENT_SOURCE_DIR}/pixelShader.hpp\""
    )
    
    set_source_files_properties(VertexShader.hlsl PROPERTIES
        VS_SHADER_TYPE Vertex
        VS_SHADER_MODEL 4.0
        VS_SHADER_ENTRYPOINT VShader
        VS_SHADER_FLAGS "/Fh \"${CMAKE_CURRENT_SOURCE_DIR}/VertexShader.hpp\""
    )
endif()

add_executable(${EXECUTABLE_NAME} WIN32 ${SRC} ${SHADER_HEADERS} ${SHADERS})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
    namigator::utility
    namigator::parser
    namigator::pathfind
    storm
    RecastNavigation::Detour
    RecastNavigation::DebugUtils
)

# Add include directory for generated shader headers
if(FXC)
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Set C++ standard for this target
target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})