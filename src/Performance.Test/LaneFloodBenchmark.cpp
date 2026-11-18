#include "pch.h"

/**/
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <thread>
#include <Engine.Common/Make.hpp>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Graph/StageDecl.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>
#include <Engine.Scheduler/Pipeline/Stage/PipelineStage.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Waitable/JoinCounter.hpp>
#include <Heliogrim/Async/Yield.hpp>

#include "../Engine.Scheduler.Test/TestCompScheduler.hpp"
#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;
using namespace hg::engine::scheduler;

/**
 * Lane flood benchmark: frame time and fan-out latency of a chained CompositePipeline while a flood of long
 *	background-lane tasks saturates the workers, plus the latency of a periodic probe task.
 *
 * DISABLED by default; run with `--gtest_also_run_disabled_tests --gtest_filter=LaneFloodBenchmark.*`.
 * Environment: `HG_BENCH_WORKERS` (0 or unset = auto), `HG_BENCH_FLOOD_COOPERATES` (unset = both flood kinds,
 *	1 = cooperating only, 0 = non-cooperating only).
 *
 * Note: Only a debug configuration is built by default, so absolute numbers are indicative; compare before/after only.
 */
namespace Performance {
	namespace {
		constexpr u32 stageCount = 15;
		constexpr u32 stageSpinMicros = 200;
		constexpr u32 fanOutStages = 8;	// the first fanOutStages stages fan out, includes stage 7
		constexpr u32 fanOut = 8;
		constexpr u32 floodPerWorker = 4;
		constexpr u32 floodTaskMicros = 5000;
		constexpr u32 floodStepMicros = 100;
		constexpr u32 probeEveryMillis = 50;
		constexpr u32 probeMicros = 2000;
		constexpr u32 frames = 2000;
		constexpr u32 maxProbes = 8192;
		constexpr u32 watchdogSeconds = 240;
		constexpr u32 budgetSeconds = 60;// a starved run reports the frames it finished instead of running for minutes

		using clock = std::chrono::steady_clock;

		[[nodiscard]] u64 nowNs() {
			return static_cast<u64>(std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now().time_since_epoch()).count());
		}

		void spinMicros(const u32 micros_) {
			const auto until = nowNs() + static_cast<u64>(micros_) * 1000uLL;
			while (nowNs() < until) {}
		}

		[[nodiscard]] u32 envU32(const char* name_, const u32 fallback_) {
			const char* const value = std::getenv(name_);
			if (not value) {
				return fallback_;
			}
			char* end = nullptr;
			const auto parsed = std::strtoull(value, &end, 10);
			const bool ok = *value != '\0' && *value != '-' && *end == '\0' && parsed <= std::numeric_limits<u32>::max();
			if (not ok) {
				ADD_FAILURE() << name_ << "='" << value << "' is not a u32";
				return fallback_;
			}
			return static_cast<u32>(parsed);
		}

		[[nodiscard]] double pct(ref<Vector<u64>> samples_, const double p_) {
			if (samples_.empty()) {
				return 0.0;
			}
			std::ranges::sort(samples_);
			const auto idx = std::min(samples_.size() - 1uz, static_cast<size_t>(p_ * static_cast<double>(samples_.size())));
			return static_cast<double>(samples_[idx]) / 1000.0;
		}

		void report(const char* kind_, const char* metric_, const double value_, const char* unit_, const size_t n_ = 0) {
			if (n_ != 0) {
				std::printf("[ BENCH    ] %-14s %-34s %12.1f %s (n=%zu)\n", kind_, metric_, value_, unit_, n_);
				return;
			}
			std::printf("[ BENCH    ] %-14s %-34s %12.1f %s\n", kind_, metric_, value_, unit_);
		}

		void reportDist(const char* kind_, const char* metric_, ref<Vector<u64>> samples_, const bool withMax_) {
			char label[64];
			const auto n = samples_.size();
			std::snprintf(label, sizeof(label), "%s p50", metric_);
			report(kind_, label, pct(samples_, 0.50), "us", n);
			std::snprintf(label, sizeof(label), "%s p99", metric_);
			report(kind_, label, pct(samples_, 0.99), "us", n);
			if (withMax_) {
				std::snprintf(label, sizeof(label), "%s max", metric_);
				report(kind_, label, samples_.empty() ? 0.0 : static_cast<double>(*std::ranges::max_element(samples_)) / 1000.0, "us", n);
			}
		}

