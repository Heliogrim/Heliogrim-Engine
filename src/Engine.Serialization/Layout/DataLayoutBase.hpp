#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "DataLayoutReflect.hpp"
#include "DataLayoutDispatch.hpp"

namespace hg::engine::serialization {
    class macro_novtable DataLayoutBase :
        public DataLayoutReflect,
        public DataLayoutDispatch {
    public:
        template <typename>
        friend class DataLayout;

    public:
        using this_type = DataLayoutBase;

    private:
        DataLayoutBase(const bool hasReflectedClass_);

    public:
        DataLayoutBase(cref<this_type>) = delete;

        DataLayoutBase(mref<this_type>) noexcept = delete;

        virtual ~DataLayoutBase();

    public:
        void tidy();

    public:
        cref<DataLayoutReflect> reflect() const noexcept;

        ref<DataLayoutReflect> reflect() noexcept;

        cref<DataLayoutDispatch> dispatch() const noexcept;

    private:
        const bool _hasReflectedClass;

    public:
        [[nodiscard]] bool hasReflectedClass() const noexcept;

    public:
        [[nodiscard]] bool hasDynamicDefines() const noexcept;

        [[nodiscard]] bool justConstantDefines() const noexcept;

        [[nodiscard]] s64 size() const noexcept;

    private:
        Vector<ptr<layout::LayoutDefine>> _defines;

    public:
        [[nodiscard]] cref<decltype(_defines)> defines() const noexcept;

    private:
        void define(mref<ptr<layout::LayoutDefine>> define_);

    public:
        virtual void describe() = 0;
    };
}
