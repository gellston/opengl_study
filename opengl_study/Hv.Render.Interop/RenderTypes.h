#pragma once


//C++
#include <cstdint>


//C++/CLI

//C#
using namespace System::Runtime::InteropServices;

namespace Hv {
	namespace V1 {
		using Handle = std::uint64_t;
		using FrameID = std::uint64_t;

		public enum class PixelFormat : std::uint32_t {
			RGBA8 = 1
		};

		public enum class Result : std::uint32_t {
			OK = 0,
			TimeOut = 1,
			Error = 2,
			UnSupported = 3
		};

		[StructLayout(LayoutKind::Sequential)]
			public value struct OutputDesc {
			int Width;
			int Height;
			PixelFormat Format;
			OutputDesc(int width, int height, PixelFormat format) : Width(width), Height(height), Format(format) {}
		};

		[StructLayout(LayoutKind::Sequential)]
			public value struct ClearColor {
			float R;
			float G;
			float B;
			float A;
			ClearColor(float r, float g, float b, float a) :R(r), G(g), B(b), A(a) {}
		};


		[StructLayout(LayoutKind::Sequential)]
			public value struct FrameView {
			FrameID ID;
			int Width;
			int Height;
			int Stride;
			PixelFormat Format;
			System::IntPtr Data;
			std::uint64_t TimeStampNS;
			FrameView(FrameID id, int width, int height, int stride, PixelFormat format, System::IntPtr data, std::uint64_t timeStampNS) :
				ID(id), Width(width), Height(height), Stride(stride), Format(format), Data(data), TimeStampNS(timeStampNS) {
			}
		};

		[StructLayout(LayoutKind::Sequential)]
			public value struct RuntimeOptions {
			int GLMajor;
			int GLMinor;
			RuntimeOptions(int glMajor, int glMinor) :GLMajor(glMajor), GLMinor(glMinor) {

			}
		};

	}
}