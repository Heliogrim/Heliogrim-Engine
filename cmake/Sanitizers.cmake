include_guard(GLOBAL)

# Translate options

include(CheckCXXSourceCompiles)

set(SANITIZERS "" CACHE STRING "List of sanitizers to apply. (Address,Memory,MemoryWithOrigins,Undefined,Thread,Leak,CFI)")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set_property(CACHE SANITIZERS PROPERTY STRINGS Address Undefined)
else ()
    set_property(CACHE SANITIZERS PROPERTY STRINGS Address Memory MemoryWithOrigins Undefined Thread Leak CFI)
endif ()

# CMake Auto-Test Flags
function(_sanitizer_auto_test_flags out flags)
    # Backup quite flag and required flags
    set(TMP_RESTORE_QUIET ${CMAKE_REQUIRED_QUIET})
    set(TMP_RESTORE_FLAGS ${CMAKE_REQUIRED_FLAGS})

    # Store flags and test
    unset(${out} CACHE)
    set(CMAKE_REQUIRED_QUIET FALSE)
    set(CMAKE_REQUIRED_FLAGS "${flags}")

    check_cxx_source_compiles("int main() { return 0; }" ${out})

    # Restore flags
    set(CMAKE_REQUIRED_FLAGS "${TMP_RESTORE_FLAGS}")
    set(CMAKE_REQUIRED_QUIET "${TMP_RESTORE_QUIET}")
endfunction()

