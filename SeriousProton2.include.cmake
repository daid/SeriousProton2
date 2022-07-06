cmake_minimum_required(VERSION 3.8.0)

set(SP2_SINGLE_EXECUTABLE OFF CACHE BOOL "Pack the result as a single executable, which is staticly linked and contains resources files appended as zip format.")
set(SP2_STATIC_LINK OFF CACHE BOOL "Link SDL and mingw DLLs static, so you do not need a copy of these.")

if(NOT SP2_RESOURCE_PATHS)
    # Default resource path, main cmake file can customize this to include one or more paths.
    set(SP2_RESOURCE_PATHS "${CMAKE_SOURCE_DIR}/resources")
endif()

set(SERIOUS_PROTON2_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

if(NOT CPACK_GENERATOR)
    if(WIN32 OR EMSCRIPTEN)
        set(CPACK_GENERATOR ZIP)
    else()
        set(CPACK_GENERATOR TGZ)
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Set our optimization flags.
set(OPTIMIZER_FLAGS "")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    # On gcc, we want some general optimalizations that improve speed a lot.
    set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -O3 -ffast-math -fno-exceptions")

    # If we are compiling for a rasberry pi, we want to aggressively optimize for the CPU we are running on.
    # Note that this check only works if we are compiling directly on the pi, as it is a dirty way of checkif if we are on the pi.
    if(EXISTS /opt/vc/include/bcm_host.h OR COMPILE_FOR_PI)
        set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -mcpu=native -mfpu=neon-vfpv4 -mfloat-abi=hard -DRASBERRY_PI=1 -DNO_ASSERT=1")
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(OPTIMIZER_FLAGS "${OPTIMIZER_FLAGS} -O3 -ffast-math -fno-exceptions")
endif()

set(WARNING_FLAGS -Wall)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(WARNING_FLAGS ${WARNING_FLAGS} -Wno-psabi -Wstrict-aliasing -Wold-style-cast)
    set(WARNING_FLAGS ${WARNING_FLAGS} -Werror=suggest-override -Werror=return-type -Werror=delete-non-virtual-dtor)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
        set(WARNING_FLAGS ${WARNING_FLAGS} -Werror=shadow-compatible-local)
    endif()
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OPTIMIZER_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g1 ${OPTIMIZER_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")

set(CMAKE_CXX_STANDARD 17)

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --always
        OUTPUT_VARIABLE GIT_VERSION ERROR_QUIET
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
endif()
if("${CPACK_PACKAGE_VERSION}" STREQUAL "")
    string(TIMESTAMP CPACK_PACKAGE_VERSION "%Y%m%d.%H%M")
endif()
include(CPack)

find_program(IMAGE_CONVERT convert)
find_program(ICOTOOL icotool)
if("${IMAGE_CONVERT}" STREQUAL "IMAGE_CONVERT-NOTFOUND" AND CMAKE_HOST_WIN32)
    file(GLOB IMAGE_CONVERT_SEARCH_PATHS "C:/Program Files/ImageMagick-*")
    find_program(IMAGE_CONVERT magick PATHS ${IMAGE_CONVERT_SEARCH_PATHS})
endif()

find_package(nlohmann_json 3.2.0 QUIET)
if (NOT nlohmann_json_FOUND)
    include(FetchContent)

    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
      FetchContent_Populate(json)
      add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" ${CMAKE_MODULE_PATH})
if (EMSCRIPTEN)
    set(EMSCRIPTEN_FLAGS "-s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -fno-exceptions -Wno-c++17-extensions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
    foreach(RESOURCE_PATH ${SP2_RESOURCE_PATHS})
        get_filename_component(RESOURCE_NAME "${RESOURCE_PATH}" NAME)
        if (IS_DIRECTORY "${RESOURCE_PATH}")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${RESOURCE_PATH}@${RESOURCE_NAME}")
        endif()
    endforeach()
else()
    find_package(SDL2 REQUIRED)
    if(NOT DEFINED SDL2_LIBRARIES)
        set(SDL2_LIBRARIES SDL2::SDL2)
    endif()

    if (WIN32 AND (SP2_SINGLE_EXECUTABLE OR SP2_STATIC_LINK))
        set(SDL2_LIBRARIES -Wl,-Bstatic ${SDL2_LIBRARIES} -lstdc++ -lpthread -Wl,-Bdynamic -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lsetupapi)
    endif()
endif()

add_subdirectory(${SERIOUS_PROTON2_BASE_DIR}/extlibs ${CMAKE_CURRENT_BINARY_DIR}/extlibs/)

file(GLOB_RECURSE SP2_SOURCES "${SERIOUS_PROTON2_BASE_DIR}/src/*.cpp" "${SERIOUS_PROTON2_BASE_DIR}/include/*.h")
if(NOT WIN32)
    list(FILTER SP2_SOURCES EXCLUDE REGEX .*/win32/.*)
endif()
if(NOT UNIX)
    list(FILTER SP2_SOURCES EXCLUDE REGEX .*/unix/.*)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug") # Build a shared library when debugging, which greatly reduces linking time for incremental builds.
    add_library(seriousproton2 SHARED ${SP2_SOURCES})
else()
    add_library(seriousproton2 OBJECT ${SP2_SOURCES})
endif()
target_compile_options(seriousproton2 PUBLIC ${WARNING_FLAGS})
target_link_libraries(seriousproton2 PUBLIC box2d bullet lua nlohmann_json::nlohmann_json miniz sp2freetype)
target_include_directories(seriousproton2 PUBLIC "${SERIOUS_PROTON2_BASE_DIR}/include" "${SERIOUS_PROTON2_BASE_DIR}/extlibs/bullet")
target_link_libraries(seriousproton2 PUBLIC ${SDL2_LIBRARIES})
target_include_directories(seriousproton2 PUBLIC ${SDL2_INCLUDE_DIRS})
if(WIN32)
    target_link_libraries(seriousproton2 PUBLIC dbghelp psapi ws2_32 iphlpapi ole32 strmiids crypt32)
elseif(UNIX AND NOT EMSCRIPTEN)
    find_package(Threads)
    target_link_libraries(seriousproton2 PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()


macro(serious_proton2_executable EXECUTABLE_NAME)
    if(SP2_ICON)
        if("${IMAGE_CONVERT}" STREQUAL "IMAGE_CONVERT-NOTFOUND")
            message(WARNING "ImageMagick not found, cannot build icons. Which is fine for test builds. But release builds are better with icons.")
        elseif(WIN32)
            if("${ICOTOOL}" STREQUAL "ICOTOOL-NOTFOUND")
                add_custom_command(
                    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/icon.ico"
                    COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 256x256 -define icon:auto-resize="256,128,96,64,48,32,16" "${CMAKE_CURRENT_BINARY_DIR}/icon.ico"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    DEPENDS "${SP2_ICON}")
            else()
                add_custom_command(
                    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/icon.png"
                    COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 64x64 "${CMAKE_CURRENT_BINARY_DIR}/icon.png"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    DEPENDS "${SP2_ICON}")
                add_custom_command(
                    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/icon.ico"
                    COMMAND "${ICOTOOL}" ARGS -c -o "${CMAKE_CURRENT_BINARY_DIR}/icon.ico" "${CMAKE_CURRENT_BINARY_DIR}/icon.png"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/icon.png")
            endif()
            file(GENERATE
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/win32.rc"
                CONTENT "id ICON \"icon.ico\"")
            set_source_files_properties(
                "${CMAKE_CURRENT_BINARY_DIR}/win32.rc"
                PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/icon.ico")
            list(APPEND ARGN "${CMAKE_CURRENT_BINARY_DIR}/win32.rc")
        elseif(ANDROID)
            file(MAKE_DIRECTORY
                "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi"
                "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-hdpi"
                "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xhdpi"
                "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxhdpi"
                "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxxhdpi")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi/ic_launcher.png"
                COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 48x48 "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi/ic_launcher.png"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS "${SP2_ICON}")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-hdpi/ic_launcher.png"
                COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 72x72 "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-hdpi/ic_launcher.png"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS "${SP2_ICON}")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xhdpi/ic_launcher.png"
                COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 96x96 "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xhdpi/ic_launcher.png"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS "${SP2_ICON}")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxhdpi/ic_launcher.png"
                COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 144x144 "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxhdpi/ic_launcher.png"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS "${SP2_ICON}")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxxhdpi/ic_launcher.png"
                COMMAND "${IMAGE_CONVERT}" ARGS "${SP2_ICON}" -resize 192x192 "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxxhdpi/ic_launcher.png"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS "${SP2_ICON}")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi/ic_launcher.png")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-hdpi/ic_launcher.png")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xhdpi/ic_launcher.png")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxhdpi/ic_launcher.png")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-xxxhdpi/ic_launcher.png")
        endif()
    else()
        if(ANDROID)
            message(WARNING "No icon specified, using SDL2 default icon. As android requires an icon.")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi")
            file(COPY "${SDL_SRC_PATH}/android-project/app/src/main/res/mipmap-mdpi/ic_launcher.png"
                DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi/")
            list(APPEND ANDROID_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/android_resources/mipmap-mdpi/ic_launcher.png")
        endif()
    endif()

    set(SP2_TARGET_NAME ${EXECUTABLE_NAME})
    if(ANDROID)
        # For Android, we need a libmain.so, which is packed into an android APK, and loaded from java sources
        set(SP2_TARGET_NAME main)
        add_library(${SP2_TARGET_NAME} SHARED ${ARGN})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -frtti")
        android_apk(${EXECUTABLE_NAME} ${SP2_RESOURCE_PATHS})
    else()
        add_executable(${SP2_TARGET_NAME} ${ARGN})
        if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            target_link_libraries(${SP2_TARGET_NAME} PUBLIC "-mconsole")
        endif()
        if (EMSCRIPTEN)
            configure_file("${SP2_DIR}/cmake/emscripten/template.html" "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.html")
        endif()
    endif()
    target_link_libraries(${SP2_TARGET_NAME} PUBLIC seriousproton2)

    if(WIN32)
        if (SP2_SINGLE_EXECUTABLE)
            set(SINGLE_EXECUTABLE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.single.exe")
            add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.resources.zip"
                COMMAND "rm" ARGS "-rf" "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.resources.zip"
                COMMAND "zip" ARGS "-r" "-D" "-9" "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.resources.zip" .
                WORKING_DIRECTORY "${SP2_RESOURCE_PATHS}"
                DEPENDS "$<TARGET_FILE:${SP2_TARGET_NAME}>"
            )
            add_custom_command(
                OUTPUT "${SINGLE_EXECUTABLE_NAME}"
                COMMAND "cat" ARGS "$<TARGET_FILE:${SP2_TARGET_NAME}>" "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.resources.zip" ">" "${SINGLE_EXECUTABLE_NAME}"
                COMMAND "zip" ARGS "-A" "${SINGLE_EXECUTABLE_NAME}"
                COMMAND "chmod" ARGS "+x" "${SINGLE_EXECUTABLE_NAME}"
                DEPENDS "$<TARGET_FILE:${SP2_TARGET_NAME}>" "${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.resources.zip"
            )
            add_custom_target("${SP2_TARGET_NAME}.single" ALL DEPENDS "${SINGLE_EXECUTABLE_NAME}")
            install(PROGRAMS "${SINGLE_EXECUTABLE_NAME}" DESTINATION .)
        else()
            install(TARGETS ${SP2_TARGET_NAME} RUNTIME DESTINATION .)
            if (NOT SP2_SINGLE_EXECUTABLE)
                execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libwinpthread-1.dll OUTPUT_VARIABLE MINGW_PTHREAD_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
                install(FILES ${MINGW_PTHREAD_DLL} DESTINATION .)
                install(FILES ${SDL2_PREFIX}/bin/SDL2.dll DESTINATION .)
            endif()
            foreach(RESOURCE_PATH ${SP2_RESOURCE_PATHS})
                install(DIRECTORY "${RESOURCE_PATH}" DESTINATION ./)
            endforeach()
        endif()
    elseif(EMSCRIPTEN)
        install(TARGETS ${SP2_TARGET_NAME} RUNTIME DESTINATION .)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.data DESTINATION .)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.wasm DESTINATION .)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.html DESTINATION .)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SP2_TARGET_NAME}.html DESTINATION . RENAME index.html)
    endif()
