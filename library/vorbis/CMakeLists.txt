function(library_vorbis_init)

	cmake_minimum_required(VERSION 3.27.7)
	include(FetchContent)

	name_to_external_alias(vorbis_alias "vorbis")
	name_to_external_alias(vorbisenc_alias "vorbisenc")
	name_to_external_alias(vorbisfile_alias "vorbisfile")

	#
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
	set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE INTERNAL "" FORCE)

	get_library_path(lib_base_dir)
	set(OGG_LIBRARY ${PROJECT_NAME}::External-ogg CACHE INTERNAL "" FORCE)
	set(OGG_INCLUDE_DIR "${lib_base_dir}/ogg/compile/include" CACHE INTERNAL "" FORCE)

	# Include Sub-Project
	get_external_include_path(lib_inc_path "vorbis")
	get_external_source_path(lib_src_path "vorbis")

	FetchContent_Declare(
			libvorbis
			GIT_REPOSITORY https://github.com/xiph/vorbis.git
			GIT_TAG v1.3.7
			SOURCE_DIR "${lib_src_path}"
			LOG_DOWNLOAD ON
			EXCLUDE_FROM_ALL
	)
	FetchContent_MakeAvailable(
			libvorbis
	)

	# Setup Library
	add_library(${PROJECT_NAME}::${vorbis_alias} ALIAS vorbis)
	add_library(${PROJECT_NAME}::${vorbisenc_alias} ALIAS vorbisenc)
	add_library(${PROJECT_NAME}::${vorbisfile_alias} ALIAS vorbisfile)

	install_includes(
			DIRECTORY "${lib_src_path}/include/"
			DESTINATION "${lib_inc_path}/"
	)

	export(TARGETS vorbis NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${vorbis_alias}/${vorbis_alias}-export.cmake)
	export(TARGETS vorbisenc NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${vorbisenc_alias}/${vorbisenc_alias}-export.cmake)
	export(TARGETS vorbisfile NAMESPACE External:: FILE ${PROJECT_BINARY_DIR}/cmake/${vorbisfile_alias}/${vorbisfile_alias}-export.cmake)

	# Deployment
	get_external_lib_path(lib_dist_path "vorbis")
	foreach (target vorbis vorbisenc vorbisfile)
		set_batch_target_properties(${target} ${lib_dist_path})
	endforeach ()

endfunction()

# Push to setup
add_managed_library(NAME libvorbis CALL library_vorbis_init DEPS libogg)