#pragma once

//C++

//C++/CLI
#include "RenderTypes.h"


//C#

namespace Hv {
	namespace V1 {
		interface class IRender : System::IDisposable
		{
		public:
#pragma region Public Functions
			void CleanUp();

			void ResizeOutput(Hv::V1::OutputDesc out);
			void SetClearColor(Hv::V1::ClearColor clear);

			void ReleaseFrame(Hv::V1::FrameID id);

			void InvalidateView();
			void InvalidateData();
			void InvalidateClear();

			void ZoomBy(float zoom);
			void SetZoom(float zoom);

			void SetPanPixel(float pixel_x, float pixel_y);
			void TranslatePixelBy(float dx_pixels, float dy_pixels);

			void SetPan(float x, float y);
			void TranslateBy(float dx, float dy);

			void SetOrbitAngle(float yawDeg, float pitchDeg);
			void OrbitBy(float dyawDeg, float dpitchDeg);
#pragma endregion

#pragma region Event
			event System::Action<Hv::V1::FrameView>^ OnFrameUpdateEvent;
#pragma endregion






		};
	}
}