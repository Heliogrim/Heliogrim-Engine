#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::engine::reflow {
    class StyleKeyStack {
    public:
        using this_type = StyleKeyStack;
        using key_type = AssocKey<string>;

        using set_type = CompactSet<key_type>;
        using stack_value_type = _STD pair<u64, set_type>;
        using stack_type = Stack<stack_value_type, Vector<stack_value_type>>;

    public:
        StyleKeyStack();

        ~StyleKeyStack() noexcept;

    private:
        stack_type _stack;
        u64 _depth;

    private:
    public:
        [[nodiscard]] cref<Vector<stack_value_type>> getSetList() const noexcept;

    public:
        [[nodiscard]] bool contains(cref<key_type> key_) const noexcept;

        void pushKey(cref<key_type> key_);

        [[deprecated]] void popKey(cref<key_type> key_);

    public:
        bool pushLayer();

        bool popLayer();

    public:
        void compress(_Inout_ ref<set_type> dst_) const noexcept;
    };
}
