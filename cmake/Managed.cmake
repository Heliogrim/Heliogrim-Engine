include_guard(GLOBAL)
set_property(GLOBAL PROPERTY global_initialized_managed_libraries "")
set_property(GLOBAL PROPERTY global_deferred_managed_libraries "")

# Internal Functions and Macros

macro(_store_global_deferred tuple)
    get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
    list(APPEND __global_deferred ${tuple})
    message(DEBUG "Deferred: ${__global_deferred}")
    set_property(GLOBAL PROPERTY global_deferred_managed_libraries "${__global_deferred}")
endmacro()

macro(_store_global_initialized name)
    get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
    list(APPEND __global_initialized ${name})
    message(DEBUG "Initialized: ${__global_initialized}")
    set_property(GLOBAL PROPERTY global_initialized_managed_libraries "${__global_initialized}")
endmacro()

macro(_load_global_deferred out)
    get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
    set(${out} "${__global_deferred}")
endmacro()

macro(_load_global_initialized out)
    get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
    set(${out} "${__global_initialized}")
endmacro()

# Data En-/Decoding
function(_encode_deferred_tuple local_name local_call guard_file local_deps out)
    set(local_tuple ${local_name} ${local_call} "${guard_file}" "${local_deps}")
    list(LENGTH local_tuple local_length)
    set(${out} ${local_length} ${local_tuple} PARENT_SCOPE)
endfunction()

function(_decode_deferred_tuple tuple out_name out_call out_guard_file out_deps)

    list(LENGTH tuple tuple_size)
    if (${tuple_size} LESS 4)
        message(FATAL_ERROR "Invalid tuple size with tuple: `${tuple}`.")
    endif ()

    list(POP_FRONT tuple stored_size)
    list(LENGTH tuple tuple_size)
    if (NOT ${tuple_size} EQUAL ${stored_size})
        message(FATAL_ERROR "Failed integrity check with tuple: `${tuple}`.")
    endif ()

    list(POP_FRONT tuple local_name)
    list(POP_FRONT tuple local_call)
    list(POP_FRONT tuple local_scope_file)

    set(${out_name} ${local_name} PARENT_SCOPE)
    set(${out_call} ${local_call} PARENT_SCOPE)
    set(${out_guard_file} ${local_scope_file} PARENT_SCOPE)
    set(${out_deps} ${tuple} PARENT_SCOPE)

endfunction()

# Deferred managed storage
function(_store_deferred_library_tuple local_name local_call guard_file local_deps)
    _encode_deferred_tuple(${local_name} ${local_call} "${guard_file}" "${local_deps}" local_tuple)
    _store_global_deferred("${local_tuple}")
endfunction()

function(_drop_deferred_library_tuple offset)
    _load_global_deferred(deferred)
    set(next "")

    if (${offset} GREATER 0)
        list(SUBLIST deferred 0 ${offset} tmp)
        list(APPEND next ${tmp})
    endif ()

    list(LENGTH deferred check_length)
    list(GET deferred ${offset} stored_tuple_size)
    math(EXPR end "${offset} + ${stored_tuple_size} + 1")

    if (${end} LESS ${check_length})
        list(SUBLIST deferred ${end} -1 tmp)
        list(APPEND next ${tmp})
    endif ()

    set_property(GLOBAL PROPERTY global_deferred_managed_libraries "${next}")
endfunction()

# Dispatch Management
function(_dispatch_guard_acquire)

    # Parse Arguments
    set(__single_var_options GUARD_FILE WAIT RESULT)
    set(__multi_var_options)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    if (NOT DEFINED args_GUARD_FILE)
        message(FATAL_ERROR "Calling `_dispatch_guard_acquire` requires argument `GUARD_FILE`.")
    endif ()

    if (DEFINED args_WAIT AND ${args_WAIT})
        file(LOCK "${args_GUARD_FILE}" GUARD PROCESS RESULT_VARIABLE result)
    else ()
        file(LOCK "${args_GUARD_FILE}" GUARD PROCESS RESULT_VARIABLE result TIMEOUT 0)
    endif ()

    if (DEFINED args_RESULT)
        set(${args_RESULT} ${result} PARENT_SCOPE)
        return()
    endif ()

    if (NOT ${result})
        message(FATAL_ERROR "Failed to acquire dispatch guard for `${args_GUARD_FILE}`.")
    endif ()

endfunction()

