set(SERIOUS_PROTON2_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

macro(serious_proton2_executable EXECUTABLE_NAME)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
    find_package(SFML 2.3 REQUIRED system window graphics network audio)
    find_package(OpenGL REQUIRED)

    file(GLOB_RECURSE SP2_SOURCES
        "${SERIOUS_PROTON2_BASE_DIR}/src/*.cpp"
        "${SERIOUS_PROTON2_BASE_DIR}/extlibs/lua/*.c"
        "${SERIOUS_PROTON2_BASE_DIR}/extlibs/json11/*.cpp"
        "${SERIOUS_PROTON2_BASE_DIR}/extlibs/GL/*.c"
        "${SERIOUS_PROTON2_BASE_DIR}/extlibs/Box2D/*.cpp"
    )
    add_executable(${EXECUTABLE_NAME} ${ARGN} ${SP2_SOURCES})

    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/include")
    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")

    target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${SFML_INCLUDE_DIR})

    target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})

    target_link_libraries(${EXECUTABLE_NAME} dbghelp psapi)
endmacro()
