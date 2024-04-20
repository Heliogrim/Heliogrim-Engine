include_guard(GLOBAL)
set_property(GLOBAL PROPERTY global_initialized_managed_libraries "")
set_property(GLOBAL PROPERTY global_deferred_managed_libraries "")

function(add_managed_library)

	# Verify CMake state integrity
	get_property(global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
	if (NOT DEFINED global_deferred)
		message(FATAL_ERROR "Internal variables not available.")
	endif ()

	# Define internal functions
	macro(store_global_deferred tuple)
		get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
		list(APPEND __global_deferred ${tuple})
		message(DEBUG "Deferred: ${__global_deferred}")
		set_property(GLOBAL PROPERTY global_deferred_managed_libraries "${__global_deferred}")
	endmacro()

	macro(store_global_initialized name)
		get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
		list(APPEND __global_initialized ${name})
		message(DEBUG "Initialized: ${__global_initialized}")
		set_property(GLOBAL PROPERTY global_initialized_managed_libraries "${__global_initialized}")
	endmacro()

	macro(load_global_deferred out)
		get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
		set(${out} "${__global_deferred}")
	endmacro()

	macro(load_global_initialized out)
		get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
		set(${out} "${__global_initialized}")
	endmacro()

	function(encode_deferred_tuple local_name local_call local_deps out)
		set(local_tuple ${local_name} ${local_call} "${local_deps}")
		list(LENGTH local_tuple local_length)
		set(${out} ${local_length} ${local_tuple} PARENT_SCOPE)
	endfunction()

	function(decode_deferred_tuple tuple out_name out_call out_deps)

		list(LENGTH tuple tuple_size)
		if (${tuple_size} LESS 3)
			message(FATAL_ERROR "Invalid tuple size with tuple: `${tuple}`.")
		endif ()

		list(POP_FRONT tuple stored_size)
		list(LENGTH tuple tuple_size)
		if (NOT ${tuple_size} EQUAL ${stored_size})
			message(FATAL_ERROR "Failed integrity check with tuple: `${tuple}`.")
		endif ()

		list(POP_FRONT tuple local_name)
		list(POP_FRONT tuple local_call)

		set(${out_name} ${local_name} PARENT_SCOPE)
		set(${out_call} ${local_call} PARENT_SCOPE)
		set(${out_deps} ${tuple} PARENT_SCOPE)

	endfunction()

	function(store_deferred_library_tuple local_name local_call local_deps)
		set(local_tuple ${local_name} ${local_call} "${local_deps}")
		list(LENGTH local_tuple local_length)
		set(local_tuple ${local_length} ${local_tuple})
		store_global_deferred("${local_tuple}")
	endfunction()

	function(drop_deferred_library_tuple offset)
		load_global_deferred(deferred)
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

	function(dispatch_fnf_managed_call local_name local_call)
		message(STATUS "Initializing managed library `${local_name}`.")
		cmake_language(CALL ${local_call})
		store_global_initialized(${local_name})
	endfunction()

	function(check_unique_name name out)
		load_global_deferred(deferred)
		load_global_initialized(initialized)

		list(FIND initialized ${name} init_index)
		if (${init_index} EQUAL -1)
			set(${out} ON PARENT_SCOPE)
		else ()
			set(${out} OFF PARENT_SCOPE)
		endif ()

		# TODO: Check deferred targets for names

	endfunction()

	function(check_contains_list inspect_set check_set out)
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

	function(try_propagate_changes out_changed)

		load_global_deferred(deferred)
		load_global_initialized(initialized)

		list(LENGTH deferred deferred_size)
		set(deferred_offset 0)
		set(${out_changed} OFF PARENT_SCOPE)

		while (${deferred_size} GREATER 0)

			list(GET deferred 0 stride)
			math(EXPR stride "${stride} + 1")

			# Get first tuple element
			list(SUBLIST deferred 0 ${stride} local_tuple)
			decode_deferred_tuple("${local_tuple}" local_name local_call local_deps)

			check_contains_list("${local_deps}" "${initialized}" satisfied)
			if (${satisfied})
				# Dispatch deferred retrieval
				dispatch_fnf_managed_call(${local_name} ${local_call})
				drop_deferred_library_tuple(${deferred_offset})
				set(${out_changed} ON PARENT_SCOPE)

				load_global_initialized(initialized)
				math(EXPR deferred_offset "${deferred_offset} - ${stride}")
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

	# Parse Arguments
	set(__single_var_options NAME PROVIDER ORDER_INDEPENDENT CALL)
	set(__multi_var_options DEPS TARGETS)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	# Verify Argument Configuration
	if (NOT DEFINED args_NAME)
		message(FATAL_ERROR "Argument `NAME` of type string is required when calling `add_managed_library`.")
	endif ()

	check_unique_name(${args_NAME} is_valid_name)
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

	# Check for unsatisfied dependencies
	set(invoke_immediately OFF)
	if (DEFINED args_ORDER_INDEPENDENT)
		set(invoke_immediately ON)
	else ()
		load_global_initialized(initialized)
		check_contains_list("${args_DEPS}" "${initialized}" satisfied)
		set(invoke_immediately ${satisfied})
	endif ()

	if (NOT ${invoke_immediately})

		# Store deferred function
		if (NOT DEFINED args_DEPS)
			message(FATAL_ERROR "Discovered deferred library without any dependencies.")
		endif ()

		store_deferred_library_tuple(${args_NAME} ${args_CALL} "${args_DEPS}")

	else ()

		# Dispatch immediate retrieval
		dispatch_fnf_managed_call(${args_NAME} ${args_CALL})

		set(changed ON)
		while (${changed})
			try_propagate_changes(changed)
		endwhile ()

	endif ()

endfunction()

function(finalize_managed_libraries)

	macro(load_global_deferred out)
		get_property(__global_deferred GLOBAL PROPERTY global_deferred_managed_libraries)
		set(${out} "${__global_deferred}")
	endmacro()

	macro(load_global_initialized out)
		get_property(__global_initialized GLOBAL PROPERTY global_initialized_managed_libraries)
		set(${out} "${__global_initialized}")
	endmacro()

	load_global_deferred(deferred)
	load_global_initialized(initialized)

	list(LENGTH deferred deferred_length)
	list(LENGTH initialized initialized_length)

	if (${deferred_length} GREATER 0)
		message(FATAL_ERROR "Could not resolve dependencies and initialization of managed libraries.")
	endif ()

	message(STATUS "Finished initializing `${initialized_length}` managed libraries.")
endfunction()