#pragma once
#include "SoundImporter.hpp"

namespace hg::engine::sfx {
	class VorbisImporter :
		public SoundImporter {
	public:
		using base_type = SoundImporter;

		using value_type = VorbisImporter;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

		using base_type::descriptor_type;
		using base_type::import_result_type;

	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 13.09.2021
		 */
		VorbisImporter() noexcept = default;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 13.09.2021
		 */
		~VorbisImporter() noexcept override = default;

	public:
		[[nodiscard]] bool canImport(cref<res::FileTypeId> typeId_, cref<::hg::fs::File> file_) const noexcept override;

		[[nodiscard]] descriptor_type descriptor() const noexcept override;

		[[nodiscard]] import_result_type import(
			ref<const res::FileTypeId> typeId_,
			ref<const hg::fs::File> file_,
			mref<res::ImportDestination> destination_
		) const override;
	};
}