		struct BenchState {
			std::atomic<u32> frameIdx { 0 };
			u64 frameStart = 0;
			Vector<u64> frameNs;
			Vector<u64> pickupNs;
			Vector<u64> completeNs;
			std::atomic<u32> fanIdx { 0 };
			// flood
			std::atomic_bool floodRun { false };
			std::atomic<u32> floodOutstanding { 0 };
			std::atomic<u64> floodDone { 0 };
			// probe
			std::atomic_bool probeRun { false };
			std::atomic<u32> probesPushed { 0 };
			std::atomic<u32> probeIdx { 0 }; // next sample slot
			std::atomic<u32> probesCompleted { 0 };
			Vector<u64> probeNs;
		};

		class BenchStage final :
			public PipelineStage {
		public:
			BenchStage(const string_view name_, const graph::StageDecl decl_, std::function<void()> body_) :
				PipelineStage(AssocKey<StageIdentifier>::from(string { name_ }), nullptr),
				_name(name_),
				_decl(decl_),
				_body(std::move(body_)) {
				_decl.name = _name;
			}

			~BenchStage() noexcept override = default;

		private:
			string _name;
			graph::StageDecl _decl;
			std::function<void()> _body;

		public:
			void staticDispatch(cref<StaticStageDispatcher>) override {}

			void dynamicDispatch(cref<DynamicStageDispatcher> dispatcher_) override {
				dispatcher_.enqueue(task::make_task([this] { _body(); }));
			}

			[[nodiscard]] graph::StageDecl decl() const noexcept override { return _decl; }
		};

		class BenchPipeline final :
			public StagePipeline {
		public:
			BenchPipeline() :
				StagePipeline(AssocKey<string>::from(string { "::bench::lane" })) {}

			~BenchPipeline() override = default;

			void mount(ref<StageRegister> register_) override {
				for (auto& stage : stages) {
					refs.emplace_back(register_.registerStage(std::move(stage)));
				}
				stages.clear();
			}

			void dismount(ref<StageRegister> register_) override {
				for (auto& stage : refs) {
					if (stage != None) {
						register_.removeStage(std::exchange(stage, None).value());
					}
				}
			}

			Vector<Opt<ref<const Stage>>> refs {};
			Vector<uptr<PipelineStage>> stages {};
		};

		void stageBody(nmpt<engine::Scheduler> sched_, ref<BenchState> st_, const u32 index_) {
			if (index_ == 0) {
				st_.frameStart = nowNs();
			}
			spinMicros(stageSpinMicros);

			if (index_ < fanOutStages) {
				JoinCounter join { fanOut };
				std::atomic<u64> lastStart { 0 };
				const auto first = nowNs();
				for (u32 c = 0; c < fanOut; ++c) {
					sched_->exec(
						task::make_task(
							[&join, &lastStart] {
								const auto started = nowNs();
								u64 prev = lastStart.load(std::memory_order::relaxed);
								while (started > prev && not lastStart.compare_exchange_weak(prev, started)) {}
								spinMicros(stageSpinMicros);
								join.done();
							}
						)
					);
				}
				join.wait();
				const auto end = nowNs();
				const auto at = st_.fanIdx.fetch_add(1);
				if (at < st_.pickupNs.size()) {
					st_.pickupNs[at] = lastStart.load() - first;
					st_.completeNs[at] = end - first;
				}
			}

			if (index_ + 1 == stageCount) {
				const auto at = st_.frameIdx.fetch_add(1);
				if (at < frames) {
					st_.frameNs[at] = nowNs() - st_.frameStart;
				}
			}
		}

