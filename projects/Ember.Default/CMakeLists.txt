# Target Name
set(target Ember.Default)

# Linker
set(link_libraries ${META_PROJECT_NAME}::Ember)

# Setup
make_library_project(${target} "${link_libraries}")