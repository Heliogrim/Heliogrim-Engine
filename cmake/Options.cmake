include_guard(GLOBAL)

#

function(support_opt_linux out)
	if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|DragonFly|GNU")
		set(${out} ON PARENT_SCOPE)
	else ()
		set(${out} OFF PARENT_SCOPE)
	endif ()
endfunction()

function(support_opt_wayland out)
	if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|DragonFly|GNU")
		set(${out} ON PARENT_SCOPE)
	else ()
		set(${out} OFF PARENT_SCOPE)
	endif ()
endfunction()

function(support_opt_x11 out)
	if (CMAKE_SYSTEM_NAME MATCHES "Linux|BSD|DragonFly|GNU")
		set(${out} ON PARENT_SCOPE)
	else ()
		set(${out} OFF PARENT_SCOPE)
	endif ()
endfunction()

#

function(make_option)

	set(__single_var_options VAR NAME CALLBACK DEFAULT ADVANCED)
	set(__multi_var_options "")
	cmake_parse_arguments(args "" "${__single_var_options}" "${__multi_var_options}" ${ARGN})

	#

	if (DEFINED ${args_VAR} AND DEFINED CACHE{${args_VAR}})
		return()
	endif ()

	#

	set(__default_val OFF)
	if (DEFINED args_DEFAULT)
		set(__default_val ${args_DEFAULT})
	elseif (DEFINED args_CALLBACK)
		cmake_language(CALL ${args_CALLBACK} "__default_val")
	else ()
		set(__default_val OFF)
		message(WARNING "Falling back to `OFF` as default value for options.")
	endif ()

	#

	if (PROJECT_IS_TOP_LEVEL)
		option(${args_VAR} "${args_NAME}" ${__default_val})
	else ()
		set(${args_VAR} ${__default_val} CACHE INTERNAL "${args_NAME}")
	endif ()

endfunction()