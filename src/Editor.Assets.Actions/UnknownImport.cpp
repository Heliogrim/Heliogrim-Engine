#include "UnknownImport.hpp"

#include <Engine.Common/__macro.hpp>

#if ENV_WIN
#include <Windows.h>
/**/
#include <ShObjIdl_core.h>
#endif

#include <Editor.Action/ActionManager.hpp>
#include <Editor.Action/Action/Import/AutoImportAction.hpp>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Platform/Platform.hpp>
#include <Heliogrim/Async/Await.hpp>

using namespace hg::editor::assets;
using namespace hg::engine::storage;
using namespace hg;

/**/

struct SelectFileFilter {
	string name;
	string pattern;
};

[[nodiscard]] static concurrent::future<Result<FileUrl, std::runtime_error>> requestPlatformSelect(mref<Vector<SelectFileFilter>> filter_);

[[nodiscard]] static std::wstring stringToWString(ref<const std::string> string_);

/**/

actions::UnknownImport::UnknownImport(mref<engine::storage::FileUrl> localFileSource_, mref<engine::storage::FileUrl> importPath_) :
	InheritMeta(),
	_localFileSource(::hg::move(localFileSource_)),
	_importPath(::hg::move(importPath_)) {}

actions::UnknownImport::~UnknownImport() = default;

bool actions::UnknownImport::isReversible() const noexcept {
	return false;
}

Result<void, std::runtime_error> actions::UnknownImport::apply() {

	setRunning();

	/**/

	if (_importPath.scheme() != FileScheme && _importPath.scheme() != FileProjectScheme) {
		IM_CORE_ERRORF(
			"Failed to import unknown data, cause the import destination schema (`{}`) is currently not support. Only `file` and `file+project` supported.",
			_importPath.scheme()
		);
		::hg::assertd(false);
		setFinished();
		return Unexpected { std::runtime_error { "Unsupported url schema for import destination." } };
	}

	auto destination = [](ref<const FileUrl> importPath_) {

		const auto& cfg = engine::Engine::getEngine()->getConfig();
		const auto& projectPath = cfg.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalAssetPath);

		::hg::assertrt(projectPath.has_value() && projectPath->has_value());

		const auto projectParent = engine::fs::normalized_path_from(**projectPath);
		::hg::assertrt(importPath_.path().equals(projectParent) || importPath_.path().isSubPathOf(projectParent));

		const auto relative = ::std::filesystem::relative(importPath_.path(), projectParent);
		return engine::res::ImportDestination { .virtualBasePath = engine::fs::Path { relative }, .virtualRemapScheme = FileProjectScheme };
	}(_importPath);

	/**/

	if (not std::filesystem::exists(_importPath.path())) {
		std::filesystem::create_directories(_importPath.path());
	}

	if (not std::filesystem::is_directory(_importPath.path())) {
		setFinished();
		return Unexpected { std::runtime_error { "Import path is not a directory." } };
	}

	auto importFile = FileUrl {
		clone(FileScheme), engine::fs::Path {
			_importPath.path() /
			static_cast<ref<const std::filesystem::path>>(_localFileSource.path()).filename()
		}
	};

	// TODO: Issue warning when we already have data at the importFile location.
	// TODO:	Add option to change the import file name and directory.

	if (not std::filesystem::copy_file(_localFileSource.path(), importFile.path(), std::filesystem::copy_options::none)) {
		setFinished();
		// Error: Will "throw" when used with double- or re-import operation
		// TODO: Check for process lock, open file, or any indexer blocking the copy'or'update filesystem operation.
		return Unexpected { std::runtime_error { "Failed to copy file to import directory." } };
	}

	/**/

	const auto module = engine::Engine::getEngine()->getModules().getSubModule(editor::ActionDepKey);
	::hg::assertrt(module != nullptr);

	static_cast<ptr<ActionManager>>(module.get())->apply(Arci<AutoImportAction>::create(::hg::move(importFile), ::hg::move(destination)));
	setFinished();

	return Expected<void> {};
}

Result<void, std::runtime_error> actions::UnknownImport::revoke() {
	::hg::todo_panic();
}

Result<void, std::runtime_error> actions::UnknownImport::undo() {
	::hg::todo_panic();
}

/**/

