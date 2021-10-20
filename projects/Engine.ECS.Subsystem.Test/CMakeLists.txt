# Target Name
set(target Engine.ECS.Subsystem.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.ECS.Subsystem
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")