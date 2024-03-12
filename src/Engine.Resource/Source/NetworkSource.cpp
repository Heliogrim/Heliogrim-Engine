#include "NetworkSource.hpp"

#include <Engine.Asserts/Todo.hpp>

using namespace hg::engine::resource;
using namespace hg;

NetworkSource::reference_type NetworkSource::operator=(mref<value_type> other_) noexcept {
	if (this != std::addressof(other_)) {
		::hg::todo_panic();
	}

	return *this;
}

bool NetworkSource::isAsync() const noexcept {
	return false;
}

bool NetworkSource::isReady() const noexcept {
	::hg::todo_panic();
}

bool NetworkSource::isReadable() const noexcept {
	::hg::todo_panic();
}

bool NetworkSource::isWritable() const noexcept {
	::hg::todo_panic();
}

streamsize NetworkSource::size() const {
	return -1;
}

streamsize NetworkSource::estimatedSize() const noexcept {
	return -1;
}

bool NetworkSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {
	::hg::todo_panic();
}

hg::concurrent::future<Source::async_result_value> NetworkSource::get(streamoff offset_, streamsize size_) {
	::hg::todo_panic();
}

bool NetworkSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {
	::hg::todo_panic();
}

hg::concurrent::future<Source::async_write_result> NetworkSource::write(
	streamoff offset_,
	streamsize size_,
	const ptr<void> src_
) {
	::hg::todo_panic();
}
