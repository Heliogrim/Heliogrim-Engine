#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Filesystem/__fwd.hpp>
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
		struct ProviderRepository {
			type_id typeId;
			uptr<IStorageFileRepository> repository;
		};

		Vector<ProviderRepository> _repositories;

		template <typename RepType_>
		[[nodiscard]] nmpt<RepType_> makeUniqueRepository(
			_Inout_ auto& repositories_,
			_In_ mref<hg::fs::File::path_type> basePath_
		);

	public:
		[[nodiscard]] Arci<LocalFileStorage> makeStorageObject(mref<hg::fs::Path> storagePath_) const noexcept;

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
