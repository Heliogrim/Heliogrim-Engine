#include "StaticGeometry.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::assets;
using namespace hg;

StaticGeometry::StaticGeometry() :
	InheritMeta(clone(invalid_asset_guid), StaticGeometry::typeId, StringView {}, AssetReferenceUrl {}, AssetUrl {}),
	_sources(),
	_meshMaterials(),
	_vertexCount(0uLL),
	_indexCount(0uLL),
	_clusterCount(0uLL),
	_clusterDepth(0uLL) {}

StaticGeometry::StaticGeometry(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	mref<Vector<fs::Url>> sources_,
	mref<AutoArray<AssetGuid>> meshMaterials_
) :
	InheritMeta(::hg::move(guid_), typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_sources(std::move(sources_)),
	_meshMaterials(::hg::move(meshMaterials_)),
	_vertexCount(),
	_indexCount(),
	_clusterCount(),
	_clusterDepth() {}

cref<Vector<fs::Url>> StaticGeometry::sources() const noexcept {
	return _sources;
}

u16 StaticGeometry::getMeshMaterialCount() const noexcept {
	return _meshMaterials.size();
}

ref<const AutoArray<AssetGuid>> StaticGeometry::getMeshMaterials() const noexcept {
	return _meshMaterials;
}

void StaticGeometry::setMeshMaterial(u16 index_, mref<TypedAssetGuid<GfxMaterial>> materialGuid_) {
	::hg::assertd(getMeshMaterialCount() > index_ && _meshMaterials.size() > index_);
	_meshMaterials.at(index_) = ::hg::move(materialGuid_);
}
