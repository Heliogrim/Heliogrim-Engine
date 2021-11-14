#include "StagedComponentSubmit.hpp"

using namespace ember::engine::ecs::subsystem;
using namespace ember;

constexpr StagedComponentSubmit::StagedComponentSubmit() noexcept :
    _pageIdx(0),
    _data(nullptr) {}

constexpr StagedComponentSubmit::StagedComponentSubmit(u32 pageIdx_, non_owning_rptr<void> data_) noexcept :
    _pageIdx(pageIdx_),
    _data(data_) {}

StagedComponentSubmit::StagedComponentSubmit(mref<StagedComponentSubmit> other_) noexcept :
    _pageIdx(other_._pageIdx),
    _data(other_._data) {}

StagedComponentSubmit::StagedComponentSubmit(cref<StagedComponentSubmit> other_) noexcept :
    _pageIdx(other_._pageIdx),
    _data(other_._data) {}

StagedComponentSubmit::~StagedComponentSubmit() noexcept = default;

ref<StagedComponentSubmit> StagedComponentSubmit::operator=(mref<StagedComponentSubmit> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _pageIdx = other_._pageIdx;
        _data = other_._data;
    }

    return *this;
}

ref<StagedComponentSubmit> StagedComponentSubmit::operator=(cref<StagedComponentSubmit> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _pageIdx = other_._pageIdx;
        _data = other_._data;
    }

    return *this;
}

u32 StagedComponentSubmit::pageIdx() const noexcept {
    return _pageIdx;
}

const non_owning_rptr<void> StagedComponentSubmit::data() const noexcept {
    return _data;
}
