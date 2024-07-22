#pragma once

#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Common/Concurrent/Future.hpp>
#include <Engine.Resource/File.hpp>
#include <Engine.Resource/Importer/Importer.hpp>

namespace hg::engine::gfx {
	class FontImporter :
		public res::Importer<Arci<assets::Font>, ptr<void>> {
	public:
		using this_type = FontImporter;
		using importer_type = res::Importer<Arci<assets::Font>, ptr<void>>;

		using importer_type::import_result_type;

	public:
		FontImporter();

		~FontImporter() override;

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
