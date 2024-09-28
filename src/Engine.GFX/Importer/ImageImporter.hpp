#pragma once

#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Resource/File.hpp>
#include <Engine.Resource/Importer/Importer.hpp>

namespace hg::engine::gfx {
	class KtxImporter :
		public res::Importer<
			std::pair<Arci<assets::TextureAsset>, Arci<assets::Image>>,
			ptr<void>
		> {
	public:
		using this_type = KtxImporter;
		using importer_type = res::Importer<
			std::pair<Arci<assets::TextureAsset>, Arci<assets::Image>>,
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
