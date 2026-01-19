add_library(parser STATIC
    Adt/Adt.cpp
    Adt/AdtChunk.cpp
    Adt/Chunks/Subchunks/MCLQ.cpp
    Adt/Chunks/Subchunks/MCVT.cpp
    Adt/Chunks/MCNK.cpp
    Adt/Chunks/MDDF.cpp
    Adt/Chunks/MH2O.cpp
    Adt/Chunks/MHDR.cpp
    Adt/Chunks/MMDX.cpp
    Adt/Chunks/MODF.cpp
    Adt/Chunks/MWMO.cpp
    Doodad/Doodad.cpp
    Doodad/DoodadInstance.cpp
    Wmo/Wmo.cpp
    Wmo/WmoDoodad.cpp
    Wmo/WmoInstance.cpp
    Wmo/GroupFile/WmoGroupChunk.cpp
    Wmo/GroupFile/WmoGroupFile.cpp
    Wmo/GroupFile/Chunks/MLIQ.cpp
    Wmo/GroupFile/Chunks/MOPY.cpp
    Wmo/GroupFile/Chunks/MOVI.cpp
    Wmo/GroupFile/Chunks/MOVT.cpp
    Wmo/RootFile/Chunks/MODD.cpp
    Wmo/RootFile/Chunks/MODN.cpp
    Wmo/RootFile/Chunks/MODS.cpp
    Map/Map.cpp
    DBC.cpp
    MpqManager.cpp
)
add_library(namigator::parser ALIAS parser)

target_link_libraries(parser 
    PUBLIC
        namigator::utility
        storm
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
)

target_include_directories(parser 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set C++ standard for this target
target_compile_features(parser PUBLIC cxx_std_17)

if (NAMIGATOR_BUILD_C_API)
    install(TARGETS parser 
        EXPORT namigatorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
