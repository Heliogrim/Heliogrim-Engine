#include "../GfxMaterial.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<GfxMaterial>::serialize(const GfxMaterial* const self_, mref<RecordScopedSlot> slot_) {

    auto root = slot_.intoStruct();

    Structure<Guid>::serialize(&self_->_guid, root.insertSlot<void>("__guid__"));
    root.insertSlot<u64>("__type__") << self_->_type.data;
    root.insertSlot<string>("name") << self_->_assetName;

    /**/

    if (self_->_albedo != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_albedo, root.insertSlot<void>("albedo"));
    }

    if (self_->_ao != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_ao, root.insertSlot<void>("ao"));
    }

    if (self_->_cavity != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_cavity, root.insertSlot<void>("cavity"));
    }

    if (self_->_displacement != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_displacement, root.insertSlot<void>("displacement"));
    }

    if (self_->_gloss != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_gloss, root.insertSlot<void>("gloss"));
    }

    if (self_->_normal != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_normal, root.insertSlot<void>("normal"));
    }

    if (self_->_roughness != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_roughness, root.insertSlot<void>("roughness"));
    }

    if (self_->_metalness != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_metalness, root.insertSlot<void>("metalness"));
    }

    if (self_->_specular != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_specular, root.insertSlot<void>("specular"));
    }

    if (self_->_alpha != invalid_asset_guid) {
        Structure<Guid>::serialize(&self_->_alpha, root.insertSlot<void>("alpha"));
    }
}

template <>
void access::Structure<GfxMaterial>::deserialize(GfxMaterial* const self_, mref<RecordScopedSlot> slot_) {

    const auto root = slot_.intoStruct();

    Structure<Guid>::deserialize(&self_->_guid, root.getSlot<void>("__guid__"));
    root.getSlot<u64>("__type__") >> self_->_type.data;
    root.getSlot<string>("name") >> self_->_assetName;

    /**/

    if (root.hasRecordSlot("albedo")) {
        Structure<Guid>::deserialize(&self_->_albedo, root.getSlot<void>("albedo"));
    } else {
        self_->_albedo = invalid_asset_guid;
    }

    if (root.hasRecordSlot("ao")) {
        Structure<Guid>::deserialize(&self_->_ao, root.getSlot<void>("ao"));
    } else {
        self_->_ao = invalid_asset_guid;
    }

    if (root.hasRecordSlot("cavity")) {
        Structure<Guid>::deserialize(&self_->_cavity, root.getSlot<void>("cavity"));
    } else {
        self_->_cavity = invalid_asset_guid;
    }

    if (root.hasRecordSlot("displacement")) {
        Structure<Guid>::deserialize(&self_->_displacement, root.getSlot<void>("displacement"));
    } else {
        self_->_displacement = invalid_asset_guid;
    }

    if (root.hasRecordSlot("gloss")) {
        Structure<Guid>::deserialize(&self_->_gloss, root.getSlot<void>("gloss"));
    } else {
        self_->_gloss = invalid_asset_guid;
    }

    if (root.hasRecordSlot("normal")) {
        Structure<Guid>::deserialize(&self_->_normal, root.getSlot<void>("normal"));
    } else {
        self_->_normal = invalid_asset_guid;
    }

    if (root.hasRecordSlot("roughness")) {
        Structure<Guid>::deserialize(&self_->_roughness, root.getSlot<void>("roughness"));
    } else {
        self_->_roughness = invalid_asset_guid;
    }

    if (root.hasRecordSlot("metalness")) {
        Structure<Guid>::deserialize(&self_->_metalness, root.getSlot<void>("metalness"));
    } else {
        self_->_metalness = invalid_asset_guid;
    }

    if (root.hasRecordSlot("specular")) {
        Structure<Guid>::deserialize(&self_->_specular, root.getSlot<void>("specular"));
    } else {
        self_->_specular = invalid_asset_guid;
    }

    if (root.hasRecordSlot("alpha")) {
        Structure<Guid>::deserialize(&self_->_alpha, root.getSlot<void>("alpha"));
    } else {
        self_->_alpha = invalid_asset_guid;
    }
}
