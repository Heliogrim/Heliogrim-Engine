# Target Name
set(target Engine.PFX.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.PFX
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")