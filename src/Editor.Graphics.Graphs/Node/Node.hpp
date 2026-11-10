#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/String.hpp>

#include "__fwd.hpp"

namespace hg::editor::gfx::graph {
    class macro_novtable Node {
    public:
        using this_type = Node;

    public:
        virtual ~Node() = default;

    private:
        string _name;

    public:
        [[nodiscard]] cref<string> getName() const noexcept;

        void setName(string_view name_);

        [[nodiscard]] virtual string_view getNodeTypeName() const noexcept = 0;

    public:
        [[nodiscard]] virtual bool carriesInternalDependencies() const noexcept;

        [[nodiscard]] virtual bool carriesGraphDependencies() const noexcept;

        [[nodiscard]] virtual bool carriesExternalDependencies() const noexcept;

    public:
        [[nodiscard]] virtual bool isAssociative() const noexcept;

        [[nodiscard]] virtual bool isCommutative() const noexcept;

    protected:
        CompactArray<sptr<NodeInput>> _inputs;
        CompactArray<sptr<NodeOutput>> _outputs;

    public:
        [[nodiscard]] u64 countInputSlots() const noexcept;

        [[nodiscard]] sptr<NodeInput> getInputSlot(const u8 inputIndex_) const noexcept;

        [[nodiscard]] sptr<NodeInput> getInputSlot(string_view uniqueName_) const noexcept;

    public:
        [[nodiscard]] u64 countOutputSlots() const noexcept;

        [[nodiscard]] sptr<NodeOutput> getOutputSlot(const u8 outputIndex_) const noexcept;

        [[nodiscard]] sptr<NodeOutput> getOutputSlot(string_view uniqueName_) const noexcept;

    protected:
        CompactArray<sptr<NodeParameter>> _parameter;

    public:
        [[nodiscard]] u64 countParameters() const noexcept;

        [[nodiscard]] const non_owning_rptr<NodeParameter> getParameter(const u8 parameterIdx_) const noexcept;

        [[nodiscard]] const non_owning_rptr<NodeParameter> getParameter(string_view uniqueName_) const noexcept;

    public:
        virtual void regenerate() = 0;
    };
}
