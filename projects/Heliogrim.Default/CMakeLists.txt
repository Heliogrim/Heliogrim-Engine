# Target Name
set(target Heliogrim.Default)

# Linker
set(link_libraries ${META_PROJECT_NAME}::Heliogrim)

# Setup
make_library_project(${target} "${link_libraries}")