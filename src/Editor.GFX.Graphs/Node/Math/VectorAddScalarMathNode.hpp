#pragma once

#include "../Node.hpp"
#include "../Parameter/ValueTypeNodeParameter.hpp"
#include "../NodeInputSpec.hpp"
#include "../NodeOutputSpec.hpp"

namespace hg::editor::gfx::graph::nodes {
    class VectorAddScalarMathNode :
        public Node {
    public:
        using this_type = VectorAddScalarMathNode;

    public:
        VectorAddScalarMathNode() :
            Node() {
            setup();
        }

        ~VectorAddScalarMathNode() override = default;

    protected:
        void setup() {
            _parameter.push_back(make_sptr<ValueTypeNodeParameter>("basetype"sv, NodeLinkValueType::f32));
            _parameter.push_back(make_sptr<ValueTypeNodeParameter>("vectortype"sv, NodeLinkValueType::f32vec3));
        }

    public:
        [[nodiscard]] string_view getNodeTypeName() const noexcept override {
            return "VectorAddScalarMathNode"sv;
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
            return true;
        }

        [[nodiscard]] bool isCommutative() const noexcept override {
            return true;
        }

    private:
        void regenerateVector(NodeLinkValueType vectorValueType_) {

            const auto slot = getInputSlot("vector"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "vector",
                        NodeInputSpec {
                            vectorValueType_,
                            NodeInputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = vectorValueType_;
        }

        void regenerateScalar(NodeLinkValueType scalarValueType_) {

            const auto slot = getInputSlot("scalar"sv);
            if (not slot) {
                _inputs.push_back(
                    make_sptr<NodeInput>(
                        "scalar",
                        NodeInputSpec {
                            scalarValueType_,
                            NodeInputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = scalarValueType_;
        }

        void regenerateSum(NodeLinkValueType vectorValueType_) {

            const auto slot = getInputSlot("sum"sv);
            if (not slot) {
                _outputs.push_back(
                    make_sptr<NodeOutput>(
                        "sum",
                        NodeOutputSpec {
                            vectorValueType_,
                            NodeOutputValueAttributes { true }
                        }
                    )
                );
                return;
            }

            slot.get()->_spec.valueType = vectorValueType_;
        }

    public:
        void regenerate() override {

            const auto baseTypeParam = getParameter("basetype"sv);
            const auto baseType = static_cast<ptr<ValueTypeNodeParameter>>(baseTypeParam)->getValueType();

            const auto vectorTypeParam = getParameter("vectortype"sv);
            const auto vectorType = static_cast<ptr<ValueTypeNodeParameter>>(vectorTypeParam)->getValueType();

            regenerateVector(vectorType);
            regenerateScalar(baseType);
            regenerateSum(vectorType);
        }
    };
}
