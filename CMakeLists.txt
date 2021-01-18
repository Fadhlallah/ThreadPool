cmake_minimum_required(VERSION 3.8)

set(libname "libthreadpool")

project("${libname}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")

file(GLOB INCS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

include_directories(include)
enable_testing()


add_library("${libname}" SHARED "${SRCS}" "${INCS}")

add_subdirectory(googletest)

add_subdirectory(test)