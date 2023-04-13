#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Widget/Widget.hpp"
#include "Widget/NullWidget.hpp"

namespace hg::engine::reflow {
    class Children :
        public Vector<sptr<Widget>> {};

    class NullChildren :
        public Children {};

    class SingleChildren :
        public Children {
    public:
        SingleChildren();

        sptr<Widget> setChild(cref<sptr<Widget>> nextChild_);

        sptr<Widget> getChild() const noexcept;

        void resetChild();
    };

    template <u32 Count_>
    class FixedChildren :
        public Children {
    public:
        FixedChildren() :
            Children() {
            for (u32 left = Count_; left > 0; --left) {
                push_back(NullWidget::instance());
            }
        }

        sptr<Widget> setChild(const u32 idx_, cref<sptr<Widget>> nextChild_) {
            assert(idx_ < Count_);
            sptr<Widget> tmp = (*this)[idx_];
            (*this)[idx_] = nextChild_;
            return tmp;
        }

        template <u32 Idx_> requires (Idx_ < Count_)
        sptr<Widget> setChild(cref<sptr<Widget>> nextChild_) {
            return setChild(Idx_, nextChild_);
        }

        sptr<Widget> getChild(const u32 idx_) const {
            assert(idx_ < Count_);
            return (*this)[idx_];
        }

        template <u32 Idx_> requires (Idx_ < Count_)
        sptr<Widget> getChild() const {
            return getChild(Idx_);
        }
    };

    extern const non_owning_rptr<const NullChildren> get_null_children() noexcept;
}
