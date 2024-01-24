#include "Stopwatch.hpp"

#include <thread>

#include "Profiler.hpp"
#include "../Types.hpp"

using namespace hg::profiling;
using namespace hg;

Stopwatch::Stopwatch() noexcept :
    _name(),
    _stopped(true),
    _timestamp() {}

Stopwatch::Stopwatch(const char* name_, const bool stopped_) :
    _name(name_),
    _stopped(true) {
    if (!stopped_) {
        start();
    }
}

Stopwatch::Stopwatch(cref<string_view> name_, const bool stopped_) :
    _name(name_),
    _stopped(true) {
    if (!stopped_) {
        start();
    }
}

Stopwatch::~Stopwatch() noexcept = default;

void Stopwatch::start() noexcept {

    if (!_stopped) {
        return;
    }

    _stopped = false;
    _timestamp = std::chrono::high_resolution_clock::now();
}

void Stopwatch::stop() noexcept {

    const auto endTimestamp = std::chrono::high_resolution_clock::now();

    const u64 start = std::chrono::time_point_cast<std::chrono::microseconds>(_timestamp).time_since_epoch().count();
    const u64 end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimestamp).time_since_epoch().count();

    const u64 tid = std::hash<std::thread::id> {}(std::this_thread::get_id());

    auto* profiler = Profiler::get();
    if (profiler) {
        *profiler << ProfileResult { _name, start, end, tid };
    }

    _stopped = true;
}

const string_view Stopwatch::name() const noexcept {
    return _name;
}

ref<string> Stopwatch::name() noexcept {
    return _name;
}

ScopedStopwatch::ScopedStopwatch(const char* name_) noexcept :
    Stopwatch(name_, false) {}

ScopedStopwatch::~ScopedStopwatch() noexcept {
    if (!_stopped) {
        stop();
    }
}