# Generate Instructions
function(_sanitizer_init)

    set(local_shared_availability TRUE)
    set(local_shared_flag_list "")
    set(local_shared_def_list "")
    set(local_shared_run_dlls "")
    set(local_shared_link_libs "")

    set(local_flag_prefix "-fsanitize")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
        set(local_flag_prefix "/fsanitize")
    endif ()

    # Check availability
    if (SANITIZERS MATCHES "([Aa]ddress)")
        message(CHECK_START "Checking availability of the address sanitizer.")
        set(local_test_flags "${local_flag_prefix}=address")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_ADDRESS_AVAILABLE ${local_test_flags})

        set(SANITIZER_ADDRESS_AVAILABLE TRUE CACHE INTERNAL "" FORCE)
        if (MSVC)
            list(APPEND local_shared_def_list _DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)

            if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
                if (CMAKE_SIZEOF_VOID_P EQUAL 8)
                    string(FIND "${CMAKE_CXX_COMPILER_VERSION}" "." delim)
                    string(SUBSTRING "${CMAKE_CXX_COMPILER_VERSION}" 0 ${delim} major)
                    list(APPEND local_shared_run_dlls
                            "$ENV{VCToolsInstallDir}/../../Llvm/x64/lib/clang/${major}/lib/windows/clang_rt.asan_dynamic-x86_64.dll"
                    )
                    list(APPEND local_shared_link_libs
                            "$ENV{VCToolsInstallDir}/../../Llvm/x64/lib/clang/${major}/lib/windows/clang_rt.asan_dynamic-x86_64.lib"
                            "$ENV{VCToolsInstallDir}/../../Llvm/x64/lib/clang/${major}/lib/windows/clang_rt.asan_dynamic_runtime_thunk-x86_64.lib"
                    )
                endif ()
            else ()
                if (CMAKE_SIZEOF_VOID_P EQUAL 8)
                    list(APPEND local_shared_run_dlls "$ENV{VCToolsInstallDir}/bin/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll")
                else ()
                    list(APPEND local_shared_run_dlls "$ENV{VCToolsInstallDir}/bin/Hostx86/x86/clang_rt.asan_dynamic-i386.dll")
                endif ()
            endif ()
        endif ()

        if (SANITIZER_ADDRESS_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    if (SANITIZERS MATCHES "([Mm]emory[Ww]ith[Oo]rigins)")
        message(CHECK_START "Checking availability of the memory sanitizer with origin tracking.")
        set(local_test_flags "-fsanitize-memory-track-origins ${local_flag_prefix}=memory")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_MEMORY_WITH_ORIGIN_AVAILABLE ${local_test_flags})

        if (SANITIZER_MEMORY_WITH_ORIGIN_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()

    elseif (SANITIZERS MATCHES "([Mm]emory)")
        message(CHECK_START "Checking availability of the memory sanitizer.")
        set(local_test_flags "${local_flag_prefix}=memory")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_MEMORY_AVAILABLE ${local_test_flags})

        if (SANITIZER_MEMORY_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    if (SANITIZERS MATCHES "([Uu]ndefined)")
        message(CHECK_START "Checking availability of the undefined behaviour sanitizer.")
        set(local_test_flags "${local_flag_prefix}=undefined")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_UNDEFINED_AVAILABLE ${local_test_flags})

        if (SANITIZER_UNDEFINED_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    if (SANITIZERS MATCHES "([Tt]hread)")
        message(CHECK_START "Checking availability of the thread sanitizer.")
        set(local_test_flags "${local_flag_prefix}=thread")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_THREAD_AVAILABLE ${local_test_flags})

        if (SANITIZER_THREAD_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    if (SANITIZERS MATCHES "([Ll]eak)")
        message(CHECK_START "Checking availability of the leak sanitizer.")
        set(local_test_flags "${local_flag_prefix}=leak")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_LEAK_AVAILABLE ${local_test_flags})

        if (SANITIZER_LEAK_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    if (SANITIZERS MATCHES "([Cc][Ff][Ii])")
        message(CHECK_START "Checking availability of the cfi sanitizer.")
        set(local_test_flags "${local_flag_prefix}=cfi")
        list(APPEND local_shared_flag_list ${local_test_flags})
        _sanitizer_auto_test_flags(SANITIZER_CFI_AVAILABLE ${local_test_flags})

        if (SANITIZER_CFI_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    # Check composed flags
    list(LENGTH SANITIZERS requested_length)
    if (requested_length GREATER_EQUAL 2)
        message(CHECK_START "Checking availability composed sanitizer flags: ${SANITIZERS}")
        list(JOIN local_shared_flag_list " " local_test_flags)
        _sanitizer_auto_test_flags(SANITIZER_COMPOSED_AVAILABLE ${local_test_flags})

        set(SANITIZER_COMPOSED_AVAILABLE TRUE CACHE INTERNAL "" FORCE)
        if (SANITIZER_COMPOSED_AVAILABLE)
            message(CHECK_PASS "PASSED")
        else ()
            message(CHECK_FAIL "FAILED")
            set(local_shared_availability FALSE)
        endif ()
    endif ()

    # Report shared sanitizer state
    if (NOT local_shared_availability)
        set(SANITIZER_COMPILE_OPTIONS "" CACHE INTERNAL "" FORCE)
        set(SANITIZER_COMPILE_DEFINITIONS "" CACHE INTERNAL "" FORCE)
        set(SANITIZER_DEPLOY_DLLS "" CACHE INTERNAL "" FORCE)
        set(SANITIZER_LINK_LIBS "" CACHE INTERNAL "" FORCE)
        message(FATAL_ERROR "Failed to verify the requested sanitizer setup.")
    else ()
        set(SANITIZER_COMPILE_OPTIONS ${local_shared_flag_list} CACHE INTERNAL "" FORCE)
        set(SANITIZER_COMPILE_DEFINITIONS ${local_shared_def_list} CACHE INTERNAL "" FORCE)
        set(SANITIZER_DEPLOY_DLLS ${local_shared_run_dlls} CACHE INTERNAL "" FORCE)
        set(SANITIZER_LINK_LIBS ${local_shared_link_libs} CACHE INTERNAL "" FORCE)
        message(STATUS "Setup sanitizers as requested (${SANITIZERS}).")
    endif ()

endfunction()
_sanitizer_init()
