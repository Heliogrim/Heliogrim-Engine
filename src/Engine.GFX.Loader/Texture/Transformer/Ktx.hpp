#pragma once

#include <Engine.GFX/Texture/SparseTextureView.hpp>

#include "../TextureTransformer.hpp"

namespace hg::engine::gfx {
	class Device;
}

namespace hg::engine::gfx::loader::transformer {
	void convertKtx(
		const non_owning_rptr<const assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureLoadOptions options_
	);

	extern void convertKtx10Gli(
		const non_owning_rptr<const engine::assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureLoadOptions options_
	);

	extern void convertKtx20(
		const non_owning_rptr<const engine::assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureLoadOptions options_
	);

	/**/

	void convertKtxPartial(
		const non_owning_rptr<const assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureStreamOptions options_
	);

	extern void convertKtx20Partial(
		const non_owning_rptr<const engine::assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureStreamOptions options_
	);

	extern void unloadPartialTmp(
		const non_owning_rptr<const engine::assets::TextureAsset> asset_,
		mref<storage::AccessBlobReadonly> src_,
		const non_owning_rptr<SparseTextureView> dst_,
		cref<sptr<Device>> device_,
		const TextureStreamOptions options_
	);
}
