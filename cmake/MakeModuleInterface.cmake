include_guard(GLOBAL)

# Module Interface Generation
#  Generates a primary module interface that exports a bundle's existing headers.
#  Own headers go into `export extern "C++" {}`, so entities stay attached to the global module
#  and remain the same entities a textual `#include` sees.

# CLion excludes its build dirs from indexing, so interfaces generated there stay unresolved in the editor.
#  One dir shared by all profiles: the content depends only on headers, and a copy per profile makes names ambiguous.
if (DEFINED ENV{CLION_IDE})
	set(__default_module_dir "${CMAKE_SOURCE_DIR}/.modules")
else ()
	set(__default_module_dir "${CMAKE_BINARY_DIR}/modules")
endif ()
set(HELIOGRIM_MODULE_DIR "${__default_module_dir}" CACHE PATH "Directory receiving generated module interfaces; keep it out of IDE-excluded dirs to get editor support.")

# Internal Functions

function(_write_if_changed path content)
	# Temp file per build dir: profiles sharing `HELIOGRIM_MODULE_DIR` may configure concurrently.
	cmake_path(GET path FILENAME name)
	set(tmp "${CMAKE_BINARY_DIR}/module-tmp/${name}")
	file(WRITE "${tmp}" "${content}")
	cmake_path(GET path PARENT_PATH dir)
	file(MAKE_DIRECTORY "${dir}")
	file(COPY_FILE "${tmp}" "${path}" ONLY_IF_DIFFERENT)
	file(REMOVE "${tmp}")
endfunction()

# Provider target `${PROJECT_NAME}-<name>.Module` (alias `${PROJECT_NAME}::<name>.Module`), built like a library project.
function(_make_module_provider out name)
	name_to_target(provider "${name}.Module")
	add_library(${provider} OBJECT)
	add_library(${PROJECT_NAME}::${name}.Module ALIAS ${provider})

	# Bundles linking the provider are exported, so the provider has to be too.
	export(TARGETS ${provider} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${provider}/${provider}-export.cmake)
	_apply_default_compile_settings(${provider})

	# External targets only: importers get their usage requirements (e.g. Jolt's `-mavx2`), and a BMI must match them,
	#  else CMake 4.4 builds a second, synthetic BMI per consumer.
	target_link_libraries(${provider} PRIVATE ${DEFAULT_LIBRARIES})
	set(${out} "${provider}" PARENT_SCOPE)
endfunction()

function(_make_module_interface_content out)
	set(__single_var_options MODULE_NAME BASE_DIR)
	set(__multi_var_options HEADERS OWN_PREFIXES EXCLUDE)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	# Resolve per header: relative path, and the resolved paths of everything it includes.
	set(excluded_paths "")
	foreach (excluded IN LISTS args_EXCLUDE)
		cmake_path(ABSOLUTE_PATH excluded BASE_DIRECTORY "${args_BASE_DIR}" NORMALIZE OUTPUT_VARIABLE excluded_path)
		list(APPEND excluded_paths "${excluded_path}")
	endforeach ()

	# Close exclusion over includers: a header that includes an excluded one would drag it into the purview.
	set(changed ON)
	while (changed)
		set(changed OFF)
		foreach (header IN LISTS args_HEADERS)
			cmake_path(NORMAL_PATH header OUTPUT_VARIABLE header_path)
			if (header_path IN_LIST excluded_paths)
				continue()
			endif ()

			cmake_path(GET header_path PARENT_PATH header_dir)

			file(STRINGS "${header}" include_lines REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"]")
			foreach (line IN LISTS include_lines)
				if (line MATCHES "^[ \t]*#[ \t]*include[ \t]*\"([^\"]+)\"")
					cmake_path(ABSOLUTE_PATH CMAKE_MATCH_1 BASE_DIRECTORY "${header_dir}" NORMALIZE OUTPUT_VARIABLE resolved)
				elseif (line MATCHES "^[ \t]*#[ \t]*include[ \t]*<([^>]+)>")
					cmake_path(ABSOLUTE_PATH CMAKE_MATCH_1 BASE_DIRECTORY "${args_BASE_DIR}" NORMALIZE OUTPUT_VARIABLE resolved)
				else ()
					continue()
				endif ()

				if (resolved IN_LIST excluded_paths)
					list(APPEND excluded_paths "${header_path}")
					set(changed ON)
					break()
				endif ()
			endforeach ()
		endforeach ()
	endwhile ()

	set(config_lines "")
	set(own_lines "")
	set(foreign_names "")
	set(import_names "")
	foreach (header IN LISTS args_HEADERS)
		cmake_path(NORMAL_PATH header OUTPUT_VARIABLE header_path)
		if (header_path IN_LIST excluded_paths)
			continue()
		endif ()

		file(RELATIVE_PATH rel "${args_BASE_DIR}" "${header}")

		file(STRINGS "${header}" import_lines REGEX "^[ \t]*import[ \t]+[A-Za-z0-9_.:]+[ \t]*;")
		foreach (line IN LISTS import_lines)
			string(REGEX REPLACE "^[ \t]*import[ \t]+([A-Za-z0-9_.:]+)[ \t]*;.*$" "\\1" imported "${line}")
			list(APPEND import_names "${imported}")
		endforeach ()

		# Macro and vendor-config headers stay textual and first (they order `#define`s before vendor includes).
		file(STRINGS "${header}" define_lines REGEX "^[ \t]*#[ \t]*define[ \t]")
		if (define_lines)
			string(APPEND config_lines "#include <${rel}>\n")
			continue()
		endif ()

		# Quoted on purpose: clang-scan-deps reports angled purview includes regardless of `-Wno-include-angled-in-module-purview`.
		string(APPEND own_lines "#include \"${rel}\"\n")

		file(STRINGS "${header}" include_lines REGEX "^[ \t]*#[ \t]*include[ \t]*<[^>]+>")
		foreach (line IN LISTS include_lines)
			string(REGEX REPLACE "^[ \t]*#[ \t]*include[ \t]*<([^>]+)>.*$" "\\1" inc "${line}")
			string(REGEX REPLACE "/.*$" "" first_segment "${inc}")
			if (NOT first_segment IN_LIST args_OWN_PREFIXES)
				list(APPEND foreign_names "${inc}")
			endif ()
		endforeach ()
	endforeach ()

	list(REMOVE_DUPLICATES foreign_names)
	list(REMOVE_DUPLICATES import_names)

	set(foreign_lines "")
	foreach (inc IN LISTS foreign_names)
		string(APPEND foreign_lines "#include <${inc}>\n")
	endforeach ()

	# Re-exported, so importers keep seeing what the headers brought in textually.
	set(import_lines "")
	foreach (imported IN LISTS import_names)
		string(APPEND import_lines "export import ${imported};\n")
	endforeach ()

	set(content "// Generated by make_module_interface; do not edit.\nmodule;\n#define HG_MODULE_INTERFACE_UNIT\n${config_lines}${foreign_lines}export module ${args_MODULE_NAME};\n${import_lines}\nexport extern \"C++\" {\n${own_lines}}\n")
	set(${out} "${content}" PARENT_SCOPE)
	set(${out}_IMPORTS "${import_names}" PARENT_SCOPE)
