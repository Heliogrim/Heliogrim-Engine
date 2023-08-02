# Target Name
set(target Engine.SFX.Test)

# Linker
set(link_libraries
        ${PROJECT_NAME}::Engine.SFX
        ${PROJECT_NAME}::Engine.Common)

# Setup
make_test_project(${target} "${link_libraries}")