Result<Arci<actions::UnknownImport>, std::runtime_error> actions::makeUnknownWithExplorer(mref<engine::storage::FileUrl> importPath_) {

	if (importPath_.path().empty()) {
		const auto& cfg = engine::Engine::getEngine()->getConfig();
		const auto& projectPath = cfg.getTyped<String>(engine::cfg::ProjectConfigProperty::eLocalAssetPath);
		::hg::assertrt(projectPath.has_value() && projectPath->has_value());
		importPath_ = FileUrl { clone(FileScheme), engine::fs::Path { **projectPath } };
	}

	auto result = await(
		::hg::Future {
			requestPlatformSelect(
				{
					{ "EXR", "*.exr" },
					{ "FBX", "*.fbx" },
					{ "FLAC", "*.flac" },
					{ "JPEG", "*.jpg;*.jpeg" },
					{ "KTX", "*.ktx;*.ktx2" },
					{ "OBJ Model", "*.obj" },
					{ "OGG", "*.ogg" },
					{ "PNG", "*.png" },
					{ "True-Type Font", "*.ttf" },
					{ "Vorbis", "*.vorbis" },
					{ "Wave Audio", "*.wav" }
				}
			)
		}
	);
	return ::hg::move(result).map(
		[&importPath_](mref<FileUrl> fileUrl_) {
			return Arci<actions::UnknownImport>::create(::hg::move(fileUrl_), ::hg::move(importPath_));
		}
	);
}

/**/

#if ENV_WIN

concurrent::future<Result<FileUrl, std::runtime_error>> requestPlatformSelect(mref<Vector<SelectFileFilter>> filter_) {

	auto promise = concurrent::promise<Result<FileUrl, std::runtime_error>> {
		[filters = ::hg::move(filter_)]() -> Result<FileUrl, std::runtime_error> {

			::IFileOpenDialog* winFileDialog = nullptr;
			auto result = ::CoCreateInstance(
				::CLSID_FileOpenDialog,
				nullptr,
				CLSCTX_ALL,
				::IID_IFileOpenDialog,
				reinterpret_cast<void**>(&winFileDialog)
			);
			if (result != HRESULT_FROM_WIN32(ERROR_SUCCESS)) {
				return Unexpected { std::runtime_error { "Failed to instantiate file dialog." } };
			}

			auto dialog = UniquePtr<::IFileOpenDialog, void(*)(::IFileOpenDialog*)> {
				std::exchange(winFileDialog, nullptr), [](::IFileOpenDialog* obj_) { obj_->Release(); }
			};

			/**/

			auto filterStrings = Vector<std::wstring> {};
			filterStrings.resize((filters.size() + 1uLL) * 2uLL);
			auto filter = Vector<::COMDLG_FILTERSPEC> {};
			filter.resize(filters.size() + 1uLL);

			for (auto idx = 0uL; idx < filters.size(); ++idx) {

				filterStrings[idx * 2] = stringToWString(filters[idx].name);
				filter[idx].pszName = filterStrings[idx * 2].c_str();

				filterStrings[idx * 2 + 1] = stringToWString(filters[idx].pattern);
				filter[idx].pszSpec = filterStrings[idx * 2 + 1].c_str();
			}

			filterStrings[filters.size() * 2] = L"All Files";
			filter[filters.size()].pszName = filterStrings[filters.size() * 2].c_str();

			filterStrings[filters.size() * 2 + 1uLL] = L"*.*";
			filter[filters.size()].pszSpec = filterStrings[filters.size() * 2 + 1uLL].c_str();

			result = dialog->SetFileTypes(static_cast<u32>(filter.size()), filter.data());

			if (result != HRESULT_FROM_WIN32(ERROR_SUCCESS)) {
				return Unexpected { std::runtime_error { "Failed to set file type filters." } };
			}

			/**/

			result = dialog->Show(nullptr);

			if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
				return Unexpected { std::runtime_error { "File dialog was canceled." } };
			} else if (result != HRESULT_FROM_WIN32(ERROR_SUCCESS)) {
				return Unexpected { std::runtime_error { "Failed to open configured file dialog." } };
			}

			/**/

			::IShellItem* reportItem = nullptr;
			result = dialog->GetResult(&reportItem);

			if (result != HRESULT_FROM_WIN32(ERROR_SUCCESS)) {
				return Unexpected { std::runtime_error { "Failed to get shell item from file dialog." } };
			}

			auto item = UniquePtr<::IShellItem, void(*)(::IShellItem*)> {
				std::exchange(reportItem, nullptr), [](::IShellItem* obj_) { obj_->Release(); }
			};

			/**/

			wchar_t* reportPath = nullptr;
			result = item->GetDisplayName(::SIGDN_FILESYSPATH, &reportPath);

			if (result != HRESULT_FROM_WIN32(ERROR_SUCCESS)) {
				return Unexpected { std::runtime_error { "Failed to get file path from shell item." } };
			}

			/**/

			auto standard = std::filesystem::path { reportPath };
			auto report = FileUrl { clone(FileScheme), engine::fs::Path { ::hg::move(standard) } };

			return Expected(::hg::move(report));
		}
	};
	auto future = promise.get();

	/**/

	const auto engine = engine::Engine::getEngine();
	const auto platform = engine->getPlatform();

	platform->platformQueue().try_push(::hg::move(promise));
	return future;
}

std::wstring stringToWString(ref<const std::string> string_) {
	auto result = std::wstring {};
	result.resize(string_.size());

	auto size = std::size_t {};
	::mbstowcs_s(&size, result.data(), result.size() + 1uLL, string_.c_str(), string_.size());
	return result;
}

#endif
