#pragma once

#include "../Node.hpp"
#include "../NodeLinkValueType.hpp"
#include "../Parameter/ValueTypeNodeParameter.hpp"
#include "../NodeInput.hpp"
#include "../NodeInputSpec.hpp"
#include "../NodeOutput.hpp"
#include "../NodeOutputSpec.hpp"

namespace hg::editor::gfx::graph::nodes {
    class SubGraphOutputNode :
        public Node {
    public:
        using this_type = SubGraphOutputNode;

    public:
        SubGraphOutputNode() :
            Node() {
            setup();
        }

        ~SubGraphOutputNode() override = default;

    protected:
        void setup() {
            _parameter.push_back(make_sptr<ValueTypeNodeParameter>("type"sv, NodeLinkValueType::eUnknown));
        }

    public:
        [[nodiscard]] string_view getNodeTypeName() const noexcept override {
            return "SubGraphOutputNode"sv;
        }

    public:
        [[nodiscard]] bool carriesInternalDependencies() const noexcept override {
            return false;
        }

        [[nodiscard]] bool carriesGraphDependencies() const noexcept override {
            return false;
        }

        [[nodiscard]] bool carriesExternalDependencies() const noexcept override {
            return false;
        }

    public:
        [[nodiscard]] bool isAssociative() const noexcept override {
            return false;
        }

        [[nodiscard]] bool isCommutative() const noexcept override {
            return false;
        }

    public:
        void regenerate() override {

            const auto typeParam = getParameter("type"sv);
            const auto valueType = static_cast<ptr<ValueTypeNodeParameter>>(typeParam)->getValueType();

            if (_inputs.empty()) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "output",
                        NodeInputSpec { valueType, NodeInputValueAttributes { false } }
                    )
                );
                return;
            }

            getInputSlot(0)->_spec.valueType = valueType;
        }
    };
}
