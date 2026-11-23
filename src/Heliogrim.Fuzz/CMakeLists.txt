if (NOT HG_FUZZ)
	return()
endif ()

# One fuzz file per executable (duplicate LLVMFuzzerTestOneInput otherwise).
get_src_path(__src_path)
foreach (__fuzz LayoutFuzz StructuredFuzz)
name_to_target(__target Heliogrim.${__fuzz})
add_executable(${__target} EXCLUDE_FROM_ALL ${__fuzz}.cpp)
target_compile_definitions(${__target} PRIVATE HG_FIXTURE_DIR="${__src_path}/Heliogrim.Test/fixtures/serialization")
target_include_directories(${__target} PRIVATE ${__src_path})
target_include_directories(${__target} SYSTEM PUBLIC ${DEFAULT_INCLUDE_DIRECTORIES})
target_link_libraries(
		${__target}
		PRIVATE
		${PROJECT_NAME}::Engine.ACS
		${PROJECT_NAME}::Engine.Asserts
		${PROJECT_NAME}::Engine.Common
		${PROJECT_NAME}::Engine.Core
		${PROJECT_NAME}::Engine.Reflect
		${PROJECT_NAME}::Heliogrim
		${DEFAULT_LIBRARIES}
)
target_compile_definitions(${__target} PRIVATE ${DEFAULT_COMPILE_DEFINITIONS})
target_link_options(${__target} PRIVATE ${DEFAULT_LINKER_OPTIONS})
if (WIN32)
	# No /MD libFuzzer runtime exists (stock one is /MT, ASan dynamic needs /MD): use the in-tree mutation driver.
	target_sources(${__target} PRIVATE MutationDriver.cpp)
	target_compile_options(${__target} PRIVATE ${DEFAULT_COMPILE_OPTIONS} -fsanitize=address)
else ()
	target_compile_options(${__target} PRIVATE ${DEFAULT_COMPILE_OPTIONS} -fsanitize=fuzzer,address)
	target_link_options(${__target} PRIVATE -fsanitize=fuzzer,address)
endif ()
get_shared_dist_path(__dist_path)
set_batch_target_properties(${__target} ${__dist_path})
endforeach ()
