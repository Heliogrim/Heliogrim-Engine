# Target Name
set(target Editor.Action)

# Linker
set(link_libraries ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_library_project(${target} "${link_libraries}")