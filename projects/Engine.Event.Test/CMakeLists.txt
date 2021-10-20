# Target Name
set(target Engine.Event.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.Event
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")