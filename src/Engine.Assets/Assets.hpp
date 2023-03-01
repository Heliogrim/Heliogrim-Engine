#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace hg::engine::assets {
    class AssetDatabase;
    class AssetFactory;
}

namespace hg::engine {
    class Assets final :
        public core::CoreModule {
    public:
        using this_type = Assets;

        using AssetDatabase = ::hg::engine::assets::AssetDatabase;
        using AssetFactory = ::hg::engine::assets::AssetFactory;

    public:
        Assets(const non_owning_rptr<Engine> engine_);

        Assets(cref<this_type>) = delete;

        Assets(mref<this_type>) noexcept = delete;

        ~Assets() override;

    public:
        void setup() override;

        void start() override;

        void stop() override;

        void destroy() override;

    private:
        uptr<AssetDatabase> _database;
        uptr<AssetFactory> _factory;

    public:
        [[nodiscard]] const non_owning_rptr<AssetDatabase> getDatabase() const noexcept;

        [[nodiscard]] const non_owning_rptr<AssetFactory> getFactory() const noexcept;
    };
}
