include_guard(GLOBAL)

function(discover_and_hydrate_vendor)

	get_vendor_path(_vendor_root)
	if (${ARGC} GREATER_EQUAL 1)
		set(_vendor_root "${ARGV0}")
	endif ()

	file(GLOB externals CONFIGURE_DEPENDS LIST_DIRECTORIES true "${_vendor_root}/*")
	foreach (external ${externals})
		add_subdirectory("${external}")
	endforeach ()

	finalize_managed_libraries()
endfunction()

function(discover_and_hydrate_targets)

	get_src_path(_target_root)
	if (${ARGC} GREATER_EQUAL 1)
		set(_target_root "${ARGV0}")
	endif ()

	list_directories("targets" "${_target_root}")
	list(FILTER targets EXCLUDE REGEX "(\\.Test)")

	foreach (target IN LISTS targets)
		add_subdirectory(${target})
	endforeach ()
endfunction()

function(discover_and_hydrate_tests)

	get_src_path(_test_root)
	if (${ARGC} GREATER_EQUAL 1)
		set(_test_root "${ARGV0}")
	endif ()

	list_directories("targets" "${_test_root}")
	list(FILTER targets INCLUDE REGEX "\\.Test")

	foreach (target IN LISTS targets)
		add_subdirectory(${target})
	endforeach ()
endfunction()