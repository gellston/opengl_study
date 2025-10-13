#pragma once


//C++
#include <render.h>


//C++/CLI
#include "IRender.h"



//C#

namespace Hv {
	namespace V1 {
		public ref class Render : public Hv::V1::IRender{
		private:
#pragma region Private Property
			bool disposed = false;
			hv::v1::render* instance = nullptr;
#pragma endregion

#pragma region Private Functions
			delegate void FrameUpdateDelegate(hv::v1::frameView view);
			FrameUpdateDelegate^ OnFrameUpdateDelegate;
			void OnFrameUpdate(hv::v1::frameView view);
#pragma endregion


		public:
#pragma region Constructor
			Render();
#pragma endregion

#pragma region Destructor
			~Render();
#pragma endregion

#pragma region Finalizer
			!Render();
#pragma endregion

#pragma region Public Functions

			void Clean(bool isFinalizing);

			virtual void CleanUp();

			virtual void ResizeOutput(Hv::V1::OutputDesc out);
			virtual void SetClearColor(Hv::V1::ClearColor clear);

			virtual void ReleaseFrame(Hv::V1::FrameID id);

			virtual void InvalidateView();
			virtual void InvalidateData();
			virtual void InvalidateClear();


			virtual void ZoomBy(float zoom);
			virtual void SetZoom(float zoom);


			virtual void SetPan(float x, float y);
			virtual void TranslateBy(float dx, float dy);

			virtual void SetPanPixel(float pixel_x, float pixel_y);
			virtual void TranslatePixelBy(float dx_pixels, float dy_pixels);

			virtual void SetOrbitAngle(float yawDeg, float pitchDeg);
			virtual void OrbitBy(float dyawDeg, float dpitchDeg);
#pragma endregion


#pragma region Event
			virtual event System::Action<Hv::V1::FrameView>^ OnFrameUpdateEvent;
#pragma endregion





		};
	}
}