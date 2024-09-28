function(vendor_assets_init)

	cmake_minimum_required(VERSION 3.25.1)
	include(FetchContent)

	name_to_external_alias(alias "assets")

	# Include Sub-Project
	get_external_include_path(vendor_inc_path "assets")
	get_external_source_path(vendor_src_path "assets")

	execute_process(
			COMMAND ssh -T git@git.neogc.net
			WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
			TIMEOUT 10
			RESULT_VARIABLE vendor_assets_git_ssh
			OUTPUT_QUIET
			ERROR_QUIET
	)

	set(vendor_git_repository "https://git.neogc.net/heliogrim-workgroup/heliogrim-engine-assets.git")
	if (NOT ${HELIOGRIM_ASSETS_FORCE_HTTPS} AND vendor_assets_git_ssh EQUAL 0)
		set(vendor_git_repository "git@git.neogc.net:heliogrim-workgroup/heliogrim-engine-assets.git")
		message(STATUS "Using ssh to access vendor assets repository, enabling support for upstream patching.")
	endif ()

	CachedFetchContent_Declare(
			assets
			GIT_REPOSITORY "${vendor_git_repository}"
			GIT_TAG develop
			GIT_SUBMODULES_RECURSE OFF
			GIT_SHALLOW ON
			GIT_PROGRESS ON
			GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
			SOURCE_DIR "${vendor_src_path}"
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			assets
	)

endfunction()

# Push to setup
add_managed_library(NAME assets CALL vendor_assets_init ORDER_INDEPENDENT ON)