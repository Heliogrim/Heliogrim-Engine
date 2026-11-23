#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Filesystem;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
	class FontImporter :
		public res::Importer<Arci<assets::FontAsset>, ptr<void>> {
	public:
		using this_type = FontImporter;
		using importer_type = res::Importer<Arci<assets::FontAsset>, ptr<void>>;

		using importer_type::import_result_type;

	public:
		FontImporter();

		~FontImporter() override;

	public:
		[[nodiscard]] bool canImport(cref<res::FileTypeId> typeId_, cref<::hg::fs::File> file_) const noexcept override;

	public:
		[[nodiscard]] descriptor_type descriptor() const noexcept override;

		[[nodiscard]] import_result_type import(
			ref<const res::FileTypeId> typeId_,
			ref<const hg::fs::File> file_,
			mref<res::ImportDestination> destination_
		) const override;
	};
}
