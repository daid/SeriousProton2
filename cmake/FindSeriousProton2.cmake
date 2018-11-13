## Find the SeriousProton2.include.cmake file and include it
# This adds the serious_proton2_executable function which can be used to build a SeriousProton2 based application with very little CMake.
# Usage: serious_proton2_executable(EXECUTABLE_NAME SOURCES...)




# All the locations to look for SeriousProton2
set(FIND_SP2_PATHS
    ${SP2_ROOT}
    $ENV{SP2_ROOT}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/SeriousProton2/..
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt)

find_path(SP2_DIR SeriousProton2.include.cmake
          PATHS ${FIND_SP2_PATHS})
if(NOT SP2_DIR)
    message(FATAL_ERROR "Failed to find SeriousProton2, set SP2_ROOT to root directory of SeriousProton2")
endif()

include("${SP2_DIR}/SeriousProton2.include.cmake")
