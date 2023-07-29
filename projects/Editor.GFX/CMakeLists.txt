# Target Name
set(target Editor.GFX)

# Linker
set(link_libraries ${PROJECT_NAME}::Heliogrim)

# Setup
make_library_project(${target} "${link_libraries}")