		void runOnce(const u32 workers_, const char* kind_, const bool flood_, const bool cooperates_) {
			SchedulerModule::structuredRuntimeTest(
				workers_,
				[&](nmpt<engine::Scheduler> s_) {
					auto state = std::make_unique<BenchState>();
					state->frameNs.resize(frames);
					state->pickupNs.resize(static_cast<size_t>(frames) * fanOutStages);
					state->completeNs.resize(static_cast<size_t>(frames) * fanOutStages);
					state->probeNs.resize(maxProbes);
					auto& st = *state;
					const auto workers = static_cast<u32>(s_->getWorkerCount());

					auto* const comp = static_cast<ptr<engine::TestCompScheduler>>(s_.get());
					CompositePipeline composite { comp->testGetSchedule() };
					composite.setMaxInFlight(1);

					static std::array<string, stageCount> names;
					static std::array<std::array<string_view, 1>, stageCount> afters;
					auto pipeline = make_uptr<BenchPipeline>();
					for (u32 i = 0; i < stageCount; ++i) {
						names[i] = "::bench::stage" + std::to_string(i);
						if (i > 0) {
							afters[i] = { string_view { names[i - 1] } };
						}
						pipeline->stages.push_back(
							make_uptr<BenchStage>(
								names[i],
								graph::StageDecl { .after = i > 0 ? std::span<const string_view> { afters[i] } : std::span<const string_view> {} },
								[s_, &st, i] { stageBody(s_, st, i); }
							)
						);
					}
					(void)composite.addPipeline(std::move(pipeline));
					composite.setup();

					std::atomic_bool finished { false };
					std::thread watchdog {
						[&] {
							const auto until = clock::now() + std::chrono::seconds { watchdogSeconds };
							while (not finished.load()) {
								if (clock::now() > until) {
									std::printf(
										"[ BENCH    ] %-14s HANG after %u s (frames done %u)\n",
										kind_,
										watchdogSeconds,
										st.frameIdx.load()
									);
									std::fflush(stdout);
									std::abort();
								}
								std::this_thread::sleep_for(std::chrono::milliseconds { 10 });
							}
						}
					};

					std::thread floodThread;
					if (flood_) {
						st.floodRun.store(true);
						floodThread = std::thread {
							[&] {
								const auto cap = floodPerWorker * workers;
								while (st.floodRun.load()) {
									while (st.floodOutstanding.load() < cap) {
										st.floodOutstanding.fetch_add(1);
										s_->exec(
											task::make_task(
												[&st, cooperates_] {
													// the cooperating task checkpoints every step, the other never does
													if (cooperates_) {
														for (u32 spun = 0; spun < floodTaskMicros; spun += floodStepMicros) {
															spinMicros(floodStepMicros);
															hg::checkpoint();
														}
													} else {
														spinMicros(floodTaskMicros);
													}
													st.floodDone.fetch_add(1);
													st.floodOutstanding.fetch_sub(1);
												},
												Lane::eBackground
											)
										);
									}
									std::this_thread::sleep_for(std::chrono::microseconds { 200 });
								}
							}
						};
					}

					st.probeRun.store(true);
					std::thread probeThread {
						[&] {
							while (st.probeRun.load()) {
								st.probesPushed.fetch_add(1);
								const auto pushed = nowNs();
								s_->exec(
									task::make_task(
										[&st, pushed] {
											spinMicros(probeMicros);
											const auto lat = nowNs() - pushed;
											const auto at = st.probeIdx.fetch_add(1);
											if (at < maxProbes) {
												st.probeNs[at] = lat;
											}
										st.probesCompleted.fetch_add(1); // publish after the sample write
										},
										Lane::eInteractive
									)
								);
								std::this_thread::sleep_for(std::chrono::milliseconds { probeEveryMillis });
							}
						}
					};

					const auto start = nowNs();
					composite.start();
					const auto deadline = start + static_cast<u64>(budgetSeconds) * 1'000'000'000uLL;
					while (st.frameIdx.load() < frames && nowNs() < deadline) {
						std::this_thread::sleep_for(std::chrono::milliseconds { 5 });
					}
					const auto elapsed = nowNs() - start;
					composite.stop();

					st.probeRun.store(false);
					probeThread.join();
					st.floodRun.store(false);
					if (floodThread.joinable()) {
						floodThread.join();
					}
					while (st.floodOutstanding.load() > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds { 1 });
					}
					// a probe pushed just before the join must finish before st is destroyed
					while (st.probesCompleted.load() != st.probesPushed.load()) {
						std::this_thread::sleep_for(std::chrono::milliseconds { 1 });
					}
					composite.destroy();
					finished.store(true);
					watchdog.join();

					std::printf("[ BENCH    ] %-14s workers=%u\n", kind_, workers);
					st.frameNs.resize(std::min(st.frameIdx.load(), frames));
					std::printf("[ BENCH    ] %-14s frames completed=%zu of %u\n", kind_, st.frameNs.size(), frames);
					reportDist(kind_, "frame time", st.frameNs, true);
					st.pickupNs.resize(std::min<size_t>(st.fanIdx.load(), st.pickupNs.size()));
					st.completeNs.resize(st.pickupNs.size());
					reportDist(kind_, "fan-out pickup", st.pickupNs, false);
					reportDist(kind_, "fan-out completion", st.completeNs, false);
					report(kind_, "flood tasks completed", static_cast<double>(st.floodDone.load()) / (static_cast<double>(elapsed) / 1e9), "/s");
					st.probeNs.resize(std::min<u32>(st.probeIdx.load(), maxProbes));
					reportDist(kind_, "probe latency", st.probeNs, false);
				}
			);
		}
	}

	TEST(LaneFloodBenchmark, DISABLED_Flood) {
		const auto workers = envU32("HG_BENCH_WORKERS", 0);
		const char* const coop = std::getenv("HG_BENCH_FLOOD_COOPERATES");

		runOnce(workers, "idle", false, true);
		if (not coop || coop[0] != '0') {
			runOnce(workers, "flood-coop", true, true);
		}
		if (not coop || coop[0] == '0') {
			runOnce(workers, "flood-noncoop", true, false);
		}
	}
}
