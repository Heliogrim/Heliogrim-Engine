#pragma once

#include <map>
#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Source/Source.hpp"

namespace ember::engine::resource {

    namespace {

        struct key_value_pair {
            asset_guid guid;
            ptr<Source> source;
        };

        struct kv_hasher :
            private _STD hash<asset_guid> {
            [[nodiscard]] _STD size_t operator()(cref<key_value_pair> value_) const {
                return static_cast<cref<_STD hash<asset_guid>>>(*this)(value_.guid);
            }
        };

        struct kv_equal :
            private _STD equal_to<asset_guid> {
            [[nodiscard]] bool operator()(cref<key_value_pair> left_, cref<key_value_pair> right_) const {
                return static_cast<cref<_STD equal_to<asset_guid>>>(*this)(left_.guid, right_.guid);
            }
        };

        struct kv_less :
            private _STD less<asset_guid> {
            [[nodiscard]] bool operator()(cref<key_value_pair> left_, cref<key_value_pair> right_) const {
                return static_cast<cref<_STD less<asset_guid>>>(*this)(left_.guid, right_.guid);
            }
        };
    }

    class Locator {
    public:
        using value_type = Locator;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        Locator() noexcept = default;

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 12.09.2021
         */
        Locator(const_reference_type) = delete;

        /**
         * Move constructor
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param [in,out] other_ The other.
         */
        Locator(mref<value_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.09.2021
         */
        ~Locator() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Assignment operator
         *
         * @author Julius
         * @date 12.09.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type> other_) noexcept;

    private:
        using map_type = _STD map<asset_guid, ptr<Source>>;

        map_type _mapping;

    public:
        /**
         * Gets a source referenced with given unique identifier
         *
         * @author Julius
         * @date 09.09.2021
         *
         * @param  guid_ The Unique identifier to get.
         *
         * @returns A ptr&lt;Source&gt; or nullptr if failed
         */
        [[nodiscard]] ptr<Source> get(cref<asset_guid> guid_) const noexcept;

        /**
         * Inserts a new source referenced by a given unique identifier or replaces them
         *
         * @author Julius
         * @date 10.09.2021
         *
         * @param  guid_ Unique identifier.
         * @param  source_ Source for the.
         *
         * @returns A pointer to the previous source or nullptr if new
         */
        [[nodiscard]] ptr<Source> insert(cref<asset_guid> guid_, ptr<Source> source_) noexcept;
    };
}
