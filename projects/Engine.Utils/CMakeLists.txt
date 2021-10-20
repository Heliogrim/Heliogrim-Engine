# Target Name
set(target Engine.Utils)

# Linker
set(link_libraries ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_library_project(${target} ${link_libraries})