#pragma once
#include <Engine.Common/Wrapper.hpp>

#include <Engine.Assets/Database/AssetDatabase.hpp>

namespace ember {

    class AssetDatabase {
    public:
        using value_type = AssetDatabase;

    public:
        friend class Ember;

    private:
        AssetDatabase(managed<void> internal_);

    public:
        ~AssetDatabase();

    private:
        managed<void> _internal;

    public:
        ref<engine::assets::AssetDatabase> __tmp__internal() {
            return *static_cast<ptr<engine::assets::AssetDatabase>>(_internal.get());
        }
    };
}
