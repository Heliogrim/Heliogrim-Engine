#include "DataLayoutDispatch.hpp"

#include "DataLayout.hpp"

using namespace ember::engine::serialization;
using namespace ember;

DataLayoutDispatch::DataLayoutDispatch(cref<DataLayoutBase> layout_) :
    _layout(layout_) {}

DataLayoutDispatch::~DataLayoutDispatch() = default;

void DataLayoutDispatch::load(ref<Archive> archive_, std::span<u8, ::std::dynamic_extent> dst_) const {

    const auto& defines { _layout.defines() };
    for (const auto& define : defines) {
        define->_load(archive_, *define, dst_);
    }

}

void DataLayoutDispatch::store(ref<Archive> archive_, std::span<u8, ::std::dynamic_extent> src_) const {

    const auto& defines { _layout.defines() };
    for (const auto& define : defines) {
        define->_store(archive_, *define, src_);
    }

}
