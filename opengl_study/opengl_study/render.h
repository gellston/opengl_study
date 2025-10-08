#pragma once


#ifndef RENDER_HEADER
#define RENDER_HEADER

#include <memory>

#include "irender.h"


namespace hv {
	namespace v1 {
		struct impl_render;
		class render : public hv::v1::irender{

#pragma region Private Property
			std::unique_ptr<impl_render> impl;
#pragma endregion


#pragma region Constructor
		public:
			render();
#pragma endregion

#pragma region Destructor
			~render();
#pragma endregion

#pragma region Public Functions
			void dispose() override;

			void resizeOutput(const hv::v1::outputDesc& out) override;
			void setClearColor(const hv::v1::clearColor& clear) override;

			void registerFrameCallback(hv::v1::frameCallback callback) override;
			void releaseFrame(hv::v1::frameID id) override;


			void invalidateView() override;
			void invalidateData() override;
			void invalidateClear() override;


			void zoomBy(float zoom) override;
			void setZoom(float zoom) override;


			void setPan(float x, float y) override;
			void translateBy(float dx, float dy) override;

			void setOrbitAngle(float yawDeg, float pitchDeg) override;
			void orbitBy(float dyawDeg, float dpitchDeg) override;

#pragma endregion

		};
	}
}


#endif
