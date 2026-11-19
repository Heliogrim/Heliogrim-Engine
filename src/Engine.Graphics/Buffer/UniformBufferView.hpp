#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "BufferLikeObject.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics {
    class UniformBufferView :
        public InheritBase<UniformBufferView> {
    public:
        using this_type = UniformBufferView;

    private:
        nmpt<const BufferLikeObject> _blo;

    public:
        [[nodiscard]] nmpt<const BufferLikeObject> object() const noexcept;

        void storeBuffer(mref<nmpt<const BufferLikeObject>> object_) noexcept;
    };
}
