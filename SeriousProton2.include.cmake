cmake_minimum_required(VERSION 3.6.0)

set(SERIOUS_PROTON2_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

macro(serious_proton2_executable EXECUTABLE_NAME)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
    find_package(SDL2 REQUIRED)
    find_package(ZLIB REQUIRED)
    if(NOT DEFINED SDL2_LIBRARIES)
        set(SDL2_LIBRARIES SDL2::SDL2)
    endif()

    file(GLOB_RECURSE SP2_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/src/*.cpp")
    file(GLOB_RECURSE LUA_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/lua/*.c")
    file(GLOB_RECURSE JSON11_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/json11/*.cpp")
    file(GLOB_RECURSE BOX2D_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/Box2D/*.cpp")
    file(GLOB_RECURSE BULLET_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet/*.cpp")
    file(GLOB_RECURSE FREETYPE_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/extlibs/freetype-2.9/src/*.c")

    if(NOT WIN32)
        list(FILTER SP2_SOURCES EXCLUDE REGEX .*/win32/.*)
    endif()
    if(NOT UNIX)
        list(FILTER SP2_SOURCES EXCLUDE REGEX .*/unix/.*)
    endif()
    
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif()

    # Set our optimization flags.
    set(OPTIMIZER_FLAGS "")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        # On gcc, we want some general optimalizations that improve speed a lot.
        set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -O3 -funsafe-math-optimizations")

        # If we are compiling for a rasberry pi, we want to aggressively optimize for the CPU we are running on.
        # Note that this check only works if we are compiling directly on the pi, as it is a dirty way of checkif if we are on the pi.
        if(EXISTS /opt/vc/include/bcm_host.h OR COMPILE_FOR_PI)
            set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -mcpu=native -mfpu=neon-vfpv4 -mfloat-abi=hard -DRASBERRY_PI=1 -DNO_ASSERT=1")
        endif()
    endif()

    set(WARNING_FLAGS "-Wall")

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-psabi -Wno-strict-aliasing")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG=1")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")

    set(CMAKE_CXX_STANDARD 11)

    add_library(box2d STATIC ${BOX2D_SOURCES})
    target_include_directories(box2d PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(bullet STATIC ${BULLET_SOURCES})
    target_include_directories(bullet PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet")
    add_library(lua STATIC ${LUA_SOURCES})
    target_include_directories(lua PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(json11 STATIC ${JSON11_SOURCES})
    target_include_directories(json11 PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs")
    add_library(sp2freetype STATIC ${FREETYPE_SOURCES})
    target_include_directories(sp2freetype PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/extlibs/freetype-2.9/include")
    target_compile_definitions(sp2freetype PRIVATE "-DFT2_BUILD_LIBRARY")

    set(SP2_TARGET_NAME ${EXECUTABLE_NAME})
    if(ANDROID)
        # For Android, we need a libmain.so, which is packed into an android APK, and loaded from java sources
        set(SP2_TARGET_NAME main)
        add_library(${SP2_TARGET_NAME} SHARED ${ARGN} ${SP2_SOURCES})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti")
        android_apk(${EXECUTABLE_NAME} resources)
    else()
        add_executable(${SP2_TARGET_NAME} ${ARGN} ${SP2_SOURCES})
    endif()
    target_link_libraries(${SP2_TARGET_NAME} PUBLIC box2d bullet lua json11 sp2freetype)
    target_include_directories(${SP2_TARGET_NAME} PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/include" "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet")
    target_link_libraries(${SP2_TARGET_NAME} PUBLIC ${SDL2_LIBRARIES} ${ZLIB_LIBRARIES})
    target_include_directories(${SP2_TARGET_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
    if(WIN32)
        target_link_libraries(${SP2_TARGET_NAME} PUBLIC dbghelp psapi ws2_32 iphlpapi ole32 strmiids crypt32)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_link_libraries(${SP2_TARGET_NAME} PUBLIC "-mconsole")
        endif()
    endif()
    if(UNIX)
        find_package(Threads)
        target_link_libraries(${SP2_TARGET_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
    endif()

    if(WIN32)
        install(TARGETS ${SP2_TARGET_NAME} RUNTIME DESTINATION .)

        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libwinpthread-1.dll OUTPUT_VARIABLE MINGW_PTHREAD_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
        install(FILES ${MINGW_STDCPP_DLL} ${MINGW_LIBGCC_DLL} ${MINGW_PTHREAD_DLL} DESTINATION .)
        install(FILES ${SDL2_PREFIX}/bin/SDL2.dll DESTINATION .)
        get_filename_component(ZLIB_PATH "${ZLIB_LIBRARY}" DIRECTORY)
        install(FILES ${ZLIB_PATH}/../bin/zlib1.dll DESTINATION .)
    endif()
endmacro()

if(WIN32)
    set(CPACK_GENERATOR NSIS ZIP)
    find_package(Git)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --always OUTPUT_VARIABLE GIT_VERSION WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
    endif()
    include(CPack)
endif()
