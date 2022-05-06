cmake_minimum_required(VERSION 3.6.0)
project(Template)

set(SP2_PATH "../SeriousProton2" CACHE STRING "Path to SeriousProton2 sources")
get_filename_component(SP2_PATH ${SP2_PATH} ABSOLUTE)
set(CMAKE_MODULE_PATH "${SP2_PATH}/cmake" ${CMAKE_MODULE_PATH})
find_package(SeriousProton2 REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
serious_proton2_executable(${PROJECT_NAME} ${SOURCES})
