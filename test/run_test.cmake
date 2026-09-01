# Driver for one golden-file test; see test/CMakeLists.txt for how it is invoked.
# Doing the work in a CMake script instead of a shell script keeps the very same logic on every
# platform - Windows has no shell, and CTest is what CI drives everywhere.
#
# LET     - the let binary
# SRC     - the .let source, relative to the project root; the tests run there because the goldens
#           pin the path exactly as the diagnostics print it
# MODE    - eval | error
# ROOT    - the project root, to resolve SRC and the golden files without depending on the cwd
# OUT_DIR - where the captured stdout/stderr are parked for post-mortem

cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

get_filename_component(base "${SRC}" NAME_WE)
set(stdout_file "${OUT_DIR}/${MODE}-${base}.stdout")
set(stderr_file "${OUT_DIR}/${MODE}-${base}.stderr")

if(MODE STREQUAL eval)
    string(REGEX REPLACE "\\.let$" ".out" golden "${ROOT}/${SRC}")
    set(flags -e)
else()
    string(REGEX REPLACE "\\.let$" ".err" golden "${ROOT}/${SRC}")
    # An optional <name>.flags file holds extra CLI arguments for this one test.
    string(REGEX REPLACE "\\.let$" ".flags" flags_file "${ROOT}/${SRC}")
    set(flags "")
    if(EXISTS "${flags_file}")
        file(READ "${flags_file}" flags)
        separate_arguments(flags NATIVE_COMMAND "${flags}")
    endif()
endif()

execute_process(
    COMMAND "${LET}" "${SRC}" ${flags}
    WORKING_DIRECTORY "${ROOT}"
    OUTPUT_FILE "${stdout_file}"
    ERROR_FILE "${stderr_file}"
    RESULT_VARIABLE rc
)

# Read the raw bytes rather than capturing them: the error tests feed the binary invalid UTF-8,
# which we do not want anybody re-encoding on the way out. On Windows the C runtime opens
# stdout/stderr in text mode, so the binary emits CRLF while the goldens are checked out with LF
# (see .gitattributes) - strip the CRs before comparing or generating.
function(read_stripped file var)
    file(READ "${file}" text)
    string(REPLACE "\r\n" "\n" text "${text}")
    set(${var} "${text}" PARENT_SCOPE)
endfunction()

# Turn a golden file into a list of lines. Escaping the semicolons first keeps a pattern such as
# `2 error(s) encountered; further diagnostics dropped` in one piece.
function(read_lines file var)
    read_stripped("${file}" text)
    string(REPLACE ";" "\;" text "${text}")
    string(REPLACE "\n" ";" text "${text}")
    set(${var} "${text}" PARENT_SCOPE)
endfunction()

function(dump file what)
    read_stripped("${file}" text)
    if(NOT text STREQUAL "")
        message("--- ${what} ---\n${text}")
    endif()
endfunction()

if(MODE STREQUAL eval)
    if(NOT EXISTS "${golden}")
        read_stripped("${stdout_file}" actual)
        file(WRITE "${golden}" "${actual}")
        message("GENERATED: ${golden}")
        return()
    endif()
    if(NOT rc EQUAL 0)
        dump("${stderr_file}" stderr)
        message(FATAL_ERROR "${SRC}: expected success, got exit code ${rc}")
    endif()
    read_stripped("${stdout_file}" actual)
    read_stripped("${golden}" expected)
    if(NOT actual STREQUAL expected)
        # NOTICE rather than part of the FATAL_ERROR: CMake indents and re-wraps error text, which
        # would ruin exactly the byte-for-byte output we are complaining about.
        message(NOTICE "--- expected ---\n${expected}--- actual ---\n${actual}")
        message(FATAL_ERROR "${SRC}: output mismatch")
    endif()
else()
    if(NOT EXISTS "${golden}")
        read_stripped("${stderr_file}" actual)
        file(WRITE "${golden}" "${actual}")
        message("GENERATED: ${golden}")
        return()
    endif()
    if(rc EQUAL 0)
        message(FATAL_ERROR "${SRC}: expected failure but exited 0")
    endif()
    read_stripped("${stderr_file}" actual)
    read_lines("${golden}" patterns)
    foreach(pattern IN LISTS patterns)
        # Blank lines and comments are not patterns.
        if(pattern STREQUAL "" OR pattern MATCHES "^#")
            continue()
        endif()
        string(FIND "${actual}" "${pattern}" pos)
        if(pos EQUAL -1)
            message(NOTICE "--- stderr ---\n${actual}")
            message(FATAL_ERROR "${SRC}: missing pattern: ${pattern}")
        endif()
    endforeach()
endif()
