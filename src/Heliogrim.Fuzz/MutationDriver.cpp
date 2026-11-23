/**
 * shortcut: no coverage guidance; upgrade when a /MD libFuzzer or a Linux/CI build exists.
 * Usage: exe [-seed=N] [-max_total_time=S] | exe <file> (replay one input).
 */

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <random>
#include <string>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t*, size_t);
extern "C" size_t HgFuzzSeed(uint8_t*, size_t);
extern "C" void __sanitizer_set_death_callback(void (*)(void));

namespace {
	using Bytes = std::vector<uint8_t>;

	Bytes current {};
	std::string crashFile {};
	unsigned long long seedValue = 0uLL;
	unsigned long long execs = 0uLL;

	void saveCurrent() {
		std::fprintf(stderr, "abort: seed=%llu executions=%llu\n", seedValue, execs);
		std::fflush(stdout);
		std::fflush(stderr);
		if (FILE* const f = std::fopen(crashFile.c_str(), "wb")) {
			std::fwrite(current.data(), 1, current.size(), f);
			std::fclose(f);
			std::fprintf(stderr, "failing input saved to %s (%zu bytes)\n", crashFile.c_str(), current.size());
		}
	}

	void onAbort(int) { saveCurrent(); }

	void mutate(Bytes& b_, std::mt19937_64& rng_, const Bytes& other_) {
		auto pick = [&](size_t n_) { return n_ == 0uLL ? 0uLL : static_cast<size_t>(rng_() % n_); };
		constexpr uint64_t extremes[] { 0uLL, 1uLL, 0x7FuLL, 0xFFuLL, 0xFFFFuLL, 0x7FFFFFFFuLL, 0xFFFFFFFFuLL, 0x7FFFFFFFFFFFFFFFuLL, ~0uLL };
		const size_t rounds = 1uLL + pick(4uLL);
		for (size_t r = 0; r < rounds; ++r) {
			switch (rng_() % 7uLL) {
				case 0: /* bit flip */
					if (not b_.empty()) { b_[pick(b_.size())] ^= static_cast<uint8_t>(1u << (rng_() % 8uLL)); }
					break;
				case 1: /* random byte */
					if (not b_.empty()) { b_[pick(b_.size())] = static_cast<uint8_t>(rng_()); }
					break;
				case 2: case 3: { /* header-aware: length/tag/offset words near max, biased to the first 64 bytes */
					const size_t w = size_t { 1uLL } << (rng_() % 4uLL);
					if (b_.size() >= w) {
						const size_t at = pick(((rng_() & 1uLL) != 0uLL && b_.size() > 64uLL ? 64uLL : b_.size()) - w + 1uLL);
						uint64_t v = extremes[rng_() % std::size(extremes)];
						if ((rng_() & 3uLL) == 0uLL) { v -= rng_() % 4uLL; }
						std::memcpy(b_.data() + at, &v, w);
					}
					break;
				}
				case 4: /* truncation */
					b_.resize(pick(b_.size() + 1uLL));
					break;
				case 5: /* splice from another seed */
					if (not other_.empty()) {
						const size_t from = pick(other_.size()), len = 1uLL + pick(other_.size() - from);
						const size_t at = pick(b_.size() + 1uLL);
						b_.insert(b_.begin() + at, other_.begin() + from, other_.begin() + from + len);
					}
					break;
				default: /* odd first byte selects patch mode of the harness */
					if (b_.empty()) { b_.push_back(1u); } else { b_[0] = static_cast<uint8_t>(rng_()); }
					break;
			}
		}
		if (b_.size() > 1uLL << 16uLL) { b_.resize(1uLL << 16uLL); }
	}
}

int main(int argc_, char** argv_) {
	uint64_t seed = std::random_device {}();
	double budget = 10.0;
	for (int i = 1; i < argc_; ++i) {
		if (std::strncmp(argv_[i], "-seed=", 6) == 0) { seed = std::strtoull(argv_[i] + 6, nullptr, 10); }
		else if (std::strncmp(argv_[i], "-max_total_time=", 16) == 0) { budget = std::atof(argv_[i] + 16); }
		else if (argv_[i][0] != '-') {
			Bytes in {};
			if (FILE* const f = std::fopen(argv_[i], "rb")) {
				uint8_t buf[4096];
				for (size_t n; (n = std::fread(buf, 1, sizeof(buf), f)) > 0;) { in.insert(in.end(), buf, buf + n); }
				std::fclose(f);
			}
			return LLVMFuzzerTestOneInput(in.data(), in.size());
		}
	}

	crashFile = "crash-" + std::to_string(seed) + ".bin";
	std::signal(SIGABRT, onAbort);
	__sanitizer_set_death_callback(saveCurrent);
	std::printf("seed=%llu max_total_time=%.0f\n", static_cast<unsigned long long>(seed), budget);

	Bytes dump(1u << 14u);
	dump.resize(HgFuzzSeed(dump.data(), dump.size()));
	std::printf("seed dump: %zu bytes, first byte 0x%02x (%s mode)\n", dump.size(), dump.empty() ? 0u : dump[0], (not dump.empty() && (dump[0] & 1u)) ? "patch" : "raw");
	std::vector<Bytes> corpus { dump };
	if (not dump.empty()) { Bytes odd = dump; odd[0] |= 1u; corpus.push_back(odd); Bytes ev = dump; ev[0] &= ~1u; corpus.push_back(ev); }

	seedValue = seed;
	std::mt19937_64 rng { seed };
	const auto start = std::chrono::steady_clock::now();
	while (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() < budget) {
		current = corpus[rng() % corpus.size()];
		mutate(current, rng, corpus[rng() % corpus.size()]);
		LLVMFuzzerTestOneInput(current.data(), current.size());
		if ((++execs & 0xFFFFuLL) == 0uLL) { std::printf("#%llu\n", execs); std::fflush(stdout); }
	}
	std::printf("DONE seed=%llu executions=%llu\n", static_cast<unsigned long long>(seed), execs);
	return 0;
}
