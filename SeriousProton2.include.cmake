cmake_minimum_required(VERSION 3.6.0)

set(SERIOUS_PROTON2_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

macro(serious_proton2_executable EXECUTABLE_NAME)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
    find_package(SFML 2.3 REQUIRED system window graphics network audio)
    find_package(OpenGL REQUIRED)
    find_package(ZLIB REQUIRED)

    file(GLOB_RECURSE SP2_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/src/*.cpp")
    file(GLOB_RECURSE LUA_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/lua/*.c")
    file(GLOB_RECURSE JSON11_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/json11/*.cpp")
    file(GLOB_RECURSE GLEW_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/GL/*.c")
    file(GLOB_RECURSE BOX2D_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/Box2D/*.cpp")
    file(GLOB_RECURSE BULLET_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet/*.cpp")
    file(GLOB_RECURSE FREETYPE_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/freetype-2.9/src/*.c")

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

    set(CMAKE_CXX_STANDARD 11)

    add_library(box2d STATIC ${BOX2D_SOURCES})
    target_include_directories(box2d PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(bullet STATIC ${BULLET_SOURCES})
    target_include_directories(bullet PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet")
    add_library(lua STATIC ${LUA_SOURCES})
    target_include_directories(lua PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(glew STATIC ${GLEW_SOURCES})
    target_include_directories(glew PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    target_link_libraries(glew ${OPENGL_LIBRARIES})
    target_include_directories(glew PUBLIC ${OPENGL_INCLUDE_DIR})
    add_library(json11 STATIC ${JSON11_SOURCES})
    target_include_directories(json11 PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(sp2freetype STATIC ${FREETYPE_SOURCES})
    target_include_directories(sp2freetype PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs/freetype-2.9/include")
    target_compile_definitions(sp2freetype PRIVATE "-DFT2_BUILD_LIBRARY")

    add_executable(${EXECUTABLE_NAME} ${ARGN} ${SP2_SOURCES})
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC box2d bullet lua glew json11 sp2freetype)
    target_include_directories(${EXECUTABLE_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/include" "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet")
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${SFML_LIBRARIES} ${ZLIB_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME} PUBLIC ${SFML_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
    if(WIN32)
        target_link_libraries(${EXECUTABLE_NAME} PUBLIC dbghelp psapi ws2_32 iphlpapi)
    endif()
    if(UNIX)
        find_package(Threads)
        target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
    endif()
endmacro()
