#pragma once

#ifndef RENDER_TYPES_HEADER
#define RENDER_TYPES_HEADER

#include <cstdint>
#include <functional>
#include <vector>

namespace hv {
	namespace v1 {
		using handle = std::uint64_t;
		using frameID = std::uint64_t;

		enum class pixelFormat : std::uint32_t {
			RGBA8=1
		};

		enum class result : std::uint32_t {
			ok = 0,
			timeout=1,
			error=2,
			unsupported=3
		};

		struct outputDesc {
			int width = 800;
			int height = 600;
			pixelFormat format = pixelFormat::RGBA8;
		};

		struct clearColor
		{
			float r = 0.28f;
			float g = 0.63f;
			float b = 0.66f;
			float a = 1.0f;
		};


		struct frameView
		{
			frameID id = 0;
			int width = 0;
			int height = 0;
			int stride = 0;
			pixelFormat format = pixelFormat::RGBA8;
			unsigned char* data = nullptr;
			std::uint64_t timestamp_ns = 0;
		};

		using frameCallback = std::function<void(const frameView &)>;


		struct runtimeOptions {
			int glMajor = 2;
			int glMinor = 1;
		};

		
	}
}

#endif