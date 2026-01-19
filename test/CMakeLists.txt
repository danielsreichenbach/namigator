# Explicitly list test scripts
set(PY_SCRIPTS
    smoke_tests.py
    manual_tests.py
)

foreach(script ${PY_SCRIPTS})
    install(PROGRAMS ${script} DESTINATION ${CMAKE_INSTALL_DATADIR}/namigator/test)
endforeach()

install(FILES test_map.mpq DESTINATION ${CMAKE_INSTALL_DATADIR}/namigator/test)
