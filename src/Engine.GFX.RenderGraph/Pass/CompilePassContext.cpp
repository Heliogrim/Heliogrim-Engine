#include "CompilePassContext.hpp"

#include <memory>
#include <memory_resource>
#include <Engine.Common/Wrapper.hpp>

using namespace hg::engine::render::graph;
using namespace hg;

/**/

class proxy_new_delete_resource final :
	public std::pmr::memory_resource {
public:
	explicit proxy_new_delete_resource(
		ref<std::pmr::memory_resource> base_ = *std::pmr::new_delete_resource()
	) noexcept :
		std::pmr::memory_resource(),
		_base(std::addressof(base_)) {}

	~proxy_new_delete_resource() override = default;

private:
	ptr<std::pmr::memory_resource> _base;

protected:
	void* do_allocate(const size_t bytes_, const size_t align_) override {
		return _base->allocate(bytes_, align_);
	}

	void do_deallocate(void* ptr_, const size_t bytes_, const size_t align_) override {
		_base->deallocate(ptr_, bytes_, align_);
	}

	[[nodiscard]] bool do_is_equal(const memory_resource& other_) const noexcept override {
		return this == std::addressof(other_);
	}
};

/**/

CompilePassContext::CompilePassContext() noexcept :
	_allocator(make_uptr<proxy_new_delete_resource>()) {}

nmpt<const GraphNodeAllocator> CompilePassContext::getGraphNodeAllocator() const noexcept {
	return &_allocator;
}

nmpt<const engine::gfx::scene::SceneView> CompilePassContext::getSceneView() const noexcept {
	return {};
}
