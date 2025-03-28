#pragma once

#include <Engine.Asserts/Todo.hpp>

#include "../Node.hpp"
#include "../Parameter/ValueTypeNodeParameter.hpp"
#include "../Parameter/ConstantNodeParameter.hpp"
#include "../NodeOutput.hpp"

namespace hg::editor::gfx::graph::nodes {
	class ConstantInputNode :
		public Node {
	public:
		ConstantInputNode() :
			Node() {
			setup();
		}

	protected:
		void setup() {
			_parameter.push_back(make_sptr<ValueTypeNodeParameter>("type"sv, NodeLinkValueType::eUnknown));
			_parameter.push_back(make_sptr<ConstantNodeParameter>("constant"sv, NodeLinkValueType::eUnknown));
		}

	public:
		[[nodiscard]] string_view getNodeTypeName() const noexcept override {
			return "ConstantInputNode"sv;
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

	public:
		void regenerate() override {

			auto* const typeParam = getParameter("type"sv);
			const auto valueType = static_cast<ptr<ValueTypeNodeParameter>>(typeParam)->getValueType();

			auto* const storeParam = getParameter("constant"sv);
			auto* const stored = static_cast<ptr<ConstantNodeParameter>>(storeParam);

			if (stored->getValueType() != valueType) {
				// TODO: Cast stored value to new value type
				// TODO: Assign new value type to stored value
				::hg::todo_panic();
			}

			if (not _outputs.empty()) {
				getOutputSlot(0)->_spec.valueType = valueType;
				return;
			}

			_outputs.push_back(
				make_sptr<NodeOutput>(
					"input",
					NodeOutputSpec {
						valueType,
						NodeOutputValueAttributes { false }
					}
				)
			);
		}
	};
}
