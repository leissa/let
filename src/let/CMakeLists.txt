add_library(liblet
    lexer.cpp
    parser.cpp
    print.cpp
    stream.cpp
    tok.cpp
)
set_target_properties(liblet PROPERTIES PREFIX "")

target_include_directories(liblet
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(liblet
    PUBLIC
        absl::flat_hash_map absl::flat_hash_set
        absl::node_hash_map absl::node_hash_set
        fe
)
target_compile_features(liblet PUBLIC cxx_std_${CMAKE_CXX_STANDARD}) # make property public.

if (MSVC AND BUILD_SHARED_LIBS)
    target_compile_definitions(liblet PUBLIC ABSL_CONSUME_DLL)
endif()
