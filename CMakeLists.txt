cmake_minimum_required(VERSION 3.29 FATAL_ERROR)
project(Let VERSION 0.14.0)

include(GNUInstallDirs)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
if(MSVC)
    add_compile_options(/utf-8 /wd4146 /wd4245)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}; shared libs: ${BUILD_SHARED_LIBS}")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Make `make test`/`ninja test` build first instead of testing a stale binary.
    set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
    include(CTest) # defines BUILD_TESTING
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX) # prevents windows.h defining min/max macros
else()
    add_compile_options(-Wall -Wextra)
endif()

add_subdirectory(submodules/fe)

add_executable(let)
target_sources(let
    PRIVATE
        src/main.cpp
        src/let/driver.cpp
        src/let/eval.cpp
        src/let/lexer.cpp
        src/let/parser.cpp
        src/let/stream.cpp
        src/let/tok.cpp
        include/let/ast.h
        include/let/driver.h
        include/let/lexer.h
        include/let/parser.h
        include/let/tok.h
)
target_link_libraries(let PRIVATE fe)
# What `--version` reports; both derive from the project(... VERSION) calls that scripts/release.sh bumps.
# fe's project() only sets fe_VERSION within its own directory scope, so fetch it from there.
get_directory_property(fe_version DIRECTORY submodules/fe DEFINITION fe_VERSION)
target_compile_definitions(let PRIVATE LET_VERSION="${Let_VERSION}" FE_VERSION="${fe_version}")
target_include_directories(let
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(let PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

install(TARGETS let RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/let")

if(BUILD_TESTING AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(test)
endif()
