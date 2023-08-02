# Target Name
set(target Engine.Assets)

# Linker
set(link_libraries ${PROJECT_NAME}::Engine.Common)

# Setup
make_library_project(${target} "${link_libraries}")