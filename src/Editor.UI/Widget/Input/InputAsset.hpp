#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>
#include <Engine.Reflow.Uikit/Atom/Image.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Exp/Button.hpp>

#include "InputText.hpp"

namespace hg::editor::ui {
	class InputAsset :
		public engine::reflow::Input<AssetGuid> {
	public:
		using this_type = InputAsset;

	public:
		InputAsset();

		~InputAsset() override;

	public:
		void setup();

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		engine::reflow::EventResponse invokeOnDrop(ref<const engine::reflow::DragDropEvent> event_) override;

		engine::reflow::EventResponse invokeOnDragOver(ref<const engine::reflow::DragDropEvent> event_) override;

	protected:
		sptr<engine::reflow::uikit::Image> _preview;
		sptr<InputText> _input;
		sptr<engine::reflow::uikit::Button> _search;
		sptr<engine::reflow::uikit::Button> _reset;
		sptr<engine::reflow::uikit::VerticalLayout> _content;

		engine::reflow::SingleChildren _children;

	public:
		//[[nodiscard]] const ptr<const engine::reflow::Children> children() const override;

	public:
		void render(
			const ptr<engine::reflow::ReflowCommandBuffer> cmd_
		) override;

	public:
		engine::reflow::PrefetchSizing prefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowState> state_
		) const override;

		engine::reflow::PassPrefetchSizing passPrefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowPassState> passState_
		) const override;

		void computeSizing(engine::reflow::ReflowAxis axis_, ref<const engine::reflow::ReflowPassState> passState_) override;

		void applyLayout(ref<engine::reflow::ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;

	public:
		void enable() override;

		void disable() override;

		void markAsDirty() override;

		void markAsPristine() override;

		void markAsTouched() override;

		void markAsUntouched() override;

		void reset() override;

		void updateValueAndValidity(const bool propagate_, const bool emit_) override;

	protected:
		AssetGuid _value;

	public:
		[[nodiscard]] input_type value() const noexcept override;

		void setValue(cref<AssetGuid> assetGuid_);

		/**/

	protected:
		Vector<asset_type_id> _acceptedTypes;

	public:
		[[nodiscard]] cref<Vector<asset_type_id>> acceptedTypes() const noexcept;

		void addAcceptedType(asset_type_id typeId_);

		void dropAcceptedType(asset_type_id typeId_);

	private:
	public:
		std::function<void(input_type)> _callback = nullptr;
	};
}
