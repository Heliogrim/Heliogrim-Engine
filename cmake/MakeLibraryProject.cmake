
function(__prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_library_project)

	set(__single_var_options LIB_NAME SOURCE)
	set(__multi_var_options LIBRARIES)
	cmake_parse_arguments(make_lib_vars "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED make_lib_vars_LIB_NAME)
		message(FATAL_ERROR "Target `${make_lib_vars_LIB_NAME}` is required parameter name to generate setup.")
		return()
	endif ()

	message(STATUS "Setup library project for target `${make_lib_vars_LIB_NAME}`.")
	name_to_target(target "${make_lib_vars_LIB_NAME}")

	list(LENGTH make_lib_vars_LIBRARIES link_libraries_size)
	if (link_libraries_size GREATER 0)
		__prepare_link_libs(${make_lib_vars_LIBRARIES} make_lib_vars_LIBRARIES)
		list(LENGTH make_lib_vars_LIBRARIES link_libraries_size)
	endif ()
	message(STATUS "Using `${link_libraries_size}` libraries at linker.")

	# Discover Sources
	get_src_path(proj_src_dir)

	if (DEFINED make_lib_vars_SOURCE)
		set(source_directory "${proj_src_dir}/${make_lib_vars_SOURCE}")
	else ()
		set(source_directory "${proj_src_dir}/${make_lib_vars_LIB_NAME}")
	endif ()

	file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
	file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

	# Sources
	set(sources ${source_files})

	# Library
	add_library(${target} ${sources})
	add_library(${PROJECT_NAME}::${make_lib_vars_LIB_NAME} ALIAS ${target})

	# Export Downstream
	export(TARGETS ${target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

	# Project Options
	set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

	# Include Directories
	target_include_directories(
			${target}
			PRIVATE
			${DEFAULT_INCLUDE_DIRECTORIES}
			${proj_src_dir}
	)

	# Libraries
	target_link_libraries(
			${target}
			PRIVATE
			${DEFAULT_LIBRARIES}
			${make_lib_vars_LIBRARIES}
	)

	# Compile
	target_compile_definitions(
			${target}
			PRIVATE
			${DEFAULT_COMPILE_DEFINITIONS}
	)

	# Compile Options
	target_compile_options(
			${target}
			PRIVATE
			${DEFAULT_COMPILE_OPTIONS}
	)

	# Linker Options
	target_link_options(
			${target}
			PRIVATE
			${DEFAULT_LINKER_OPTIONS}
	)

	# Target Health
	# Deployment
	get_shared_dist_path(proj_dist_path)
	set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_path})
	set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_path})
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_path})
endfunction()