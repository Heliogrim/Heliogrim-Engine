#include "Wait.hpp"

#include "../Fiber/Fiber.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

void engine::scheduler::waitOnAtomic(ref<std::atomic_flag> atomic_, const bool prev_) {
    if (/* TODO: Check whether we are thread or fiber */false) {
        while (atomic_.test(std::memory_order::relaxed) == prev_) {
            fiber::self::yield();
        }
    } else {
        atomic_.wait(prev_, _STD memory_order::relaxed);
    }
}

void engine::scheduler::waitOnAtomic(ref<std::atomic_uint_fast8_t> atomic_, const u8 prev_) {
    if (/* TODO: Check whether we are thread or fiber */false) {
        while (atomic_.load(_STD memory_order::relaxed) == prev_) {
            fiber::self::yield();
        }
    } else {
        atomic_.wait(prev_, _STD memory_order::relaxed);
    }
}

void engine::scheduler::waitUntilAtomic(ref<std::atomic_flag> atomic_, const bool expect_) {

    if (/* TODO: Check whether we are thread or fiber */false) {
        while (atomic_.test(std::memory_order::relaxed) != expect_) {
            fiber::self::yield();
        }
    } else {
        atomic_.wait(not expect_, _STD memory_order::relaxed);
    }

}

void engine::scheduler::waitUntilAtomic(ref<std::atomic_uint_fast8_t> atomic_, const u8 expect_) {

    if (/* TODO: Check whether we are thread or fiber */false) {
        while (atomic_.load(_STD memory_order::relaxed) != expect_) {
            fiber::self::yield();
        }
    } else {

        // TODO: Check for better implementation
        // Warning: This is pretty costy

        // Spin on atomic while not matches with expected value
        auto prev { atomic_.load() };
        while (prev != expect_) {
            // Wait for value changes to read next stored value
            atomic_.wait(prev, _STD memory_order::consume);
            prev = atomic_.load();
        }
    }

}
