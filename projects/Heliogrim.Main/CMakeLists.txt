# Target Name
set(target Heliogrim.Main)

# Linker
set(
        link_libraries
        ${PROJECT_NAME}::Engine.Common
        ${PROJECT_NAME}::Engine.Core
        ${PROJECT_NAME}::Engine.Core.Game
)

# Setup
make_library_project(${target} "${link_libraries}")