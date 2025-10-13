#pragma once


#ifndef RENDER_HEADER
#define RENDER_HEADER

#include <memory>

#include "irender.h"
#include "renderExport.h"

namespace hv {
	namespace v1 {
		struct impl_render;
		class render : public hv::v1::irender{

#pragma region Private Property
			std::unique_ptr<impl_render> impl;

#pragma endregion





#pragma region Constructor
		public:
			RENDER_API render();
#pragma endregion

#pragma region Destructor
			RENDER_API ~render();
#pragma endregion

#pragma region Public Functions
			RENDER_API void cleanUp() override;

			RENDER_API void resizeOutput(const hv::v1::outputDesc& out) override;
			RENDER_API void setClearColor(const hv::v1::clearColor& clear) override;

			RENDER_API void registerFrameCallback(hv::v1::frameCallback callback) override;
			RENDER_API void releaseFrame(hv::v1::frameID id) override;


			RENDER_API void invalidateView() override;
			RENDER_API void invalidateData() override;
			RENDER_API void invalidateClear() override;


			RENDER_API void zoomBy(float zoom) override;
			RENDER_API void setZoom(float zoom) override;


			RENDER_API void setPan(float x, float y) override;
			RENDER_API void translateBy(float dx, float dy) override;

			RENDER_API void setPanPixel(float x_pixels, float y_pixels) override;
			RENDER_API void translatePixelBy(float dx_pixels, float dy_pixels) override;

			RENDER_API void setOrbitAngle(float yawDeg, float pitchDeg) override;
			RENDER_API void orbitBy(float dyawDeg, float dpitchDeg) override;

#pragma endregion

		};
	}
}


#endif
