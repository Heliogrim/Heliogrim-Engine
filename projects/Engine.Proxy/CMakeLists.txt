# Target Name
set(target Engine.Proxy)

# Linker
set(link_libraries ${META_PROJECT_NAME}::Engine.Common)

# Setup
make_library_project(${target} "${link_libraries}")