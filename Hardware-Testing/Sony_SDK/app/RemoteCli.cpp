#include <cstdint>
#include <filesystem>
namespace fs = std::filesystem;
#include <cstdlib>
#include <iomanip>
#include "CRSDK/CameraRemote_SDK.h"
#include "CameraDevice.h"
#include "Text.h"
#include <fstream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
namespace SDK = SCRSDK;

int main()
{
    auto init_success = SDK::Init();
    if (!init_success) {
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    
	SDK::ICrEnumCameraObjectInfo* camera_list = nullptr;
	auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr) {
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    auto* camera_info = camera_list->GetCameraObjectInfo(0);
    CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(1, nullptr, camera_info));
	camera->connect(SDK::CrSdkControlMode_Remote);
    std::this_thread::sleep_for(0.4s);
	camera->af_shutter();
	std::this_thread::sleep_for(5s);
	
	camera->disconnect();
	camera->release();
    SDK::Release();
}