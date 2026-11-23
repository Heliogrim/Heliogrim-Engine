include(${CMAKE_CURRENT_LIST_DIR}/../MakeLibraryProject.cmake)

function(_expect_true condition message)
	cmake_language(EVAL CODE "if (${condition})
else ()
	message(FATAL_ERROR \"FAILED: ${message}\")
endif ()")
endfunction()

# Sub-project bookkeeping only: the target half of the merge (`target_sources`, aliases) needs a configure, not `-P`.
set(PROJECT_NAME Heliogrim)

set_property(GLOBAL PROPERTY global_module_declared Root Other)
set_property(GLOBAL PROPERTY global_module_Root_headers "Root/A.hpp")
set_property(GLOBAL PROPERTY global_module_Root_prefixes Root)

set_property(GLOBAL PROPERTY global_module_sub_Sub_root Root)
set_property(GLOBAL PROPERTY global_module_sub_Sub_headers "Sub/B.hpp" "Sub/C.hpp")
set_property(GLOBAL PROPERTY global_module_sub_Sub_libraries "Heliogrim::Root" "Heliogrim::Other")

_record_module_sub(Sub root libraries)

get_property(headers GLOBAL PROPERTY global_module_Root_headers)
get_property(prefixes GLOBAL PROPERTY global_module_Root_prefixes)

_expect_true("root STREQUAL \"Root\"" "sub resolves to its root")
_expect_true("headers STREQUAL \"Root/A.hpp;Sub/B.hpp;Sub/C.hpp\"" "sub headers appended to the root module")
_expect_true("prefixes STREQUAL \"Root;Sub\"" "sub prefix appended to the root module")
_expect_true("libraries STREQUAL \"Heliogrim::Other\"" "root dropped from the sub's libraries")

# Module kind
set(base "${CMAKE_CURRENT_BINARY_DIR}/mlp-fixture")
file(REMOVE_RECURSE "${base}")
file(WRITE "${base}/Hand/CxxModule.ixx" "export module Hand;\n")
file(MAKE_DIRECTORY "${base}/Plain")

_module_kind("${base}/Hand" OFF kind)
_expect_true("kind STREQUAL \"HANDWRITTEN\"" "CxxModule.ixx marks a hand-written module")
_module_kind("${base}/Plain" ON kind)
_expect_true("kind STREQUAL \"GENERATED\"" "MODULE without CxxModule.ixx generates")
_module_kind("${base}/Plain" OFF kind)
_expect_true("kind STREQUAL \"\"" "neither: no module")

message(STATUS "MakeLibraryProject.Test passed")
