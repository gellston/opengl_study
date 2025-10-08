#pragma once


#ifndef RENDER_RUNTIME_HEADER
#define RENDER_RUNTIME_HEADER

#include "renderTypes.h"
#include "renderExport.h"

namespace hv {
	namespace v1 {
		
		RENDER_API void configuration(const hv::v1::runtimeOptions& options);
		RENDER_API hv::v1::runtimeOptions configuration();
	}
}


#endif