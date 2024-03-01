
function(__prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_test_project)

	set(__single_var_options TEST_NAME SOURCE)
	set(__multi_var_options LIBRARIES)
	cmake_parse_arguments(make_lib_vars "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED make_lib_vars_TEST_NAME)
		message(FATAL_ERROR "Target `${make_lib_vars_TEST_NAME}` is required parameter name to generate setup.")
		return()
	endif ()

	message(STATUS "Setup library project for target `${make_lib_vars_TEST_NAME}`.")
	name_to_target(target "${make_lib_vars_TEST_NAME}")

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
		set(source_directory "${proj_src_dir}/${make_lib_vars_TEST_NAME}")
	endif ()

	file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
	file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

	# Sources
	set(headers ${header_files})
	set(sources ${source_files})

	# Executable
	add_executable(${target} EXCLUDE_FROM_ALL ${sources} ${headers})
	add_executable(${PROJECT_NAME}::${make_lib_vars_TEST_NAME} ALIAS ${target})

	# Project Options
	set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

	# Include Directories
	target_include_directories(
			${target}
			PRIVATE
			${DEFAULT_INCLUDE_DIRECTORIES}
			${TEST_INCLUDE_DIRECTORIES}
			${proj_src_dir}
	)

	# Libraries
	target_link_libraries(
			${target}
			PRIVATE
			${DEFAULT_LIBRARIES}
			${TEST_LIBRARIES}
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
	get_dist_path(proj_dist_path)
	set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)
	set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)
	set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_path}/$<CONFIG>)

	# Register Tests
	include(GoogleTest)
	gtest_discover_tests(${target} DISCOVERY_MODE PRE_TEST)

endfunction()