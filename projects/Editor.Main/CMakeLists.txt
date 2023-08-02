# Target Name
set(name Editor)

message(STATUS "Setup executable project for target `${name}`.")
name_to_target(target ${name})

# Discover Sources
get_src_path(proj_src_dir)
set(source_directory "${proj_src_dir}/${name}.Main")

file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

# Sources
set(headers ${header_files})
set(sources ${source_files})

# Executable
add_executable(${target} ${sources} ${headers})
add_executable(${PROJECT_NAME}::${name} ALIAS ${target})

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
		${PROJECT_NAME}::Editor.Action
		${PROJECT_NAME}::Editor.Core
		${PROJECT_NAME}::Editor.GFX
	    ${PROJECT_NAME}::Editor.UI
        ${PROJECT_NAME}::Heliogrim
		${PROJECT_NAME}::Heliogrim.Default
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
get_dist_path(proj_dist_dir)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)
set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${proj_dist_dir}/$<CONFIG>)

add_deploy_to_target(${target})