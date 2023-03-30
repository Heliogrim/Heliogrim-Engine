#pragma once

#include "../Node.hpp"
#include "../Parameter/ValueTypeNodeParameter.hpp"
#include "../NodeInputSpec.hpp"
#include "../NodeOutputSpec.hpp"

namespace hg::editor::gfx::graph::nodes {
    class ScalarPowMathNode :
        public Node {
    public:
        using this_type = ScalarPowMathNode;

    public:
        ScalarPowMathNode() :
            Node() {
            setup();
        }

        ~ScalarPowMathNode() override = default;

    protected:
        void setup() {
            _parameter.push_back(make_sptr<ValueTypeNodeParameter>("type"sv, NodeLinkValueType::f32));
        }

    public:
        [[nodiscard]] string_view getNodeTypeName() const noexcept override {
            return "ScalarPowMathNode"sv;
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
        void regenerateBase(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("base"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "base",
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

        void regenerateExponent(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("exponent"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "exponent",
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

        void regeneratePotency(NodeLinkValueType valueType_) {

            const auto slot = getInputSlot("potency"sv);
            if (not slot) {
                _outputs.push_back(
                    make_sptr<NodeOutput>(
                        "potency",
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

            regenerateBase(valueType);
            regenerateExponent(valueType);
            regeneratePotency(valueType);
        }
    };
}
