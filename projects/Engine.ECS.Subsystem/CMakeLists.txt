# Target Name
set(target Engine.ECS.Subsystem)

# Linker
set(
        link_libraries
        ${META_PROJECT_NAME}::Engine.Common
        ${META_PROJECT_NAME}::Engine.ECS
        ${META_PROJECT_NAME}::Engine.Session
)

# Setup
make_library_project(${target} "${link_libraries}")