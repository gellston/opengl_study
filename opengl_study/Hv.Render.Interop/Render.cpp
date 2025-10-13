
//C++
#include <msclr/marshal_cppstd.h>

//C++/CLI
#include "Render.h"



//C#






#pragma region Constructor

Hv::V1::Render::Render() {

	if (this->instance == nullptr) {
		this->instance = new hv::v1::render();
	}

	this->OnFrameUpdateDelegate = gcnew Hv::V1::Render::FrameUpdateDelegate(this, &Hv::V1::Render::OnFrameUpdate);
	auto OnFrameUpdatePointer = (void (*)(hv::v1::frameView)) System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(this->OnFrameUpdateDelegate).ToPointer();
	this->instance->registerFrameCallback(OnFrameUpdatePointer);

}
#pragma endregion

#pragma region Destructor
Hv::V1::Render::~Render() {
	this->Clean(false);
}
#pragma endregion

#pragma region Finalizer
Hv::V1::Render::!Render() {
	this->Clean(true);
}
#pragma endregion




#pragma region Private Functions

void Hv::V1::Render::OnFrameUpdate(hv::v1::frameView view) {

	Hv::V1::FrameView managedView;
	managedView.Data = System::IntPtr(view.data);
	managedView.Format = static_cast<Hv::V1::PixelFormat>(view.format);
	managedView.Height = view.height;
	managedView.Width = view.width;
	managedView.ID = view.id;
	managedView.Stride = view.stride;
	managedView.TimeStampNS = view.timestamp_ns;



	
	this->OnFrameUpdateEvent(managedView);

}
#pragma endregion


#pragma region Public Functions

void Hv::V1::Render::Clean(bool isFinalizing) {
	if (!this->disposed) {


		//Managed자원 해제
		if (!isFinalizing) {
			//여긴 특별히 지울 managed자원이 없음
			this->OnFrameUpdateDelegate = nullptr;

		}

		if (this->instance != nullptr) {
			delete this->instance;
			this->instance = nullptr;
		}

		this->disposed = true;
	}
}

void Hv::V1::Render::CleanUp() {
	try {

		this->instance->cleanUp();

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::ResizeOutput(Hv::V1::OutputDesc out) {
	try {

		hv::v1::outputDesc nativeOut;
		nativeOut.format = static_cast<hv::v1::pixelFormat>(out.Format);
		nativeOut.height = out.Height;
		nativeOut.width = out.Width;

		this->instance->resizeOutput(nativeOut);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
void Hv::V1::Render::SetClearColor(Hv::V1::ClearColor clear) {
	try {

		hv::v1::clearColor nativeColor;
		nativeColor.r = clear.R;
		nativeColor.g = clear.G;
		nativeColor.b = clear.B;
		nativeColor.a = clear.A;

		this->instance->setClearColor(nativeColor);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::ReleaseFrame(Hv::V1::FrameID id) {
	try {

		this->instance->releaseFrame(id);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::InvalidateView() {
	try {

		this->instance->invalidateView();
	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
void Hv::V1::Render::InvalidateData() {
	try {

		this->instance->invalidateData();

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
void Hv::V1::Render::InvalidateClear() {
	try {

		this->instance->invalidateClear();

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::ZoomBy(float zoom) {
	try {

		this->instance->zoomBy(zoom);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::SetZoom(float zoom) {
	try {

		this->instance->setZoom(zoom);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}


void Hv::V1::Render::SetPan(float x, float y) {
	try {

		this->instance->setPan(x, y);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::TranslateBy(float dx, float dy) {
	try {

		this->instance->translateBy(dx, dy);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}

void Hv::V1::Render::SetPanPixel(float pixel_x, float pixel_y) {
	try {

		this->instance->setPanPixel(pixel_x, pixel_y);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
void Hv::V1::Render::TranslatePixelBy(float dx_pixels, float dy_pixels) {
	try {

		this->instance->translatePixelBy(dx_pixels, dy_pixels);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}


void Hv::V1::Render::SetOrbitAngle(float yawDeg, float pitchDeg) {
	try {

		this->instance->setOrbitAngle(yawDeg, pitchDeg);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
void Hv::V1::Render::OrbitBy(float dyawDeg, float dpitchDeg) {
	try {

		this->instance->orbitBy(dyawDeg, dpitchDeg);

	}
	catch (std::exception ex) {
		throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what()));
	}
}
#pragma endregion


