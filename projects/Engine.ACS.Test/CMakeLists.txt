# Target Name
set(target Engine.ACS.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.ACS
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")