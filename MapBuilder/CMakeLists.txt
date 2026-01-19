set(EXECUTABLE_NAME MapBuilder)
set(LIBRARY_NAME libmapbuild)
set(PYTHON_NAME mapbuild)

set(SRC BVHConstructor.cpp GameObjectBVHBuilder.cpp MeshBuilder.cpp RecastContext.cpp Worker.cpp FileExist.cpp)
if (NAMIGATOR_BUILD_C_API)
    set(SRC ${SRC} MapBuilder_c_bindings.cpp)
endif()

add_library(${LIBRARY_NAME} ${SRC})
add_library(namigator::mapbuild ALIAS ${LIBRARY_NAME})

target_include_directories(${LIBRARY_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LIBRARY_NAME} 
    PRIVATE
        namigator::parser
        namigator::utility
        RecastNavigation::Recast
        RecastNavigation::Detour
        storm
        Threads::Threads
)

# Set C++ standard for this target
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_17)

if (NAMIGATOR_BUILD_C_API)
    install(TARGETS ${LIBRARY_NAME} 
        EXPORT namigatorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if (NAMIGATOR_BUILD_EXECUTABLES)
    add_executable(${EXECUTABLE_NAME} main.cpp)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LIBRARY_NAME})
    install(TARGETS ${EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (NAMIGATOR_BUILD_PYTHON)
    pybind11_add_module(${PYTHON_NAME} python.cpp)

    target_link_libraries(${PYTHON_NAME} PRIVATE 
        ${LIBRARY_NAME}
        namigator::parser
        namigator::utility
        RecastNavigation::Recast
        RecastNavigation::Detour
        storm
    )

    install(TARGETS ${PYTHON_NAME} DESTINATION namigator)
endif()
