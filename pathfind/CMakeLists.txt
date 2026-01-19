set(LIBRARY_NAME libpathfind)
set(PYTHON_NAME pathfind)

set(SRC
    BVH.cpp
    Map.cpp
    TemporaryObstacle.cpp
    Tile.cpp
)
if (NAMIGATOR_BUILD_C_API)
    set(SRC ${SRC} pathfind_c_bindings.cpp)
endif()

add_library(${LIBRARY_NAME} STATIC ${SRC})
add_library(namigator::pathfind ALIAS ${LIBRARY_NAME})

target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${LIBRARY_NAME} 
    PRIVATE
        namigator::utility
        RecastNavigation::Recast
        RecastNavigation::Detour
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
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

if (NAMIGATOR_BUILD_PYTHON)
    pybind11_add_module(${PYTHON_NAME} python.cpp )

    target_link_libraries(${PYTHON_NAME} PRIVATE 
        ${LIBRARY_NAME}
        namigator::utility
        RecastNavigation::Recast
        RecastNavigation::Detour
        storm
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    )

    install(TARGETS ${PYTHON_NAME} DESTINATION namigator)
endif()