function(_dispatch_guard_release)

    # Parse Arguments
    set(__single_var_options GUARD_FILE)
    set(__multi_var_options)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    if (NOT DEFINED args_GUARD_FILE)
        message(FATAL_ERROR "Calling `_dispatch_guard_release` requires argument `GUARD_FILE`.")
    endif ()

    if (NOT EXISTS ${args_GUARD_FILE})
        message(FATAL_ERROR "Tried to release a dispatch guard which does not exist (`${args_GUARD_FILE}`).")
    endif ()

    file(LOCK "${args_GUARD_FILE}" RELEASE GUARD PROCESS)
    file(REMOVE "${args_GUARD_FILE}")

endfunction()

function(_dispatch_fnf_managed_call local_name local_call)
    message(STATUS "Initializing managed library `${local_name}`.")
    cmake_language(CALL ${local_call})
    _store_global_initialized(${local_name})
endfunction()

function(_try_dispatch_managed_call wait_for_guard local_name local_call guard_file out_result)
    _dispatch_guard_acquire(GUARD_FILE "${guard_file}" WAIT ${wait_for_guard} RESULT acquired_successful)
    if (NOT ${acquired_successful})
        set(${out_result} False PARENT_SCOPE)
    endif ()

    _dispatch_fnf_managed_call(${local_name} ${local_call})

    _dispatch_guard_release(GUARD_FILE "${guard_file}")
    set(${out_result} True PARENT_SCOPE)
endfunction()

# Global state integrity
function(_check_unique_name name out)
    _load_global_deferred(deferred)
    _load_global_initialized(initialized)

    list(FIND initialized ${name} init_index)
    if (${init_index} EQUAL -1)
        set(${out} ON PARENT_SCOPE)
    else ()
        set(${out} OFF PARENT_SCOPE)
    endif ()

    # TODO: Check deferred targets for names

endfunction()

function(_check_contains_list inspect_set check_set out)
    foreach (item ${check_set})
        list(REMOVE_ITEM inspect_set ${item})
    endforeach ()
    list(LENGTH inspect_set check_size)
    if (${check_size} EQUAL 0)
        set(${out} ON PARENT_SCOPE)
    else ()
        set(${out} OFF PARENT_SCOPE)
    endif ()
endfunction()

# Internal recursive behaviour
function(_try_propagate_changes wait_for_guard out_changed)

    _load_global_deferred(deferred)
    _load_global_initialized(initialized)

    list(LENGTH deferred deferred_size)
    set(deferred_offset 0)
    set(${out_changed} OFF PARENT_SCOPE)

    while (${deferred_size} GREATER 0)

        list(GET deferred 0 stride)
        math(EXPR stride "${stride} + 1")

        # Get first tuple element
        list(SUBLIST deferred 0 ${stride} local_tuple)
        _decode_deferred_tuple("${local_tuple}" local_name local_call guard_file local_deps)

        _check_contains_list("${local_deps}" "${initialized}" satisfied)
        if (${satisfied})

            # Dispatch deferred retrieval
            _try_dispatch_managed_call(${wait_for_guard} ${local_name} ${local_call} "${guard_file}" succeeded)

            if (${succeeded})
                _drop_deferred_library_tuple(${deferred_offset})
                set(${out_changed} ON PARENT_SCOPE)

                _load_global_initialized(initialized)
                math(EXPR deferred_offset "${deferred_offset} - ${stride}")
            endif ()
        endif ()

        # Update loop variables
        math(EXPR deferred_size "${deferred_size} - ${stride}")
        math(EXPR deferred_offset "${deferred_offset} + ${stride}")
        if (${deferred_size} GREATER 0)
            list(SUBLIST deferred ${stride} ${deferred_size} deferred)
            list(LENGTH deferred deferred_size)
        endif ()
    endwhile ()
endfunction()

# Exposed Functions

