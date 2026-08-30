# Golden-file tests: one CTest test per .let file, all run by test/run_test.cmake.
#
#   test/*.let       + test/<name>.out       run with -e; must succeed and match stdout exactly
#   test/error/*.let + test/error/<name>.err must fail; every non-empty, non-# line of the golden
#                                            must occur verbatim in stderr
#
# A missing golden file is *generated* from the current binary output and the test passes - that is
# how a new test is added: write the .let file, run ctest, review the generated golden.
#
# CONFIGURE_DEPENDS so that a new .let file re-runs CMake instead of silently going untested.
file(GLOB eval_lets  CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.let")
file(GLOB error_lets CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "error/*.let")

function(let_add_test mode let)
    string(REGEX REPLACE "\\.let$" "" name "${let}")
    add_test(
        NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            -D "LET=$<TARGET_FILE:let>"
            -D "SRC=test/${let}"
            -D "MODE=${mode}"
            -D "ROOT=${PROJECT_SOURCE_DIR}"
            -D "OUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/run_test.cmake"
    )
    set_tests_properties(${name} PROPERTIES LABELS ${mode})
endfunction()

foreach(let IN LISTS eval_lets)
    let_add_test(eval "${let}")
endforeach()
foreach(let IN LISTS error_lets)
    let_add_test(error "${let}")
endforeach()
