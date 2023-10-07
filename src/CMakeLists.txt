add_executable(let main.cpp)

target_sources(let PRIVATE main.cpp)
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

add_subdirectory(let)
