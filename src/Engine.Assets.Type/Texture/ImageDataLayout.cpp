#include "ImageAsset.hpp"

#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace hg::engine::assets;

namespace hg::engine::serialization {
	template <>
	void DataLayout<ImageAsset>::describe() {

		using namespace ::hg::engine::serialization::layout;

		const auto guidLayout = make_sptr<DataLayout<Guid>>();
		guidLayout->reflect().storeType<AssetGuid>();
		guidLayout->describe();

		const auto assetNameLayout = make_sptr<DataLayout<u8>>();
		assetNameLayout->reflect().storeType<u8>();
		assetNameLayout->describe();

		const auto urlLayout = make_sptr<DataLayout<fs::Url>>();
		urlLayout->reflect().storeType<fs::Url>();
		urlLayout->describe();

		/**/
		defineObject(offsetof(ImageAsset, _guid), guidLayout);
		defineValue<LayoutDefineValueType::eUInt64>(offsetof(ImageAsset, _type));
		defineSlice<string>(offsetof(ImageAsset, _assetName), assetNameLayout);

		/**/
		defineSlice<Vector<fs::Url>>(offsetof(ImageAsset, _sources), urlLayout);
	}
}
