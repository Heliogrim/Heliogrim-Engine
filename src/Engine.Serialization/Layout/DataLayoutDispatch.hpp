#pragma once

#include <span>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Types.hpp>

#include "__fwd.hpp"
#include "../Archive/__fwd.hpp"

namespace hg::engine::serialization {
    class DataLayoutDispatch {
    public:
        using this_type = DataLayoutDispatch;

    protected:
        DataLayoutDispatch(cref<DataLayoutBase> layout_);

    public:
        DataLayoutDispatch(cref<this_type>) = delete;

        DataLayoutDispatch(mref<this_type>) noexcept = delete;

        ~DataLayoutDispatch();

    private:
        cref<DataLayoutBase> _layout;

    public:
        void load(ref<Archive> archive_, _STD span<u8, _STD dynamic_extent> dst_) const;

        void store(ref<Archive> archive_, _STD span<u8, _STD dynamic_extent> src_) const;
    };
}
