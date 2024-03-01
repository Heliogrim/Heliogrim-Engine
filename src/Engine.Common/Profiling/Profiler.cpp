#include "Profiler.hpp"

#include <algorithm>
#include <fstream>

using namespace hg::profiling;
using namespace hg;

ptr<Profiler> Profiler::_instance = nullptr;

Profiler::Profiler() noexcept :
    _name(),
    _output(nullptr),
    _count(0uLL) {}

Profiler::~Profiler() noexcept {
    if (_output != nullptr) {
        stopSession();
    }
}

const ptr<Profiler> Profiler::get() {
    return _instance;
}

const ptr<Profiler::value_type> Profiler::get(std::nothrow_t) noexcept {
    return _instance;
}

Profiler::reference_type Profiler::make() {
    if (_instance == nullptr) {
        _instance = new Profiler();
    }

    return *_instance;
}

const ptr<Profiler::value_type> Profiler::make(std::nothrow_t) noexcept {
    if (_instance == nullptr) {
        _instance = new Profiler();
    }

    return _instance;
}

void Profiler::destroy() {
    delete _instance;
    _instance = nullptr;
}

void Profiler::flush() {}

void Profiler::startSession(cref<string_view> name_, cref<string_view> file_) {

    _name = name_;
    _output = new std::ofstream { file_.data() };

    writeHeader();
}

void Profiler::stopSession() {

    writeFooter();

    static_cast<ptr<std::ofstream>>(_output)->close();
    delete _output;
    _output = nullptr;

    _name.clear();
    _count = 0;
}

void Profiler::writeProfile(cref<ProfileResult> result_) {

    concurrent::SpinLock _lck { _mtx };

    if (_count++ > 0) {
        *_output << ',';
    }

    string name { result_.name };
    std::ranges::replace(name, '"', '\'');

    *_output << '{';
    *_output << R"("cat": "function",)";
    *_output << R"("dur": )" << (result_.end - result_.start) << ',';
    *_output << R"("name": ")" << name << R"(",)";
    *_output << R"("ph": "X",)";
    *_output << R"("pid": 0,)";
    *_output << R"("tid": )" << result_.thread << ',';
    *_output << R"("ts": )" << result_.start;
    *_output << '}';

    _output->flush();
}

void Profiler::writeFooter() {

    *_output << R"(]})";
    _output->flush();
}

void Profiler::writeHeader() {

    *_output << R"({"otherData": {}, "traceEvents": [)";
    _output->flush();
}

ref<Profiler> profiling::operator<<(ref<Profiler> self_, cref<ProfileResult> result_) {
    self_.writeProfile(result_);
    return self_;
}
