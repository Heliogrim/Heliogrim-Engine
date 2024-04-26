#pragma once

#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Resource/Importer/Importer.hpp>
#include <Engine.Resource/File.hpp>

namespace hg::engine::gfx {
	class FbxImporter :
		public res::Importer<Arci<assets::StaticGeometry>, ptr<void>> {
	public:
		using this_type = FbxImporter;
		using importer_type = res::Importer<Arci<assets::StaticGeometry>, ptr<void>>;

		using importer_type::import_result_type;

	public:
		FbxImporter();

		~FbxImporter() override;

	public:
		[[nodiscard]] bool canImport(cref<res::FileTypeId> typeId_, cref<::hg::fs::File> file_) const noexcept override;

	public:
		[[nodiscard]] descriptor_type descriptor() const noexcept override;

		[[nodiscard]] import_result_type import(
			cref<res::FileTypeId> typeId_,
			cref<::hg::fs::File> file_
		) const override;
	};
}
