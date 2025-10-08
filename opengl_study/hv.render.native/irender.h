#pragma once

#include "renderTypes.h"

#ifndef IRENDER_HEADER
#define IRENDER_HEADER

namespace hv {
	namespace v1 {
		
		class irender {

		public:
#pragma region Constructor
			irender(const irender&) = delete;
			irender& operator=(const irender&) = delete;

		protected:
			irender() = default;
#pragma endregion

#pragma region Desturctor
		public:
			virtual ~irender() = default;
#pragma endregion

#pragma region Public Functions
			virtual void cleanUp() = 0;

			virtual void resizeOutput(const hv::v1::outputDesc& out) = 0;
			virtual void setClearColor(const hv::v1::clearColor& clear) = 0;

			virtual void registerFrameCallback(hv::v1::frameCallback callback) = 0;
			virtual void releaseFrame(hv::v1::frameID id) = 0;


			virtual void invalidateView() = 0;
			virtual void invalidateData() = 0;
			virtual void invalidateClear() = 0;


			virtual void zoomBy(float zoom) = 0;
			virtual void setZoom(float zoom) = 0;


			virtual void setPan(float x, float y) = 0;
			virtual void translateBy(float dx, float dy) = 0;

			

			virtual void setOrbitAngle(float yawDeg, float pitchDeg) = 0;
			virtual void orbitBy(float dyawDeg, float dpitchDeg) = 0;


#pragma endregion

		};
	}
}

#endif