endfunction()

function(_make_vendor_module_content out)
	set(__single_var_options MODULE_NAME SOURCE SCAN_DIR)
	set(__multi_var_options PRELUDE HEADERS SKIP)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	set(prelude "")
	foreach (inc IN LISTS args_PRELUDE)
		string(APPEND prelude "#include <${inc}>\n")
	endforeach ()

	if (args_HEADERS)
		# Generated (style A): every include leaving `SCAN_DIR` goes to the global fragment, so its guard fires before the purview.
		file(GLOB_RECURSE scanned LIST_DIRECTORIES false "${args_SCAN_DIR}/*.h" "${args_SCAN_DIR}/*.hh" "${args_SCAN_DIR}/*.hpp" "${args_SCAN_DIR}/*.inl")
		set(foreign_names "")
		foreach (header IN LISTS scanned)
			file(STRINGS "${header}" include_lines REGEX "^[ \t]*#[ \t]*include[ \t]*<[^>]+>")
			foreach (line IN LISTS include_lines)
				string(REGEX REPLACE "^[ \t]*#[ \t]*include[ \t]*<([^>]+)>.*$" "\\1" inc "${line}")
				if (NOT EXISTS "${args_SCAN_DIR}/${inc}" AND NOT inc IN_LIST args_SKIP)
					list(APPEND foreign_names "${inc}")
				endif ()
			endforeach ()
		endforeach ()

		list(REMOVE_DUPLICATES foreign_names)

		set(foreign_lines "")
		foreach (inc IN LISTS foreign_names)
			string(APPEND foreign_lines "#include <${inc}>\n")
		endforeach ()

		set(entry_lines "")
		foreach (header IN LISTS args_HEADERS)
			string(APPEND entry_lines "#include \"${header}\"\n")
		endforeach ()

		set(${out} "// Generated by make_vendor_module from ${args_SCAN_DIR}; do not edit.\nmodule;\n#define HG_MODULE_INTERFACE_UNIT\n${prelude}${foreign_lines}export module ${args_MODULE_NAME};\n\nexport extern \"C++\" {\n${entry_lines}}\n" PARENT_SCOPE)
		return()
	endif ()

	file(READ "${args_SOURCE}" text)
	string(FIND "${text}" "export module " body_at)
	if (body_at EQUAL -1)
		message(FATAL_ERROR "Vendor module source `${args_SOURCE}` has no `export module` declaration.")
	endif ()

	# The vendor's global module fragment is dropped; PRELUDE (our config) replaces it.
	string(SUBSTRING "${text}" ${body_at} -1 body)
	string(REGEX REPLACE "export module [^;]+;" "export module ${args_MODULE_NAME};" body "${body}")

	# `export import` (`import std`, partitions) is dropped; std comes in through the prelude.
	string(REGEX REPLACE "export import [^;]+;[ \t]*\r?\n?" "" body "${body}")

	set(${out} "// Generated by make_vendor_module from ${args_SOURCE}; do not edit.\nmodule;\n#define HG_MODULE_INTERFACE_UNIT\n${prelude}\n${body}" PARENT_SCOPE)
