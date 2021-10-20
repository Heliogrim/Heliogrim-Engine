# Target Name
set(target Engine.SFX.Test)

# Linker
set(link_libraries
        ${META_PROJECT_NAME}::Engine.SFX
        ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")