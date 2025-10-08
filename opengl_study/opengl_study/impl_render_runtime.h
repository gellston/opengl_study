#pragma once


#ifndef IMPL_RENDER_RUNTIME
#define IMPL_RENDER_RUNTIME

#include <thread>

namespace hv {
	namespace v1 {
		inline int global_glMajor = 2;
		inline int global_glMinor = 1;
		inline std::thread::id main_thread_id;
		inline bool init_done = false;
	}
}

#endif