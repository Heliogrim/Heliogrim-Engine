#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/Registry.hpp>
#include <Engine.Session/Session.hpp>

namespace ember::engine::ecs {

	class Subsystem {
	public:
		using value_type = Subsystem;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	public:
		Subsystem(cref<sptr<Session>> session_, ptr<registry> registry_ = &registry::make()) noexcept;

		~Subsystem() noexcept;

	private:
		void setupComponents();

	public:
		void setup();

		void schedule();

	private:
		sptr<Session> _session;

	public:
		[[nodiscard]] sptr<Session> session() const noexcept;

	private:
		ptr<registry> _registry;

	public:
		[[nodiscard]] ptr<registry> registry() const noexcept;
	};

}
