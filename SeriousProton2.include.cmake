cmake_minimum_required(VERSION 3.6.0)

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
    if(NOT WIN32)
        list(FILTER SP2_SOURCES EXCLUDE REGEX .*/win32/.*)
    endif()
    if(NOT UNIX)
        list(FILTER SP2_SOURCES EXCLUDE REGEX .*/unix/.*)
    endif()

    # Set our optimization flags.
    set(OPTIMIZER_FLAGS "")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # On gcc, we want some general optimalizations that improve speed a lot.
        set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -O3 -flto -funsafe-math-optimizations")

        # If we are compiling for a rasberry pi, we want to aggressively optimize for the CPU we are running on.
        # Note that this check only works if we are compiling directly on the pi, as it is a dirty way of checkif if we are on the pi.
        if(EXISTS /opt/vc/include/bcm_host.h OR COMPILE_FOR_PI)
            set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -mcpu=native -mfpu=neon-vfpv4 -mfloat-abi=hard")
        endif()
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")

    if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
        set(CMAKE_CXX_STANDARD 11)
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    add_executable(${EXECUTABLE_NAME} ${ARGN} ${SP2_SOURCES})

    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/include")
    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")

    target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${SFML_INCLUDE_DIR})

    target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})

    if(WIN32)
        target_link_libraries(${EXECUTABLE_NAME} dbghelp psapi wsock32 iphlpapi)
    endif()
endmacro()
