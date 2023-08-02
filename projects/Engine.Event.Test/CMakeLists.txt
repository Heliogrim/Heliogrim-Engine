# Target Name
set(target Engine.Event.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.Event
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")