endfunction()

# MSVC toolset dir holding `modules/std.ixx`: `VCToolsInstallDir` from the environment, else `HELIOGRIM_MSVC_TOOLSET_DIR`.
#  Neither: `out` is empty and `out_error` says why.
function(_find_std_toolset_dir out out_error)
	set(toolset_dir "$ENV{VCToolsInstallDir}")
	if (NOT EXISTS "${toolset_dir}/modules/std.ixx")
		set(toolset_dir "${HELIOGRIM_MSVC_TOOLSET_DIR}")
	endif ()

	if (NOT EXISTS "${toolset_dir}/modules/std.ixx")
		set(${out} "" PARENT_SCOPE)
		set(${out_error} "No `modules/std.ixx` below `VCToolsInstallDir` (`$ENV{VCToolsInstallDir}`) or `HELIOGRIM_MSVC_TOOLSET_DIR` (`${HELIOGRIM_MSVC_TOOLSET_DIR}`)." PARENT_SCOPE)
		return()
	endif ()

	cmake_path(NORMAL_PATH toolset_dir)
	set(${out} "${toolset_dir}" PARENT_SCOPE)
	set(${out_error} "" PARENT_SCOPE)
endfunction()

# Public Functions

function(make_module_interface)
	set(__single_var_options TARGET MODULE_NAME BASE_DIR)
	set(__multi_var_options HEADERS OWN_PREFIXES EXCLUDE)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	_make_module_interface_content(content
			MODULE_NAME "${args_MODULE_NAME}" BASE_DIR "${args_BASE_DIR}"
			HEADERS ${args_HEADERS} OWN_PREFIXES ${args_OWN_PREFIXES} EXCLUDE ${args_EXCLUDE}
	)

	set(path "${HELIOGRIM_MODULE_DIR}/${args_MODULE_NAME}.cppm")
	_write_if_changed("${path}" "${content}")

	target_sources(${args_TARGET} PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${HELIOGRIM_MODULE_DIR}" FILES "${path}")
	# `HG_MODULE_INTERFACE_UNIT` is only read by headers that import.
	target_compile_options(${args_TARGET} PRIVATE -Wno-unused-macros)

	set(MODULE_INTERFACE_IMPORTS "${content_IMPORTS}" PARENT_SCOPE)
endfunction()

# Creates `${PROJECT_NAME}-<MODULE_NAME>.Module`, either adapted from an upstream interface (`SOURCE`)
#  or generated over entry `HEADERS` found below `SCAN_DIR` (`SKIP`: includes of other platforms).
function(make_vendor_module)
	set(__single_var_options MODULE_NAME SOURCE SCAN_DIR)
	set(__multi_var_options PRELUDE HEADERS SKIP)
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED args_MODULE_NAME OR (NOT DEFINED args_SOURCE AND NOT (args_HEADERS AND DEFINED args_SCAN_DIR)))
		message(FATAL_ERROR "Vendor module requires `MODULE_NAME` and either `SOURCE` or `HEADERS` with `SCAN_DIR`, got `${args_MODULE_NAME}`.")
	endif ()

	_make_vendor_module_content(content
			MODULE_NAME "${args_MODULE_NAME}" SOURCE "${args_SOURCE}" SCAN_DIR "${args_SCAN_DIR}"
			PRELUDE ${args_PRELUDE} HEADERS ${args_HEADERS} SKIP ${args_SKIP}
	)

	set(path "${HELIOGRIM_MODULE_DIR}/${args_MODULE_NAME}.cppm")
	_write_if_changed("${path}" "${content}")

	_make_module_provider(provider "${args_MODULE_NAME}")
	target_sources(${provider} PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${HELIOGRIM_MODULE_DIR}" FILES "${path}")

	# Third-party code; `HG_MODULE_INTERFACE_UNIT` is unused by most.
	target_compile_options(${provider} PRIVATE -w)
endfunction()

# Creates the providers of `std` and `std.compat` from the MSVC toolset's module sources.
function(make_std_module)
	set(HELIOGRIM_MSVC_TOOLSET_DIR "" CACHE PATH "MSVC toolset dir holding `modules/std.ixx`, used when `VCToolsInstallDir` is not set.")

	_find_std_toolset_dir(toolset_dir toolset_error)
	if (toolset_error)
		message(FATAL_ERROR "${toolset_error}")
	endif ()

	foreach (name IN ITEMS std std.compat)
		_make_module_provider(provider "${name}")
		target_sources(${provider} PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${toolset_dir}/modules" FILES "${toolset_dir}/modules/${name}.ixx")
		target_compile_options(${provider} PRIVATE -w)
	endforeach ()

	target_link_libraries(${PROJECT_NAME}-std.compat.Module PRIVATE ${PROJECT_NAME}::std.Module)
endfunction()
