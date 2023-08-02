
macro(list_directories retval cwd)

    file(GLOB sub-dir LIST_DIRECTORIES true ${cwd}/*)
    set(list_of "")

    foreach (dir ${sub-dir})
        if (IS_DIRECTORY "${dir}")
            list(APPEND list_of "${dir}")
        endif ()
    endforeach ()

    set(${retval} ${list_of})
endmacro()

# Directories

macro(get_projects_path out)
    set(${out} "${PROJECT_SOURCE_DIR}/projects")
endmacro()

macro(get_src_path out)
    set(${out} "${PROJECT_SOURCE_DIR}/src")
endmacro()

macro(get_library_path out)
    set(${out} "${PROJECT_SOURCE_DIR}/library")
endmacro()

macro(get_dist_path out)
    set(${out} "${PROJECT_SOURCE_DIR}/dist")
endmacro()

# Naming

macro(name_to_target out name)
    set(${out} "${PROJECT_NAME}-${name}")
endmacro()