endmacro()

# Macro to build the android apk
macro(android_apk NAME ASSETS_FOLDER)
    configure_file("${SP2_DIR}/cmake/android/AndroidManifest.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml")
    configure_file("${SP2_DIR}/cmake/android/SP2Activity.java.in" "${CMAKE_CURRENT_BINARY_DIR}/java_source/sp2/${NAME}/SP2Activity.java")
    # Generate the R.java file
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/java_source/sp2/${NAME}/R.java"
        COMMAND "${AAPT}" ARGS package -m -J "${CMAKE_CURRENT_BINARY_DIR}/java_source" -M "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" -S "${CMAKE_CURRENT_BINARY_DIR}/android_resources" -I "${ANDROID_PLATFORM_JAR}"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" ${ANDROID_RESOURCE_FILES}
    )
    # Generate apk with resource files and manifest, but nothing else
    # Compile the java sources (TODO, warning about bootstrap classpath, rt.jar wrong version)
    file(GLOB JAVA_SOURCES "${SDL_SRC_PATH}/android-project/app/src/main/java/org/libsdl/app/*.java")
    list(APPEND JAVA_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/java_source/sp2/${NAME}/R.java")
    list(APPEND JAVA_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/java_source/sp2/${NAME}/SP2Activity.java")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/java_compiled/")
    add_custom_command(
        OUTPUT "java_compiled/sp2/${NAME}/R.class"
        COMMAND ${Java_JAVAC_EXECUTABLE} ARGS -source 1.7 -target 1.7 -classpath "${ANDROID_PLATFORM_JAR}" ${JAVA_SOURCES} -d "${CMAKE_CURRENT_BINARY_DIR}/java_compiled/"
        DEPENDS ${JAVA_SOURCES}
    )
    # Convert sources into dex file
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/classes.dex"
        COMMAND "${DX}" ARGS "--dex" "--output=${CMAKE_CURRENT_BINARY_DIR}/apk_contents/classes.dex" "${CMAKE_CURRENT_BINARY_DIR}/java_compiled/"
        DEPENDS "java_compiled/sp2/${NAME}/R.class"
    )

    file(COPY "${SDL_INSTALL_PATH}/lib/libSDL2.so" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/lib/${ANDROID_ABI}/")
    file(COPY "${SDL_INSTALL_PATH}/lib/libhidapi.so" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/lib/${ANDROID_ABI}/")
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/lib/${ANDROID_ABI}/libmain.so"
        COMMAND "${CMAKE_COMMAND}" ARGS -E copy "$<TARGET_FILE:main>" "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/lib/${ANDROID_ABI}/libmain.so"
        DEPENDS "$<TARGET_FILE:main>"
    )

    set(APK_ALIGNED "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.apk")
    set(APK "${APK_ALIGNED}.unaligned")
    file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/assets/")
    file(COPY "${ASSETS_FOLDER}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/assets/")
    file(GLOB_RECURSE ASSETS LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/" "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/assets/*")
    add_custom_command(
        OUTPUT "${APK}"
        COMMAND "${AAPT}" ARGS package -f -M "${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml" -S "${CMAKE_CURRENT_BINARY_DIR}/android_resources" -I "${ANDROID_PLATFORM_JAR}" -F "${APK}"
        COMMAND "${AAPT}" ARGS add "${APK}" classes.dex
        COMMAND "${AAPT}" ARGS add "${APK}" lib/${ANDROID_ABI}/libmain.so lib/${ANDROID_ABI}/libSDL2.so lib/${ANDROID_ABI}/libhidapi.so
        COMMAND "${AAPT}" ARGS add "${APK}" ${ASSETS}
        COMMAND "${Java_JARSIGNER_EXECUTABLE}" ARGS -verbose -sigalg SHA1withRSA -digestalg SHA1 -storepass "${ANDROID_SIGN_KEY_PASSWORD}" "${APK}" "${ANDROID_SIGN_KEY_NAME}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/classes.dex" "${CMAKE_CURRENT_BINARY_DIR}/apk_contents/lib/${ANDROID_ABI}/libmain.so"
    )
    add_custom_command(
        OUTPUT "${APK_ALIGNED}"
        COMMAND "${CMAKE_COMMAND}" -E remove -f "${APK_ALIGNED}"
        COMMAND "${ZIPALIGN}" ARGS 4 "${APK}" "${APK_ALIGNED}"
        DEPENDS "${APK}"
    )
    add_custom_target(apk ALL DEPENDS "${APK_ALIGNED}")
    add_custom_target(upload
        COMMAND ${ADB} install -r "${APK_ALIGNED}"
        COMMAND ${ADB} shell am start -n "sp2.${NAME}/.SP2Activity"
        DEPENDS "${APK_ALIGNED}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endmacro()
