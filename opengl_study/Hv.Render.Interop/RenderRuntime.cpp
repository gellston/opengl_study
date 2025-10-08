

//C++
#include <renderRuntime.h>


//C++/CLI
#include "RenderRuntime.h"

//C#


#pragma region Static Functions
void Hv::V1::RenderRuntime::Configuration(Hv::V1::RuntimeOptions option) {

	hv::v1::runtimeOptions nativeOption{
		option.GLMajor,
		option.GLMinor
	};

	hv::v1::configuration(nativeOption);

}


Hv::V1::RuntimeOptions Hv::V1::RenderRuntime::Configuration() {
	auto nativeOption = hv::v1::configuration();

	Hv::V1::RuntimeOptions managedOption;
	managedOption.GLMajor = nativeOption.glMajor;
	managedOption.GLMinor = nativeOption.glMinor;

	return managedOption;
}
#pragma endregion

