#pragma once

#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Resource/File.hpp>

#include "../IStorageProvider.hpp"

namespace hg::engine::storage::system {
	class LocalFileStorage;
	class IStorageFileRepository;
	class CacheFileSystemRepository;
	class EditorReadOnlyRepository;
	class ProjectFileSystemRepository;
	class RuntimeFileSystemRepository;
}

namespace hg::engine::storage::system {
	class LocalFileSystemProvider final :
		public IStorageProvider {
	public:
		using this_type = LocalFileSystemProvider;

	public:
		LocalFileSystemProvider() noexcept;

		~LocalFileSystemProvider() noexcept override;

	private:
		Vector<uptr<IStorageFileRepository>> _repositories;

	public:
		[[nodiscard]] Arci<LocalFileStorage> makeStorageObject() const noexcept;

	public:
		[[nodiscard]] nmpt<CacheFileSystemRepository> makeCacheRepository(
			mref<::hg::fs::File::path_type> basePath_
		);

		[[nodiscard]] nmpt<ProjectFileSystemRepository> makeProjectRepository(
			mref<::hg::fs::File::path_type> basePath_
		);

		[[nodiscard]] nmpt<RuntimeFileSystemRepository> makeRuntimeRepository(
			mref<::hg::fs::File::path_type> basePath_
		);

		[[nodiscard]] nmpt<EditorReadOnlyRepository> makeEditorRepository(
			mref<::hg::fs::File::path_type> basePath_
		);
	};
}
