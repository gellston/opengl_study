#pragma once


//C++

//C++/CLI
#include "RenderTypes.h"

//C#


namespace Hv {
	namespace V1 {
		public value class RenderRuntime {
		public:

#pragma region Static Functions

			static void Configuration(Hv::V1::RuntimeOptions option);
			static Hv::V1::RuntimeOptions Configuration();
#pragma endregion

		};
	}
}