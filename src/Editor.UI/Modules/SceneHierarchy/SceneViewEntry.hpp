#pragma once

#include <cassert>
#include <Engine.Common/Wrapper.hpp>

namespace ember {
    class Actor;
    class ActorComponent;
}

namespace ember::editor::ui {

    enum class SceneViewEntryType {
        eEmpty,
        eActor,
        eComponent
    };

    class SceneViewEntry {
    public:
        using this_type = SceneViewEntry;

    public:
        SceneViewEntry() = default;

        ~SceneViewEntry() = default;

    private:
        SceneViewEntryType _type;
        ptr<void> _target;

    public:
        [[nodiscard]] SceneViewEntryType type() const noexcept {
            return _type;
        }

        template <typename Type_>
        void storeTarget(const ptr<Type_> target_);

        template <typename Type_>
        [[nodiscard]] const ptr<Type_> target() const noexcept;

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept {
            return _type == other_._type && _target == other_._target;
        }

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept {
            return _type != other_._type || _target != other_._target;
        }
    };

    template <>
    inline void SceneViewEntry::storeTarget<Actor>(const ptr<Actor> target_) {
        _target = static_cast<ptr<void>>(target_);
        _type = SceneViewEntryType::eActor;
    }

    template <>
    inline const ptr<Actor> SceneViewEntry::target() const noexcept {
        assert(_type == SceneViewEntryType::eActor);
        return static_cast<const ptr<Actor>>(_target);
    }

    template <>
    inline void SceneViewEntry::storeTarget<ActorComponent>(const ptr<ActorComponent> target_) {
        _target = static_cast<ptr<void>>(target_);
        _type = SceneViewEntryType::eComponent;
    }

    template <>
    inline const ptr<ActorComponent> SceneViewEntry::target() const noexcept {
        assert(_type == SceneViewEntryType::eComponent);
        return static_cast<const ptr<ActorComponent>>(_target);
    }

    template <>
    inline void SceneViewEntry::storeTarget<void>(const ptr<void> target_) {
        _target = static_cast<ptr<void>>(target_);
        _type = SceneViewEntryType::eEmpty;
    }

    template <>
    inline const ptr<void> SceneViewEntry::target() const noexcept {
        //assert(_type == SceneViewEntryType::eEmpty);
        return static_cast<const ptr<void>>(_target);
    }
}

namespace std {
    template <>
    struct hash<ember::editor::ui::SceneViewEntry> {
        [[nodiscard]] size_t operator()(
            ::ember::cref<ember::editor::ui::SceneViewEntry> entry_
        ) const noexcept {
            return _STD hash<ember::u64> {}(reinterpret_cast<ember::u64>(entry_.target<void>()));
        }
    };
}
