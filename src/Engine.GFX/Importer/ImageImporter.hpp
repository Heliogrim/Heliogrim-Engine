#pragma once

#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Resource/Importer/Importer.hpp>
#include <Engine.Resource/File.hpp>

namespace hg::engine::gfx {
	class KtxImporter :
		public res::Importer<
			std::pair<non_owning_rptr<assets::TextureAsset>, non_owning_rptr<assets::Image>>,
			ptr<void>
		> {
	public:
		using this_type = KtxImporter;
		using importer_type = res::Importer<
			std::pair<non_owning_rptr<assets::TextureAsset>, non_owning_rptr<assets::Image>>,
			ptr<void>
		>;

		using importer_type::import_result_type;

	public:
		KtxImporter();

		~KtxImporter() override;

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
