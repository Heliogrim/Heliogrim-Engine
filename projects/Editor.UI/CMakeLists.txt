# Target Name
set(target Editor.UI)

# Linker
set(link_libraries ${PROJECT_NAME}::Heliogrim)

# Setup
make_library_project(${target} "${link_libraries}")
