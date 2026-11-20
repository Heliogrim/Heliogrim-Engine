
# Setup
make_library_project(
		LIB_NAME Engine.Common
		MODULE
		LIBRARIES ${PROJECT_NAME}::Engine.Asserts ${PROJECT_NAME}::glm.Module
		MODULE_EXCLUDE
		# Names that clash with bundles' own under `using namespace hg;` once every importer sees them.
		Engine.Common/Buffer.hpp
		Engine.Common/String/Convert.hpp
		Engine.Common/String/Text.hpp
		Engine.Common/Math/Transform.hpp
		Engine.Common/Profiling/Profiler.hpp
		Engine.Common/StringLiterals.hpp
		Engine.Common/stdafx.h
)