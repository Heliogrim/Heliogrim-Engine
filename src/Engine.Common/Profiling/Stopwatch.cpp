#include "Stopwatch.hpp"

#include <thread>

#include "Profiler.hpp"
#include "../Types.hpp"

using namespace ember::profiling;
using namespace ember;

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
    _timestamp = _STD chrono::high_resolution_clock::now();
}

void Stopwatch::stop() noexcept {

    const auto endTimestamp = _STD chrono::high_resolution_clock::now();

    const u64 start = _STD chrono::time_point_cast<_STD chrono::microseconds>(_timestamp).time_since_epoch().count();
    const u64 end = _STD chrono::time_point_cast<_STD chrono::microseconds>(endTimestamp).time_since_epoch().count();

    const u64 tid = _STD hash<_STD thread::id> {}(_STD this_thread::get_id());

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
