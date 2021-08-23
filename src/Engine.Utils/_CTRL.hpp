#pragma once

#include <mutex>
#include <shared_mutex>

#define _CTRL_OBJ(var_) std::mutex var_
#define _CTRL_GATE(cond_) std::unique_lock<std::mutex> __gate_lock(cond_)

#define _SCTRL_OBJ(var_) std::shared_mutex var_
#define _SCTRL_GATE(cond_) std::lock_guard<std::shared_mutex> __gate_lock(cond_)
#define _SCTRL_SGATE(cond_) std::shared_lock<std::shared_mutex> __sgate_lock(cond_)