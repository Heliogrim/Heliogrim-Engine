#include "ParallelSource.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Asserts/Todo.hpp>

using namespace hg::engine::resource;
using namespace hg;

ParallelSource::reference_type ParallelSource::operator=(mref<value_type> other_) noexcept {
	if (this != std::addressof(other_)) {
		_sources = std::move(other_._sources);
	}

	return *this;
}

bool ParallelSource::isAsync() const noexcept {
	return std::ranges::any_of(
		_sources,
		[](const auto& entry) {
			return entry->isAsync();
		}
	);
}

bool ParallelSource::isReady() const noexcept {
	return std::ranges::any_of(
		_sources,
		[](const auto& entry) {
			return entry->isReady();
		}
	);
}

bool ParallelSource::isReadable() const noexcept {
	return std::ranges::any_of(
		_sources,
		[](const auto& entry) {
			return entry->isReadable();
		}
	);
}

bool ParallelSource::isWritable() const noexcept {
	return std::ranges::any_of(
		_sources,
		[](const auto& entry) {
			return entry->isWritable();
		}
	);
}

streamsize ParallelSource::size() const {

	if (_sources.empty()) {
		return -1;
	}

	return _sources.front()->size();
}

streamsize ParallelSource::estimatedSize() const noexcept {

	if (_sources.empty()) {
		return -1;
	}

	return _sources.front()->estimatedSize();
}

bool ParallelSource::get(streamoff offset_, streamsize size_, ptr<void> dst_, ref<streamsize> actualSize_) {
	::hg::todo_panic();
}

hg::concurrent::future<Source::async_result_value> ParallelSource::get(streamoff offset_, streamsize size_) {
	::hg::todo_panic();
}

bool ParallelSource::write(streamoff offset_, streamsize size_, const ptr<void> src_, ref<streamsize> actualSize_) {
	::hg::todo_panic();
}

hg::concurrent::future<Source::async_write_result> ParallelSource::write(
	streamoff offset_,
	streamsize size_,
	const ptr<void> src_
) {
	::hg::todo_panic();
}
