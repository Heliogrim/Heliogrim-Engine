#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Reflect/__fwd.hpp>
#include <Engine.Serialization/Layout/__fwd.hpp>

#include "LayoutOptimizer.hpp"

namespace hg::engine::serialization {
    class LayoutManager {
    public:
        using this_type = LayoutManager;

    public:
        [[nodiscard]] static ref<LayoutManager> get() noexcept;

        static ref<LayoutManager> make();

        static void destroy();

    private:
        /**
         * Singleton Instance
         */
        static uptr<LayoutManager> _instance;

    private:
        LayoutManager();

    public:
        ~LayoutManager();

    private:
        void setup();

        void tidy();

    private:
        LayoutOptimizer _optimizer;

    public:
        void optimize(cref<sptr<DataLayoutBase>> layout_) const;

    private:
        sptr<DataLayoutBase> _nullLayout;

        RobinMap<type_id, sptr<DataLayoutBase>> _typeMapping;
        RobinMap<ptr<const HeliogrimClass>, sptr<DataLayoutBase>> _classMapping;
        RobinMap<AssocKey<string>, sptr<DataLayoutBase>> _symbolMapping;

    public:
        bool storeLayout(cref<string> symbol_, cref<sptr<DataLayoutBase>> layout_);

        bool storeLayout(string_view symbol_, cref<sptr<DataLayoutBase>> layout_);

        bool storeLayout(const ptr<const HeliogrimClass> class_, cref<sptr<DataLayoutBase>> layout_);

        bool storeLayout(cref<type_id> typeId_, cref<sptr<DataLayoutBase>> layout_);

        [[nodiscard]] cref<sptr<DataLayoutBase>> getLayout(cref<string> symbol_) const noexcept;

        [[nodiscard]] cref<sptr<DataLayoutBase>> getLayout(string_view symbol_) const noexcept;

        [[nodiscard]] cref<sptr<DataLayoutBase>> getLayout(const ptr<const HeliogrimClass> class_) const noexcept;

        [[nodiscard]] cref<sptr<DataLayoutBase>> getLayout(cref<type_id> typeId_) const noexcept;
    };
}
