# Target Name
set(target Engine.GFX.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.GFX
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")