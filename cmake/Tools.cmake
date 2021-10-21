
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