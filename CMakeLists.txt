cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(Let)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

if(MSVC)
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks non-stable c++20 features
    add_compile_options(/utf-8 /wd4146 /wd4245)
else()
    set(CMAKE_CXX_STANDARD 20) # -std=c++20 for everyone else
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}; shared libs: ${BUILD_SHARED_LIBS}")

if(WIN32)
    add_compile_definitions(NOMINMAX) # prevents windows.h defining min/max macros
else()
    add_compile_options(-Wall -Wextra)
endif()

set(FE_ABSL OFF)
if(FE_ABSL)
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL ON)
    find_package(abseil)
endif()

add_subdirectory(external/fe)

add_executable(let)
target_sources(let
    PRIVATE
        src/main.cpp
        src/let/eval.cpp
        src/let/lexer.cpp
        src/let/parser.cpp
        src/let/stream.cpp
        src/let/tok.cpp
        include/let/ast.h
        include/let/driver.h
        include/let/lexer.h
        include/let/tok.h
)
target_link_libraries(let PRIVATE fe)
target_include_directories(let
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(let PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
if (MSVC AND BUILD_SHARED_LIBS AND FE_ABSL)
    target_compile_definitions(liblet PUBLIC ABSL_CONSUME_DLL)
endif()
