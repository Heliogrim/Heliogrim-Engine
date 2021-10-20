# Target Name
set(target Engine.GFX.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.GFX
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")