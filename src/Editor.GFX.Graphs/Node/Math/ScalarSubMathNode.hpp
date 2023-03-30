#pragma once

#include "../Node.hpp"
#include "../Parameter/ValueTypeNodeParameter.hpp"
#include "../NodeInputSpec.hpp"
#include "../NodeOutputSpec.hpp"

namespace hg::editor::gfx::graph::nodes {
    class ScalarSubMathNode :
        public Node {
    public:
        using this_type = ScalarSubMathNode;

    public:
        ScalarSubMathNode() :
            Node() {
            setup();
        }

        ~ScalarSubMathNode() override = default;

    protected:
        void setup() {
            _parameter.push_back(make_sptr<ValueTypeNodeParameter>("type"sv, NodeLinkValueType::f32));
        }

    public:
        [[nodiscard]] string_view getNodeTypeName() const noexcept override {
            return "ScalarSubMathNode"sv;
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

    private:
        void regenerateMinuend(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("minuend"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "minuend",
                        NodeInputSpec {
                            valueType_,
                            NodeInputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = valueType_;
        }

        void regenerateSubtrahend(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("subtrahend"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "subtrahend",
                        NodeInputSpec {
                            valueType_,
                            NodeInputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = valueType_;
        }

        void regenerateDifference(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("difference"sv);
            if (not slot) {
                _outputs.push_back(
                    make_sptr<NodeOutput>(
                        "difference",
                        NodeOutputSpec {
                            valueType_,
                            NodeOutputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = valueType_;
        }

    public:
        void regenerate() override {

            const auto typeParam = getParameter("type"sv);
            const auto valueType = static_cast<ptr<ValueTypeNodeParameter>>(typeParam)->getValueType();

            regenerateMinuend(valueType);
            regenerateSubtrahend(valueType);
            regenerateDifference(valueType);
        }
    };
}