function(add_managed_library)

    # Verify CMake state integrity
    get_property(global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
    if (NOT DEFINED global_deferred)
        message(FATAL_ERROR "Internal variables not available.")
    endif ()

    # Parse Arguments
    set(__single_var_options NAME PROVIDER ORDER_INDEPENDENT CALL GUARD_FILE)
    set(__multi_var_options DEPS TARGETS)
    cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

    # Verify Argument Configuration
    if (NOT DEFINED args_NAME)
        message(FATAL_ERROR "Argument `NAME` of type string is required when calling `add_managed_library`.")
    endif ()

    _check_unique_name(${args_NAME} is_valid_name)
    if (NOT ${is_valid_name})
        message(FATAL_ERROR "Name `${args_NAME}` is already known as a managed library.")
    endif ()

    if (NOT DEFINED args_CALL)
        message(FATAL_ERROR "Argument `CALL` of type function is required when calling `add_managed_library`.")
    endif ()

    if (DEFINED args_ORDER_INDEPENDENT AND DEFINED args_DEPS)
        message(WARNING "Omitting `DEPS` (`ORDER_INDEPENDENT` takes precedence) when calling `add_managed_library`.")
        unset(args_DEPS)
    endif ()

    if (DEFINED args_PROVIDER AND NOT "${args_PROVIDER}" IN_LIST "FIND;FETCH_CONTENT;EXTERNAL_PROJECT")
        message(FATAL_ERROR "Argument `PROVIDER` is defined but does not have value `FIND`, `FETCH_CONTENT` or `EXTERNAL_PROJECT`.")
    endif ()

    if (NOT DEFINED args_GUARD_FILE)
        set(args_GUARD_FILE "${CMAKE_CURRENT_LIST_FILE}.lock")
    endif ()

    # Check for unsatisfied dependencies
    set(invoke_immediately OFF)
    if (DEFINED args_ORDER_INDEPENDENT)
        set(invoke_immediately ON)
    else ()
        _load_global_initialized(initialized)
        _check_contains_list("${args_DEPS}" "${initialized}" satisfied)
        set(invoke_immediately ${satisfied})
    endif ()

    if (NOT ${invoke_immediately})

        # Store deferred function
        if (NOT DEFINED args_DEPS)
            message(FATAL_ERROR "Discovered deferred library without any dependencies.")
        endif ()

        _store_deferred_library_tuple(${args_NAME} ${args_CALL} "${args_GUARD_FILE}" "${args_DEPS}")

    else ()

        # Dispatch immediate retrieval
        _try_dispatch_managed_call(OFF ${args_NAME} ${args_CALL} "${args_GUARD_FILE}" succeeded)

        while (${succeeded})
            _try_propagate_changes(OFF succeeded)
        endwhile ()

    endif ()

endfunction()

function(finalize_managed_libraries)

    macro(_load_global_deferred out)
        get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
        set(${out} "${__global_deferred}")
    endmacro()

    macro(_load_global_initialized out)
        get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
        set(${out} "${__global_initialized}")
    endmacro()

    _load_global_deferred(deferred)
    _load_global_initialized(initialized)

    list(LENGTH deferred deferred_length)
    list(LENGTH initialized initialized_length)

    # If leftover initialization, force looping for at most once per target ( may occur due to parallel locking )
    set(succeeded ON)
    set(repeat ${deferred_length})
    while (${succeeded} OR (NOT ${succeeded} AND ${repeat} GREATER 0))
        _try_propagate_changes(ON succeeded)
        math(EXPR repeat "${repeat} - 1")
    endwhile ()

    if (${deferred_length} GREATER 0)
        message(FATAL_ERROR "Could not resolve dependencies and initialization of managed libraries.")
    endif ()

    message(STATUS "Finished initializing `${initialized_length}` managed libraries.")
endfunction()

# FetchContent Override
include(FetchContent)

function(CachedFetchContent_Declare content_name)

    # Helper Functions ( Partially extracted from ExternalProject )

    # @see ExternalProject::_ep_parse_arguments(...)
    function(_adv_parse_patch_arguments out args)

        set(keywords
                #
                # Directory options
                #
                PREFIX
                TMP_DIR
                STAMP_DIR
                LOG_DIR
                DOWNLOAD_DIR
                SOURCE_DIR
                BINARY_DIR
                INSTALL_DIR
                #
                # Download step options
                #
                DOWNLOAD_COMMAND
                #
                URL
                URL_HASH
                URL_MD5
                DOWNLOAD_NAME
                DOWNLOAD_EXTRACT_TIMESTAMP
                DOWNLOAD_NO_EXTRACT
                DOWNLOAD_NO_PROGRESS
                TIMEOUT
                INACTIVITY_TIMEOUT
                HTTP_USERNAME
                HTTP_PASSWORD
                HTTP_HEADER
                TLS_VERIFY     # Also used for git clone operations
                TLS_CAINFO
                NETRC
                NETRC_FILE
                #
                GIT_REPOSITORY
                GIT_TAG
                GIT_REMOTE_NAME
                GIT_SUBMODULES
                GIT_SUBMODULES_RECURSE
                GIT_SHALLOW
                GIT_PROGRESS
                GIT_CONFIG
                GIT_REMOTE_UPDATE_STRATEGY
                #
                SVN_REPOSITORY
                SVN_REVISION
                SVN_USERNAME
                SVN_PASSWORD
                SVN_TRUST_CERT
                #
                HG_REPOSITORY
                HG_TAG
                #
                CVS_REPOSITORY
                CVS_MODULE
                CVS_TAG
                #
                # Update step options
                #
                UPDATE_COMMAND
                UPDATE_DISCONNECTED
                #
                # Patch step options
                #
                PATCH_COMMAND
                #
                # Configure step options
                #
                CONFIGURE_COMMAND
                CMAKE_COMMAND
                CMAKE_GENERATOR
                CMAKE_GENERATOR_PLATFORM
                CMAKE_GENERATOR_TOOLSET
                CMAKE_GENERATOR_INSTANCE
                CMAKE_ARGS
                CMAKE_CACHE_ARGS
                CMAKE_CACHE_DEFAULT_ARGS
                SOURCE_SUBDIR
                CONFIGURE_HANDLED_BY_BUILD
                #
                # Build step options
                #
                BUILD_COMMAND
                BUILD_IN_SOURCE
                BUILD_ALWAYS
                BUILD_BYPRODUCTS
                BUILD_JOB_SERVER_AWARE
                #
                # Install step options
                #
                INSTALL_COMMAND
                INSTALL_BYPRODUCTS
                #
                # Test step options
                #
                TEST_COMMAND
                TEST_BEFORE_INSTALL
                TEST_AFTER_INSTALL
                TEST_EXCLUDE_FROM_MAIN
                #
                # Logging options
                #
                LOG_DOWNLOAD
                LOG_UPDATE
                LOG_PATCH
                LOG_CONFIGURE
                LOG_BUILD
                LOG_INSTALL
                LOG_TEST
                LOG_MERGED_STDOUTERR
                LOG_OUTPUT_ON_FAILURE
                #
                # Terminal access options
                #
                USES_TERMINAL_DOWNLOAD
                USES_TERMINAL_UPDATE
                USES_TERMINAL_PATCH
                USES_TERMINAL_CONFIGURE
                USES_TERMINAL_BUILD
                USES_TERMINAL_INSTALL
                USES_TERMINAL_TEST
                #
                # Target options
                #
                DEPENDS
                EXCLUDE_FROM_ALL
                STEP_TARGETS
                INDEPENDENT_STEP_TARGETS
                #
                # Miscellaneous options
                #
                LIST_SEPARATOR
                #
                # Internal options (undocumented)
                #
                EXTERNALPROJECT_INTERNAL_ARGUMENT_SEPARATOR
        )

        set(needle PATCH_COMMAND)
        set(result "")

        set(key)
        foreach (arg IN LISTS args)
            set(is_value 1)

            if (arg MATCHES "^[A-Z][A-Z0-9_][A-Z0-9_]+$" AND NOT (("x${arg}x" STREQUAL "x${key}x") AND ("x${key}x" STREQUAL "xCOMMANDx")) AND NOT arg MATCHES "^(TRUE|FALSE)$")
                if (arg IN_LIST keywords)
                    set(is_value 0)
                endif ()
            endif ()

            if (is_value)
                if (key)
                    if (NOT key STREQUAL "${needle}")
                        # TODO: Maybe extract other parameters
                    else ()
                        if (NOT arg STREQUAL "")
                            list(APPEND result "${arg}")
                        else ()
                            set(result "${arg}")
                        endif ()
                    endif ()
                else ()
                    # Missing Keyword
                    message(AUTHOR_WARNING "value '${arg}' with no previous keyword in ${f}")
                endif ()
            else ()
                set(key "${arg}")
            endif ()
        endforeach ()

        # Export result
        set(${out} "${result}" PARENT_SCOPE)
    endfunction()


    # Implementation
    string(TOUPPER "${content_name}" upper_content_name)
    string(TOLOWER "${content_name}" lower_content_name)

    cmake_parse_arguments(PARSE_ARGV 1 args "" "FETCHCONTENT_SOURCE_DIR_${upper_content_name}" "")
    if (DEFINED ${args_FETCHCONTENT_SOURCE_DIR_${upper_content_name}})
        FetchContent_Declare(${content_name} ${ARGN})
        return()
    endif ()

    cmake_parse_arguments(PARSE_ARGV 1 args "" "" "SOURCE_DIR;SUBBUILD_DIR")
    if (NOT DEFINED args_SOURCE_DIR)
        message(FATAL_ERROR "Call to `CachedFetchContent_Declare` missing required argument `SOURCE_DIR`.")
    endif ()

    set(patch_cmd_index_end 0)
    set(inject_patch_prefix "PATCH_COMMAND")
    list(FIND ARGN "PATCH_COMMAND" patch_cmd_index)

    if (${patch_cmd_index} GREATER_EQUAL 0)
        list(SUBLIST ARGN ${patch_cmd_index} -1 unparsed_patch_args)
        _adv_parse_patch_arguments(patch_args "${unparsed_patch_args}")

        list(LENGTH patch_args sequence_length)
        math(EXPR patch_cmd_index_end "${patch_cmd_index} + ${sequence_length} + 1")
        set(inject_patch_prefix "PATCH_COMMAND" ${patch_args} "COMMAND")
    endif ()

    set(rest_command "")
    if (${patch_cmd_index_end} GREATER ${patch_cmd_index})
        if (${patch_cmd_index} GREATER 0)
            list(SUBLIST ARGN 0 ${patch_cmd_index} tmp)
            set(rest_command "${rest_command};${tmp}")
        endif ()
        list(LENGTH ARGN argn_length)
        if (${patch_cmd_index_end} LESS ${argn_length})
            list(SUBLIST ARGN ${patch_cmd_index_end} -1 tmp)
            set(rest_command "${rest_command};${tmp}")
        endif ()
    endif ()

    # Check whether we have already populated data
    list(GET args_SOURCE_DIR 0 args_SOURCE_DIR)
    file(GLOB check_populated "${args_SOURCE_DIR}/**")
    list(FILTER check_populated EXCLUDE REGEX ".*(\.git)$")
    list(LENGTH check_populated check_populated_length)

    # Check whether the populated data was already signed
    get_filename_component(source_parent "${args_SOURCE_DIR}" PATH)
    set(signature_file "${source_parent}/.signature")

    if (NOT EXISTS "${args_SOURCE_DIR}" OR ${check_populated_length} LESS_EQUAL 0 OR NOT EXISTS "${signature_file}")

        # Reset managed cached variable
        set(FETCHCONTENT_SOURCE_DIR_${upper_content_name} "" CACHE PATH "" FORCE)
        if (EXISTS "${FETCHCONTENT_BASE_DIR}/${lower_content_name}-subbuild")
            message(DEBUG "Invalidating subbuild directory to prevent git errors while reconstructing intermediate library state.")
            file(REMOVE_RECURSE "${FETCHCONTENT_BASE_DIR}/${lower_content_name}-subbuild")
        endif ()

        # Sign target on patch command
        string(SHA256 command_signature "${content_name};${ARGN}")
        set(INJECT_CMD_EXPORT_SIG ${inject_patch_prefix} ${CMAKE_COMMAND} -E echo "${command_signature}" > ${signature_file})

        FetchContent_Declare(${content_name} ${INJECT_CMD_EXPORT_SIG} ${rest_command})
        return()
    endif ()

    # Verify caller signature
    # TODO: Hold some kind of verification state to check the integrity

    file(READ "${signature_file}" stored_signature)
    string(STRIP "${stored_signature}" verify_signature)
    string(SHA256 command_signature "${content_name};${ARGN}")

    if (${verify_signature} STREQUAL ${command_signature})
        set(FETCHCONTENT_SOURCE_DIR_${upper_content_name} "${args_SOURCE_DIR}" CACHE PATH "" FORCE)
        FetchContent_Declare(${content_name} ${rest_command})
        return()
    endif ()

    # Fallback to population due to mismatch
    set(FETCHCONTENT_SOURCE_DIR_${upper_content_name} "" CACHE PATH "" FORCE)

    string(SHA256 command_signature "${content_name};${ARGN}")
    set(INJECT_CMD_EXPORT_SIG ${inject_patch_prefix} ${CMAKE_COMMAND} -E echo_append "${command_signature}" > ${signature_file})

    FetchContent_Declare(${content_name} ${INJECT_CMD_EXPORT_SIG} ${rest_command})

endfunction()