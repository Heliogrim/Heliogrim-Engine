# Target Name
set(target Game)

# Discover Sources
set(source_directory "${META_PROJECT_SRC_DIR}/${target}.Main")

file(GLOB_RECURSE header_files ${source_directory}/*.hpp ${source_directory}/*.h ${source_directory}/*.hh)
file(GLOB_RECURSE source_files ${source_directory}/*.cpp ${source_directory}/*.c ${source_directory}/*.cc)

# Sources
set(headers ${header_files})
set(sources ${source_files})

# Executable
add_executable(${target} ${sources} ${headers})
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Project Options
set_target_properties(${target} PROPERTIES ${DEFAULT_PROJECT_OPTIONS} FOLDER "${IDE_FOLDER}")

# Include Directories
target_include_directories(
        ${target}
        PRIVATE
        ${DEFAULT_INCLUDE_DIRECTORIES}
        ${META_PROJECT_SRC_DIR}
)

# Libraries
target_link_libraries(
        ${target}
        PRIVATE
		${DEFAULT_LIBRARIES}
		${META_PROJECT_NAME}::Heliogrim
		${META_PROJECT_NAME}::Heliogrim.Default
		${META_PROJECT_NAME}::Heliogrim.Main
		${META_PROJECT_NAME}::Engine.ACS
		${META_PROJECT_NAME}::Engine.Assets
		${META_PROJECT_NAME}::Engine.Common
		${META_PROJECT_NAME}::Engine.Core
		${META_PROJECT_NAME}::Engine.Core.Game
		${META_PROJECT_NAME}::Engine.Core.Schedule
		${META_PROJECT_NAME}::Engine.Env
		${META_PROJECT_NAME}::Engine.Event
		${META_PROJECT_NAME}::Engine.GFX
		${META_PROJECT_NAME}::Engine.GFX.Glow
		${META_PROJECT_NAME}::Engine.GFX.Glow.3D
		${META_PROJECT_NAME}::Engine.GFX.Glow.UI
		${META_PROJECT_NAME}::Engine.GFX.Scene
		${META_PROJECT_NAME}::Engine.GFX.Schedule
		${META_PROJECT_NAME}::Engine.Input
		${META_PROJECT_NAME}::Engine.Input.Schedule
		${META_PROJECT_NAME}::Engine.Logging
		${META_PROJECT_NAME}::Engine.Network
		${META_PROJECT_NAME}::Engine.PFX
		${META_PROJECT_NAME}::Engine.Platform
		${META_PROJECT_NAME}::Engine.Reflect
		${META_PROJECT_NAME}::Engine.Reflow
		${META_PROJECT_NAME}::Engine.Reflow.Schedule
		${META_PROJECT_NAME}::Engine.Resource
		${META_PROJECT_NAME}::Engine.Scene
		${META_PROJECT_NAME}::Engine.Scheduler
		${META_PROJECT_NAME}::Engine.Serialization
		${META_PROJECT_NAME}::Engine.SFX
		${META_PROJECT_NAME}::Engine.Utils
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
add_deploy_to_target(${target})