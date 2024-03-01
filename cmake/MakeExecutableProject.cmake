
function(__prepare_link_libs libs out)
	list(TRANSFORM libs STRIP)
	set("${out}" ${libs} PARENT_SCOPE)
endfunction()

function(make_app_project)

	set(__single_var_options APP_NAME LIB_NAME SOURCE)
	set(__multi_var_options LIBRARIES)
	cmake_parse_arguments(make_app_vars "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	if (NOT DEFINED make_app_vars_APP_NAME)
		message(FATAL_ERROR "Target `${make_app_vars_APP_NAME}` is required parameter name to generate setup.")
		return()
	endif ()

	if (NOT DEFINED make_app_vars_LIB_NAME)
		message(STATUS "Using `${make_app_vars_APP_NAME}` as root to autogenerate library name.")
		set(make_app_vars_LIB_NAME "${make_app_vars_APP_NAME}-Lib")
	endif ()

	message(STATUS "Setup executable and library project for target `${make_app_vars_APP_NAME}` and `${make_app_vars_LIB_NAME}` respectively.")
	name_to_target(app_target ${make_app_vars_APP_NAME})
	name_to_target(lib_target ${make_app_vars_LIB_NAME})

	# Prepare Link Libraries
	list(LENGTH make_app_vars_LIBRARIES link_libraries_size)
	if (link_libraries_size GREATER 0)
		__prepare_link_libs(${make_app_vars_LIBRARIES} make_app_vars_LIBRARIES)
		list(LENGTH make_app_vars_LIBRARIES link_libraries_size)
	endif ()
	message(STATUS "Using `${link_libraries_size}` libraries at linker.")

	# Discover Sources
	get_src_path(proj_src_dir)

	if (NOT DEFINED make_app_vars_SOURCE)
		if (EXISTS "${proj_src_dir}/${make_app_vars_APP_NAME}")
			set(source_directory "${proj_src_dir}/${make_app_vars_APP_NAME}")
		elseif (EXISTS "${proj_src_dir}/${make_app_vars_LIB_NAME}")
			set(source_directory "${proj_src_dir}/${make_app_vars_LIB_NAME}")
		else ()
			set(source_directory "${proj_src_dir}/${make_app_vars_APP_NAME}")
		endif ()
	else ()
		set(source_directory "${proj_src_dir}/${make_app_vars_SOURCE}")
	endif ()

	file(GLOB_RECURSE header_files CONFIGURE_DEPENDS ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
	file(GLOB_RECURSE source_files CONFIGURE_DEPENDS ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

	# Sources
	set(headers ${header_files})
	set(sources ${source_files})

	# Executable
	add_executable(${app_target} ${sources} ${headers})
	add_executable(${PROJECT_NAME}::${make_app_vars_APP_NAME} ALIAS ${app_target})

	# Library
	add_library(${lib_target} ${sources})
	add_library(${PROJECT_NAME}::${make_app_vars_LIB_NAME} ALIAS ${lib_target})

	# Export Downstream
	export(TARGETS ${lib_target} NAMESPACE ${PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${lib_target}/${lib_target}-export.cmake)

	# Project Options
	set_target_properties(${app_target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")
	set_target_properties(${lib_target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

	# Include Directories
	target_include_directories(
			${app_target}
			PRIVATE
			${DEFAULT_INCLUDE_DIRECTORIES}
			${proj_src_dir}
	)
	target_include_directories(
			${lib_target}
			PRIVATE
			${DEFAULT_INCLUDE_DIRECTORIES}
			${proj_src_dir}
	)

	# Link Libraries
	target_link_libraries(
			${app_target}
			PRIVATE
			${DEFAULT_LIBRARIES}
			${make_app_vars_LIBRARIES}
	)
	target_link_libraries(
			${lib_target}
			PRIVATE
			${DEFAULT_LIBRARIES}
			${make_app_vars_LIBRARIES}
	)

	# Compile Definitions
	target_compile_definitions(
			${app_target}
			PRIVATE
			${DEFAULT_COMPILE_DEFINITIONS}
	)

	# Compile Options
	target_compile_options(
			${app_target}
			PRIVATE
			${DEFAULT_COMPILE_OPTIONS}
	)
	target_compile_options(
			${lib_target}
			PRIVATE
			${DEFAULT_COMPILE_OPTIONS}
	)

	# Linker Options
	target_link_options(
			${app_target}
			PRIVATE
			${DEFAULT_LINKER_OPTIONS}
	)
	target_link_options(
			${lib_target}
			PRIVATE
			${DEFAULT_LINKER_OPTIONS}
	)

	# Target Health
	get_dist_path(proj_dist_dir)

	set_target_properties(${app_target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
	set_target_properties(${app_target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
	set_target_properties(${app_target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)

	set_target_properties(${lib_target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
	set_target_properties(${lib_target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
	set_target_properties(${lib_target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)

	# Deployment
	add_deploy_to_target(${app_target})

endfunction()