#pragma once

#include <Engine.Common/__macro.hpp>

#pragma region Windows Assert Support
#if _WIN32 || _WIN64
#if not defined(_APISETDEBUG_)

#if not defined(WINBASEAPI)
#define WINBASEAPI __declspec(dllimport)
#endif

#if not defined(WINAPI)
#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define WINAPI __stdcall
#endif
#endif

extern "C" {
	WINBASEAPI void WINAPI OutputDebugStringA(/*_In_opt_ _Null_terminated_*/ const char* msg_);
}

#endif
#endif
#pragma endregion Windows Assert Support
