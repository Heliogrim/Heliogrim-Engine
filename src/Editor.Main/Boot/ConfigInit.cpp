#include "ConfigInit.hpp"

#include <filesystem>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Config/Provider/EditorProvider.hpp>
#include <Engine.Config/Provider/ProjectProvider.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <tl/expected.hpp>

using namespace hg;

/**/

[[nodiscard]] static std::filesystem::path getProjectRootPath();

[[nodiscard]] static std::filesystem::path getProjectCachePath();

[[nodiscard]] static std::filesystem::path getEditorInstallPath();

[[nodiscard]] static std::filesystem::path getEditorUserCachePath();

[[nodiscard]] static std::filesystem::path getProcessModulePath();

/**/

void editor::boot::initProjectConfig(ref<engine::Config> cfg_, ref<engine::cfg::ProjectProvider> provider_) {

	/**/

	{
		auto projectPath = getProjectRootPath();
		std::ignore = cfg_.init(
			engine::cfg::ProjectConfigProperty::eLocalBasePath,
			std::move(projectPath).generic_string(),
			engine::cfg::query_provider_id(provider_)
		);
	}

	/**/

	{
		auto projectCachePath = getProjectCachePath();
		std::ignore = cfg_.init(
			engine::cfg::ProjectConfigProperty::eLocalCachePath,
			std::move(projectCachePath).generic_string(),
			query_provider_id(provider_)
		);
	}

}

void editor::boot::initEditorConfig(ref<engine::Config> cfg_, ref<cfg::EditorProvider> provider_) {

	/**/

	{
		auto userCachePath = getEditorUserCachePath();
		std::ignore = cfg_.init(
			engine::cfg::EditorConfigProperty::eLocalCachePath,
			std::move(userCachePath).generic_string(),
			query_provider_id(provider_)
		);
	}

	/**/

	{
		auto editorPath = getEditorInstallPath();
		std::ignore = cfg_.init(
			engine::cfg::EditorConfigProperty::eLocalEditorPath,
			std::move(editorPath).generic_string(),
			query_provider_id(provider_)
		);
	}

}

/**/

#if ENV_WIN

struct RegReadError {};

static tl::expected<std::filesystem::path, RegReadError> readPathFromReg(std::wstring_view keyPath_);

/**/

std::filesystem::path getProjectRootPath() {
	// Question: How should we determine the root path, as we need the installation path for the runtime installation or editor, or the project path for a project setup?
	const auto cwd = std::filesystem::current_path();
	return clone(cwd);
}

std::filesystem::path getProjectCachePath() {
	return getProjectRootPath().append(R"(.cache)");
}

std::filesystem::path getEditorInstallPath() {
	return getProcessModulePath();
	//return getCommonInstallPath().value<>().append(R"(Heliogrim\Editor)");
}

std::filesystem::path getEditorUserCachePath() {
	auto common = readPathFromReg(
		L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Local AppData"
	);
	return common.map(
		[](std::filesystem::path& known_) {
			return known_.append(R"(Heliogrim)");
		}
	).value_or(getProjectRootPath()).append(R"(.cache)");
}

/**/

#include <Windows.h>
// Note: <psapi> depends on <windows.h> header
#include <psapi.h>

std::filesystem::path getProcessModulePath() {
	Array<_::byte, 256> tmpData {};
	auto length = GetModuleFileNameW(
		NULL,
		std::bit_cast<wchar_t*>(tmpData.data()),
		static_cast<DWORD>(tmpData.size() / sizeof(wchar_t))
	);
	::hg::assertrt(GetLastError() != ERROR_INSUFFICIENT_BUFFER);
	return std::filesystem::path {
		std::wstring_view { std::bit_cast<wchar_t*>(tmpData.data()), length }
	}.parent_path();
}

[[nodiscard]] static decltype(auto) getCommonInstallPath() {
	return readPathFromReg(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProgramFilesDir");
}

tl::expected<std::filesystem::path, RegReadError> readPathFromReg(std::wstring_view keyPath_) {

	constexpr auto checkPattern = std::wstring_view { L"HKEY_LOCAL_MACHINE" };
	const auto keyRootIdx = keyPath_.find(checkPattern);
	if (keyRootIdx == std::wstring::npos) {
		return tl::make_unexpected(RegReadError {});
	}

	/**/

	constexpr auto delimPattern = std::wstring_view { L"/" };
	const auto keyNameIdx = keyPath_.find_last_of(delimPattern);
	if (keyNameIdx == std::wstring::npos) {
		return tl::make_unexpected(RegReadError {});
	}

	/**/

	const auto keySubPath = keyPath_.substr(
		keyRootIdx + checkPattern.size(),
		keyNameIdx - (keyRootIdx + checkPattern.size())
	);
	const auto keyName = keySubPath.substr(keyNameIdx + 1uLL);

	/**/

	Array<_::byte, 256> tmpRegData {};
	DWORD tmpRegDataSize {};

	const auto result = RegGetValueW(
		HKEY_LOCAL_MACHINE,
		keySubPath.data(),
		keyName.data(),
		REG_SZ,
		nullptr,
		tmpRegData.data(),
		&tmpRegDataSize
	);

	/**/

	if (result != ERROR_SUCCESS) {
		return tl::make_unexpected(RegReadError {});
	}

	/**/

	const auto regDataView = std::wstring_view {
		std::bit_cast<wchar_t*>(tmpRegData.data()), tmpRegDataSize / sizeof(wchar_t)
	};
	return std::filesystem::path { regDataView };
}
#endif
