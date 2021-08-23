#pragma once

#include <functional>
#include <Engine.Common/Wrapper.hpp>

#include "Shader.hpp"

namespace ember::engine::gfx {
    struct ShaderStorageKey {
        shader::ShaderType type;
        string name;

        [[nodiscard]] bool operator==(const ShaderStorageKey& other_) const noexcept;
    };
}

template <>
struct _STD hash<ember::engine::gfx::ShaderStorageKey> :
    public _STD hash<string> {

    size_t operator()(const ember::engine::gfx::ShaderStorageKey& key_) const noexcept {
        size_t s { static_cast<size_t>(key_.type) };
        size_t h = static_cast<const _STD hash<string>*>(this)->operator()(key_.name);

        ember::hash::hash_combine(s, h);

        return s;
    }
};

namespace ember::engine::gfx {

    class ShaderStorage {
    private:
        /**
         * Storage
         */
        _STD unordered_map<ShaderStorageKey, ptr<Shader>, _STD hash<ember::engine::gfx::ShaderStorageKey>>
        _storage;

    public:
        /**
         * Stores the given shader
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  shader_ The shader to store.
         */
        void store(ptr<Shader> shader_);

        /**
         * Determine if exists
         *
         * @author Julius
         * @date 18.12.2020
         *
         * @param  scope_ The scope.
         * @param  name_ The name.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool exists(const shader::ShaderType& scope_, const string& name_) const noexcept;

        /**
         * Gets a Shader instance
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  scope_ The scope.
         * @param  name_ The name.
         *
         * @returns A ptr&lt;Shader&gt;
         */
        ptr<Shader> get(const shader::ShaderType& scope_, const string& name_) const;

        /**
         * Gets a Shader instance
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  scope_ The scope.
         * @param  name_ The name.
         *
         * @returns A ptr&lt;Shader&gt;
         */
        ptr<Shader> get(const shader::ShaderType& scope_, const string& name_);

        /**
         * Create a new singleton instance of shader storage
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to a ShaderStorage.
         */
        static ShaderStorage& make() noexcept;

        /**
         * Gets the singleton instance
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to a ShaderStorage.
         */
        [[nodiscard]] static ref<ShaderStorage> get() noexcept;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 03.12.2020
         */
        static void destroy();

    private:
        /** The instance */
        static ShaderStorage* _instance;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        ShaderStorage() noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        ~ShaderStorage() noexcept = default;
    };

}
