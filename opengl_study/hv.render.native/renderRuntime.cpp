

#include "renderRuntime.h"
#include "implRenderRuntime.h"

#pragma region Static Variables

#pragma endregion



#pragma region Functions
void hv::v1::configuration(const hv::v1::runtimeOptions& options) {

	hv::v1::global_glMajor = options.glMajor;
	hv::v1::global_glMinor = options.glMinor;

}

hv::v1::runtimeOptions hv::v1::configuration() {
	hv::v1::runtimeOptions option;

	option.glMajor = hv::v1::global_glMajor;
	option.glMinor = hv::v1::global_glMinor;

	return option;
}
#pragma endregion
