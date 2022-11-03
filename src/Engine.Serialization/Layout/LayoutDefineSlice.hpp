#pragma once

#include <cassert>

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"
#include "DataLayout.hpp"

namespace ember::engine::serialization::layout {

    template <class Type_>
    concept HasDefIter = requires {
        typename Type_::iterator;
    };

    template <class Type_>
    concept HasAccIter = HasDefIter<Type_> && requires(Type_ obj_) {
        { obj_.begin() } -> std::same_as<typename Type_::iterator>;
        { obj_.end() } -> std::same_as<typename Type_::iterator>;
    };

    /**/

    template <class Type_>
    concept HasReserveFnc = requires(Type_ obj_, u64 count_) {
        { obj_.reserve(count_) };
    };

    /**/

    class LayoutDefineSlice final :
        public LayoutDefine {
    public:
        using this_type = LayoutDefineSlice;
        using type_traits = LayoutDefineValueTrait<LayoutDefineValueType::eSlice>;

    public:
        LayoutDefineSlice(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) :
            LayoutDefine(offset_, type_traits::size) {

            this_type::_load = nullptr;
            this_type::_store = nullptr;

            _sub = layout_;
        }

    public:
        template <HasAccIter ContainerType_>
        ref<this_type> withType() {
            this_type::_size = sizeof(ContainerType_);

            this_type::_load = this_type::loadImpl<ContainerType_>;
            this_type::_store = this_type::storeImpl<ContainerType_>;
            return *this;
        }

    protected:
        template <HasAccIter Type_>
        static ref<serialization::Archive> loadImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> dst_
        ) {

            const auto& layout { define_._sub };

            /**/

            u64 storedCount { ~0ui64 };
            archive_ >> storedCount;

            assert(storedCount != ~0ui64);

            /**/

            const ptr<Type_> container { reinterpret_cast<ptr<Type_>>(&dst_[define_._offset]) };

            // Warning: Experimental temporary speed-up solution
            if constexpr (
                _STD _Iterator_is_contiguous<typename Type_::iterator> &&
                HasReserveFnc<Type_> &&
                _STD is_trivial_v<typename Type_::value_type>
            ) {

                container->resize(storedCount);
                const auto layoutSize { static_cast<u64>(layout->size()) };

                for (const auto& entry : *container) {
                    const _STD span<u8, _STD dynamic_extent> view {
                        reinterpret_cast<ptr<u8>>(&const_cast<ref<typename Type_::value_type>>(entry)),
                        layoutSize
                    };
                    layout->dispatch().load(archive_, view);
                }

            } else {

                if constexpr (_STD _Iterator_is_contiguous<typename Type_::iterator> && HasReserveFnc<Type_>) {
                    container->reserve(storedCount);
                    // TODO: If `_STD is_trivial_v<Type_::value_type>` and contiguous memory, than we can memcpy directly and assume elements as `constructed`
                }

                auto tmp { calloc(1ui64, /*layout->size()*/sizeof(typename Type_::value_type)) };
                const _STD span<u8, _STD dynamic_extent> view {
                    static_cast<ptr<u8>>(tmp),
                    static_cast<u64>(layout->size()/*sizeof(typename Type_::value_type)*/)
                };

                _STD insert_iterator<Type_> inserter { *container, container->end() };

                for (s64 i { 0i64 }; i < storedCount; ++i) {

                    // Warning: Experimental temporary solution
                    if constexpr (not _STD is_trivial_v<typename Type_::value_type>) {
                        layout->reflect().constructInPlace(tmp);
                    }

                    layout->dispatch().load(archive_, view);
                    inserter = _STD move(*static_cast<ptr<typename Type_::value_type>>(tmp));

                    // Warning: Experimental temporary solution
                    if constexpr (not _STD is_trivial_v<typename Type_::value_type>) {
                        layout->reflect().destroyInPlace(tmp);
                    }
                }

                /**/

                free(tmp);
            }

            // TODO:

            return archive_;
        }

        template <HasAccIter Type_>
        static ref<serialization::Archive> storeImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> src_
        ) {

            const auto& layout { define_._sub };

            /**/

            const auto markCountPos { archive_.tell() };
            u64 count { 0ui64 };
            archive_ << count;

            /**/

            const auto* container { reinterpret_cast<ptr<Type_>>(&src_[define_._offset]) };

            auto begin { (*container).begin() };
            const auto end { (*container).end() };

            for (auto iter { _STD move(begin) }; iter != end; ++iter) {

                const _STD span<u8, _STD dynamic_extent> view {
                    reinterpret_cast<ptr<u8>>(
                        const_cast<ptr<typename Type_::value_type>>(
                            &const_cast<ref<typename Type_::value_type>>(*iter)
                        )
                    ),
                    sizeof(decltype(*iter))
                };
                layout->dispatch().store(archive_, view);
                // Warning: destructive calls from serialization are unsafe due to lack of strong typing and context ownership
                //layout->reflect()->destroyInPlace(&(*iter));

                ++count;
            }

            /**/

            const auto cursor { archive_.tell() };
            archive_.seek(markCountPos);
            archive_ << count;

            archive_.seek(cursor);

            // TODO:

            return archive_;
        }
    };

}
