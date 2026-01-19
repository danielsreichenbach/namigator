add_library(utility STATIC
    AABBTree.cpp
    BinaryStream.cpp
    BoundingBox.cpp
    Matrix.cpp
    Vector.cpp
    Quaternion.cpp
    MathHelper.cpp
    Ray.cpp
    String.cpp
)
add_library(namigator::utility ALIAS utility)

target_include_directories(utility 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

# Set C++ standard for this target
target_compile_features(utility PUBLIC cxx_std_17)

if (NAMIGATOR_BUILD_C_API)
    install(TARGETS utility 
        EXPORT namigatorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
