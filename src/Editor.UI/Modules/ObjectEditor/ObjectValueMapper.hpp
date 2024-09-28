#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
	template <typename ObjectType_>
	class ObjectValueMapper;
}

namespace hg::editor::ui {
	class ObjectValueMapperBase {
	public:
		template <typename ObjectType_>
		friend class ::hg::editor::ui::ObjectValueMapper;

	public:
		using this_type = ObjectValueMapperBase;

	private:
		ObjectValueMapperBase() = default;

	public:
		virtual ~ObjectValueMapperBase() = default;

	public:
		virtual void update(cref<sptr<engine::reflow::VerticalPanel>> parent_, const ptr<void> obj_) = 0;

		virtual void build(cref<sptr<engine::reflow::VerticalPanel>> parent_) = 0;

		virtual void cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) = 0;
	};

	template <typename ObjectType_>
	class ObjectValueMapper :
		public ObjectValueMapperBase {
	public:
		using this_type = ObjectValueMapper<ObjectType_>;

	public:
		ObjectValueMapper() = default;

		~ObjectValueMapper() override = default;

	public:
		void update(cref<sptr<engine::reflow::VerticalPanel>> parent_, const ptr<void> obj_) override;

		void build(cref<sptr<engine::reflow::VerticalPanel>> parent_) override;

		void cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) override;
	};

	template <>
	inline void ObjectValueMapper<void>::update(
		cref<sptr<engine::reflow::VerticalPanel>> parent_,
		const ptr<void> obj_
	) {}

	template <>
	inline void ObjectValueMapper<void>::build(cref<sptr<engine::reflow::VerticalPanel>> parent_) {}

	template <>
	inline void ObjectValueMapper<void>::cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) {}
}
