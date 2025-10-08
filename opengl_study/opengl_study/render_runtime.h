#pragma once


#ifndef RENDER_RUNTIME_HEADER
#define RENDER_RUNTIME_HEADER

#include "render_types.h"

namespace hv {
	namespace v1 {
		
		void configuration(const hv::v1::runtimeOptions& options = {});
		hv::v1::runtimeOptions configuration();
	}
}


#endif