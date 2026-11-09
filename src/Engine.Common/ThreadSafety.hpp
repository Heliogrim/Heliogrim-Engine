#pragma once

#if defined(__clang__)
	#define HG_CAPABILITY(x) __attribute__((capability(x)))
	#define HG_SCOPED_CAPABILITY __attribute__((scoped_lockable))
	#define HG_GUARDED_BY(x) __attribute__((guarded_by(x)))
	#define HG_ACQUIRE(...) __attribute__((acquire_capability(__VA_ARGS__)))
	#define HG_ACQUIRE_SHARED(...) __attribute__((acquire_shared_capability(__VA_ARGS__)))
	#define HG_RELEASE(...) __attribute__((release_capability(__VA_ARGS__)))
	#define HG_RELEASE_SHARED(...) __attribute__((release_shared_capability(__VA_ARGS__)))
	#define HG_TRY_ACQUIRE(...) __attribute__((try_acquire_capability(__VA_ARGS__)))
	#define HG_TRY_ACQUIRE_SHARED(...) __attribute__((try_acquire_shared_capability(__VA_ARGS__)))
	#define HG_REQUIRES(...) __attribute__((requires_capability(__VA_ARGS__)))
	#define HG_NO_THREAD_SAFETY_ANALYSIS __attribute__((no_thread_safety_analysis))
#else
	#define HG_CAPABILITY(x)
	#define HG_SCOPED_CAPABILITY
	#define HG_GUARDED_BY(x)
	#define HG_ACQUIRE(...)
	#define HG_ACQUIRE_SHARED(...)
	#define HG_RELEASE(...)
	#define HG_RELEASE_SHARED(...)
	#define HG_TRY_ACQUIRE(...)
	#define HG_TRY_ACQUIRE_SHARED(...)
	#define HG_REQUIRES(...)
	#define HG_NO_THREAD_SAFETY_ANALYSIS
#endif
