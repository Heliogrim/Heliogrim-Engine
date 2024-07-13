#include "RuntimeConfigInit.hpp"

#include <filesystem>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Config/Provider/RuntimeProvider.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg;

/**/

[[nodiscard]] static std::filesystem::path getRuntimeRootPath();

[[nodiscard]] static std::filesystem::path getRuntimeCachePath();

/**/

void editor::boot::initRuntimeConfig(
	cref<EditorEngine> engine_,
	ref<engine::Config> cfg_,
	ref<engine::cfg::RuntimeProvider> provider_
) {

	/**/

	{
		auto runtimePath = getRuntimeRootPath();
		std::ignore = cfg_.init(
			engine::cfg::RuntimeConfigProperty::eLocalBasePath,
			std::move(runtimePath).generic_string(),
			engine::cfg::query_provider_id(provider_)
		);
	}

}

/**/

std::filesystem::path getRuntimeRootPath() {
	// Question: How should we determine the root path, as we need the installation path for the runtime installation or editor, or the project path for a project setup?
	const auto cwd = std::filesystem::current_path();
	return clone(cwd);
}

std::filesystem::path getRuntimeCachePath() {
	return getRuntimeRootPath().append(R"(.cache)");
}
