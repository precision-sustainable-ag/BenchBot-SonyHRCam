#include "CameraDevice.h"
#include <chrono>
#if defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#if defined(__APPLE__)
#include <unistd.h>
#endif
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
#include <fstream>
#include <thread>
#include "CRSDK/CrDeviceProperty.h"
#include "Text.h"

namespace SDK = SCRSDK;
using namespace std::chrono_literals;

constexpr int const ImageSaveAutoStartNo = -1;

namespace cli
{
CameraDevice::CameraDevice(std::int32_t no, CRLibInterface const* cr_lib, SCRSDK::ICrCameraObjectInfo const* camera_info)
    : m_cr_lib(cr_lib)
    , m_number(no)
    , m_device_handle(0)
    , m_connected(false)
    , m_conn_type(ConnectionType::UNKNOWN)
    , m_net_info()
    , m_usb_info()
    , m_prop()
    , m_lvEnbSet(true)
    , m_modeSDK(SCRSDK::CrSdkControlMode_ContentsTransfer)
    , m_spontaneous_disconnection(false)
{
    m_info = SDK::CreateCameraObjectInfo(
        camera_info->GetName(),
        camera_info->GetModel(),
        camera_info->GetUsbPid(),
        camera_info->GetIdType(),
        camera_info->GetIdSize(),
        camera_info->GetId(),
        camera_info->GetConnectionTypeName(),
        camera_info->GetAdaptorName(),
        camera_info->GetPairingNecessity()
    );

    m_conn_type = parse_connection_type(m_info->GetConnectionTypeName());
    switch (m_conn_type)
    {
    case ConnectionType::NETWORK:
        m_net_info = parse_ip_info(m_info->GetId(), m_info->GetIdSize());
        break;
    case ConnectionType::USB:
        m_usb_info.pid = m_info->GetUsbPid();
        break;
    case ConnectionType::UNKNOWN:
        [[fallthrough]];
    default:
        // Do nothing
        break;
    }
}

CameraDevice::~CameraDevice()
{
    if (m_info) m_info->Release();
}

bool CameraDevice::connect(SCRSDK::CrSdkControlMode openMode)
{
    m_spontaneous_disconnection = false;
    // auto connect_status = m_cr_lib->Connect(m_info, this, &m_device_handle);
    auto connect_status = SDK::Connect(m_info, this, &m_device_handle, openMode);
    if (CR_FAILED(connect_status)) {
        text id(this->get_id());
        tout << std::endl << "Failed to connect : 0x" << std::hex << connect_status << std::dec << ". " << m_info->GetModel() << " (" << id.data() << ")\n";
        return false;
    }
    set_save_info();
    return true;
}

bool CameraDevice::disconnect()
{
    m_spontaneous_disconnection = true;
    tout << "Disconnect from camera...\n";
    // auto disconnect_status = m_cr_lib->Disconnect(m_device_handle);
    auto disconnect_status = SDK::Disconnect(m_device_handle);
    if (CR_FAILED(disconnect_status)) {
        tout << "Disconnect failed to initialize.\n";
        return false;
    }
    return true;
}

bool CameraDevice::release()
{
    tout << "Release camera...\n";
    // auto finalize_status = m_cr_lib->FinalizeDevice(m_device_handle);
    auto finalize_status = SDK::ReleaseDevice(m_device_handle);
    m_device_handle = 0; // clear
    if (CR_FAILED(finalize_status)) {
        tout << "Finalize device failed to initialize.\n";
        return false;
    }
    return true;
}

SCRSDK::CrSdkControlMode CameraDevice::get_sdkmode() 
{
    load_properties();
    if (SDK::CrSdkControlMode_ContentsTransfer == m_modeSDK) {
        tout << TEXT("Contets Transfer Mode\n");
    }
    else {
        tout << TEXT("Remote Control Mode\n");
    }
    return m_modeSDK;
}

void CameraDevice::capture_image() const
{
    tout << "Capture image...\n";
    tout << "Shutter down\n";
    // m_cr_lib->SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam_Down);
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam_Down);

    // Wait, then send shutter up
    std::this_thread::sleep_for(35ms);
    tout << "Shutter up\n";
    // m_cr_lib->SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam_Up);
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam_Up);
}

void CameraDevice::s1_shooting() const
{
    text input;
    tout << "Is the focus mode set to AF? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Set the focus mode to AF\n";
        return;
    }

    tout << "S1 shooting...\n";
    tout << "Shutter Halfpress down\n";
    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_S1);
    prop.SetCurrentValue(SDK::CrLockIndicator::CrLockIndicator_Locked);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16);
    SDK::SetDeviceProperty(m_device_handle, &prop);

    // Wait, then send shutter up
    std::this_thread::sleep_for(1s);
    tout << "Shutter Halfpress up\n";
    prop.SetCurrentValue(SDK::CrLockIndicator::CrLockIndicator_Unlocked);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::af_shutter() const
{
    text input;
    tout << "Is the focus mode set to AF? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Set the focus mode to AF\n";
        return;
    }

    tout << "S1 shooting...\n";
    tout << "Shutter Halfpress down\n";
    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_S1);
    prop.SetCurrentValue(SDK::CrLockIndicator::CrLockIndicator_Locked);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16);
    SDK::SetDeviceProperty(m_device_handle, &prop);

    // Wait, then send shutter down
    std::this_thread::sleep_for(500ms);
    tout << "Shutter down\n";
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam::CrCommandParam_Down);

    // Wait, then send shutter up
    std::this_thread::sleep_for(35ms);
    tout << "Shutter up\n";
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam::CrCommandParam_Up);

    // Wait, then send shutter up
    std::this_thread::sleep_for(1s);
    tout << "Shutter Halfpress up\n";
    prop.SetCurrentValue(SDK::CrLockIndicator::CrLockIndicator_Unlocked);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::continuous_shooting() const
{
    tout << "Capture image...\n";
    tout << "Continuous Shooting\n";

    // Set, PriorityKeySettings property
    SDK::CrDeviceProperty priority;
    priority.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_PriorityKeySettings);
    priority.SetCurrentValue(SDK::CrPriorityKeySettings::CrPriorityKey_PCRemote);
    priority.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);
    auto err_priority = SDK::SetDeviceProperty(m_device_handle, &priority);
    if (CR_FAILED(err_priority)) {
        tout << "Priority Key setting FAILED\n";
        return;
    }
    else {
        tout << "Priority Key setting SUCCESS\n";
    }

    // Set, still_capture_mode property
    SDK::CrDeviceProperty mode;
    mode.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_DriveMode);
    mode.SetCurrentValue(SDK::CrDriveMode::CrDrive_Continuous_Hi);
    mode.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);
    auto err_still_capture_mode = SDK::SetDeviceProperty(m_device_handle, &mode);
    if (CR_FAILED(err_still_capture_mode)) {
        tout << "Still Capture Mode setting FAILED\n";
        return;
    }
    else {
        tout << "Still Capture Mode setting SUCCESS\n";
    }

    // get_still_capture_mode();
    std::this_thread::sleep_for(1s);
    tout << "Shutter down\n";
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam::CrCommandParam_Down);

    // Wait, then send shutter up
    std::this_thread::sleep_for(500ms);
    tout << "Shutter up\n";
    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_Release, SDK::CrCommandParam::CrCommandParam_Up);
}

void CameraDevice::get_aperture()
{
    load_properties();
    tout << format_f_number(m_prop.f_number.current) << '\n';
}

void CameraDevice::get_iso()
{
    load_properties();

    tout << "ISO: " << format_iso_sensitivity(m_prop.iso_sensitivity.current) << '\n';
}

void CameraDevice::get_shutter_speed()
{
    load_properties();
    tout << "Shutter speed: " << format_shutter_speed(m_prop.shutter_speed.current) << '\n';
}

void CameraDevice::get_position_key_setting()
{
    load_properties();
    tout << "Position Key Setting: " << format_position_key_setting(m_prop.position_key_setting.current) << '\n';
}

void CameraDevice::get_exposure_program_mode()
{
    load_properties();
    tout << "Exposure Program Mode: " << format_exposure_program_mode(m_prop.exposure_program_mode.current) << '\n';
}

void CameraDevice::get_still_capture_mode()
{
    load_properties();
    tout << "Still Capture Mode: " << format_still_capture_mode(m_prop.still_capture_mode.current) << '\n';
}

void CameraDevice::get_focus_mode()
{
    load_properties();
    tout << "Focus Mode: " << format_focus_mode(m_prop.focus_mode.current) << '\n';
}

void CameraDevice::get_focus_area()
{
    load_properties();
    tout << "Focus Area: " << format_focus_area(m_prop.focus_area.current) << '\n';
}

void CameraDevice::get_live_view()
{
    tout << "GetLiveView...\n";

    CrInt32 num = 0;
    SDK::CrLiveViewProperty* property = nullptr;
    auto err = SDK::GetLiveViewProperties(m_device_handle, &property, &num);
    if (CR_FAILED(err)) {
        tout << "GetLiveView FAILED\n";
        return;
    }
    SDK::ReleaseLiveViewProperties(m_device_handle, property);

    SDK::CrImageInfo inf;
    err = SDK::GetLiveViewImageInfo(m_device_handle, &inf);
    if (CR_FAILED(err)) {
        tout << "GetLiveView FAILED\n";
        return;
    }

    CrInt32u bufSize = inf.GetBufferSize();
    if (bufSize < 1)
    {
        tout << "GetLiveView FAILED \n";
    }
    else
    {
        auto* image_data = new SDK::CrImageDataBlock();
        if (!image_data)
        {
            tout << "GetLiveView FAILED (new CrImageDataBlock class)\n";
            return;
        }
        CrInt8u* image_buff = new CrInt8u[bufSize];
        if (!image_buff)
        {
            delete image_data;
            tout << "GetLiveView FAILED (new Image buffer)\n";
            return;
        }
        image_data->SetSize(bufSize);
        image_data->SetData(image_buff);

        err = SDK::GetLiveViewImage(m_device_handle, image_data);
        if (CR_FAILED(err))
        {
            // FAILED
            if (err == SDK::CrWarning_Frame_NotUpdated) {
                tout << "Warning. GetLiveView Frame NotUpdate\n";
            }
            else if (err == SDK::CrError_Memory_Insufficient) {
                tout << "Warning. GetLiveView Memory insufficient\n";
            }
            delete[] image_buff; // Release
            delete image_data; // Release
        }
        else
        {
            if (0 < image_data->GetSize())
            {
                // Display
                // etc.
#if defined(__APPLE__)
                char path[255]; /*MAX_PATH*/
                getcwd(path, sizeof(path) -1);
                char filename[] ="/LiveView000000.JPG";
                strcat(path, filename);
#else
                auto path = fs::current_path();
                path.append(TEXT("LiveView000000.JPG"));
#endif
                tout << path << '\n';

                std::ofstream file(path, std::ios::out | std::ios::binary);
                if (!file.bad())
                {
                    file.write((char*)image_data->GetImageData(), image_data->GetImageSize());
                    file.close();
                }
                tout << "GetLiveView SUCCESS\n";
                delete[] image_buff; // Release
                delete image_data; // Release
            }
            else
            {
                // FAILED
                delete[] image_buff; // Release
                delete image_data; // Release
            }
        }
    }
}

void CameraDevice::get_live_view_image_quality()
{
    load_properties();
    tout << "Live View Image Quality: " << format_live_view_image_quality(m_prop.live_view_image_quality.current) << '\n';
}

void CameraDevice::get_live_view_status()
{
    load_properties();
    tout << "LiveView Enabled: " << format_live_view_status(m_prop.live_view_status.current) << '\n';
}

void CameraDevice::get_select_media_format()
{
    load_properties();
    tout << "Media SLOT1 Full Format Enable Status: " << format_media_slotx_format_enable_status(m_prop.media_slot1_full_format_enable_status.current) << std::endl;
    tout << "Media SLOT2 Full Format Enable Status: " << format_media_slotx_format_enable_status(m_prop.media_slot2_full_format_enable_status.current) << std::endl;
    // Valid Quick format
    if (m_prop.media_slot1_quick_format_enable_status.writable || m_prop.media_slot2_quick_format_enable_status.writable){
        tout << "Media SLOT1 Quick Format Enable Status: " << format_media_slotx_format_enable_status(m_prop.media_slot1_quick_format_enable_status.current) << std::endl;
        tout << "Media SLOT2 Quick Format Enable Status: " << format_media_slotx_format_enable_status(m_prop.media_slot2_quick_format_enable_status.current) << std::endl;
    }
}

void CameraDevice::get_white_balance()
{
    load_properties();
    tout << "White Balance: " << format_white_balance(m_prop.white_balance.current) << '\n';
}

bool CameraDevice::get_custom_wb()
{
    bool state = false;
    load_properties();
    tout << "CustomWB Capture Standby Operation: " << format_customwb_capture_stanby(m_prop.customwb_capture_stanby.current) << '\n';
    tout << "CustomWB Capture Standby CancelOperation: " << format_customwb_capture_stanby_cancel(m_prop.customwb_capture_stanby_cancel.current) << '\n';
    tout << "CustomWB Capture Operation: " << format_customwb_capture_operation(m_prop.customwb_capture_operation.current) << '\n';
    tout << "CustomWB Capture Execution State : " << format_customwb_capture_execution_state(m_prop.customwb_capture_execution_state.current) << '\n';
    if (m_prop.customwb_capture_operation.current == 1) {
        state = true;
    }
    return state;
}

void CameraDevice::get_zoom_operation()
{
    load_properties();
    tout << "Zoom Operation Status: " << format_zoom_operation_status(m_prop.zoom_operation_status.current) << '\n';
    tout << "Zoom Setting Type: " << format_zoom_setting_type(m_prop.zoom_setting_type.current) << '\n';
    tout << "Zoom Type Status: " << format_zoom_types_status(m_prop.zoom_types_status.current) << '\n';
    tout << "Zoom Operation: " << format_zoom_operation(m_prop.zoom_operation.current) << '\n';

    // Zoom Speed Range is not supported
    if (m_prop.zoom_speed_range.possible.size() < 2) {
        tout << "Zoom Speed Range: -1 to 1" << std::endl 
             << "Zoom Speed Type: " << format_remocon_zoom_speed_type(m_prop.remocon_zoom_speed_type.current) << std::endl;
    }
    else {
        tout << "Zoom Speed Range: " << (int)m_prop.zoom_speed_range.possible.at(0) << " to " << (int)m_prop.zoom_speed_range.possible.at(1) << std::endl
             << "Zoom Speed Type: " << format_remocon_zoom_speed_type(m_prop.remocon_zoom_speed_type.current) << std::endl;
    }

    std::int32_t nprop = 0;
    SDK::CrDeviceProperty* prop_list = nullptr;
    CrInt32u getCode = SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Bar_Information;
    auto status = SDK::GetSelectDeviceProperties(m_device_handle, 1, &getCode, &prop_list, &nprop);

    if (CR_FAILED(status)) {
        tout << "Failed to get Zoom Bar Information.\n";
        return;
    }

    if (prop_list && 0 < nprop) {
        auto prop = prop_list[0];
        if (SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Bar_Information == prop.GetCode())
        {
            tout << "Zoom Bar Information: 0x" << std::hex << prop.GetCurrentValue() << std::dec << '\n';
        }
        SDK::ReleaseDeviceProperties(m_device_handle, prop_list);
    }
}

void CameraDevice::get_remocon_zoom_speed_type()
{
    load_properties();
    tout << "Zoom Speed Type: " << format_remocon_zoom_speed_type(m_prop.remocon_zoom_speed_type.current) << '\n';
}

void CameraDevice::set_aperture()
{
    if (!m_prop.f_number.writable) {
        // Not a settable property
        tout << "Aperture is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Aperture value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Aperture value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.f_number.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_f_number(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Aperture value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_FNumber);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_iso()
{
    if (!m_prop.iso_sensitivity.writable) {
        // Not a settable property
        tout << "ISO is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new ISO value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new ISO value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.iso_sensitivity.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_iso_sensitivity(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new ISO value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_IsoSensitivity);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

bool CameraDevice::set_save_info() const
{
#if defined(__APPLE__)
    text_char path[255]; /*MAX_PATH*/
    getcwd(path, sizeof(path) -1);

    auto save_status = SDK::SetSaveInfo(m_device_handle
        , path, (char*)"", ImageSaveAutoStartNo);
#else
    text path = fs::current_path().native();
    tout << path.data() << '\n';

    auto save_status = SDK::SetSaveInfo(m_device_handle
        , const_cast<text_char*>(path.data()), TEXT(""), ImageSaveAutoStartNo);
#endif
    if (CR_FAILED(save_status)) {
        tout << "Failed to set save path.\n";
        return false;
    }
    return true;
}

void CameraDevice::set_shutter_speed()
{
    if (!m_prop.shutter_speed.writable) {
        // Not a settable property
        tout << "Shutter Speed is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Shutter Speed value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Shutter Speed value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.shutter_speed.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_shutter_speed(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Shutter Speed value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_ShutterSpeed);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_position_key_setting()
{
    if (!m_prop.position_key_setting.writable) {
        // Not a settable property
        tout << "Position Key Setting is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Position Key Setting value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Position Key Setting value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.position_key_setting.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_position_key_setting(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Position Key Setting value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_PriorityKeySettings);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt8Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_exposure_program_mode()
{
    if (!m_prop.exposure_program_mode.writable) {
        // Not a settable property
        tout << "Exposure Program Mode is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Exposure Program Mode value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Exposure Program Mode value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.exposure_program_mode.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_exposure_program_mode(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Exposure Program Mode value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_ExposureProgramMode);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_still_capture_mode()
{
    if (!m_prop.still_capture_mode.writable) {
        // Not a settable property
        tout << "Still Capture Mode is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Still Capture Mode value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Still Capture Mode value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.still_capture_mode.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_still_capture_mode(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Still Capture Mode value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_DriveMode);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_focus_mode()
{
    if (!m_prop.focus_mode.writable) {
        // Not a settable property
        tout << "Focus Mode is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Focus Mode value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Focus Mode value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.focus_mode.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_focus_mode(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Focus Mode value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_FocusMode);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_focus_area()
{
    if (!m_prop.focus_area.writable) {
        // Not a settable property
        tout << "Focus Area is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Focus Area value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Focus Area value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.focus_area.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_focus_area(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Focus Area value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_FocusArea);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_live_view_image_quality()
{
    if (!m_prop.live_view_image_quality.writable) {
        // Not a settable property
        tout << "Live View Image Quality is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Live View Image Quality value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Live View Image Quality value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.live_view_image_quality.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_live_view_image_quality(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Live View Image Quality value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_LiveView_Image_Quality);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::set_live_view_status()
{
    if (!m_prop.live_view_status.writable) {
        // Not a settable property
        tout << "Live View Status is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new Live View Image Quality value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new Live View Status value:\n";
    tout << "[-1] Cancel input\n";

    tout << '[' << 1 << "] Disabled" << '\n';
    tout << '[' << 2 << "] Enabled" << '\n';

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new Live View Image Quality value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || 2 < selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_LiveViewStatus);
    prop.SetCurrentValue(selected_index);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt8);

    SDK::SetDeviceProperty(m_device_handle, &prop);

    get_live_view_status();
}

void CameraDevice::set_white_balance()
{
    if (!m_prop.white_balance.writable) {
        // Not a settable property
        tout << "White Balance is not writable\n";
        return;
    }

    text input;
    tout << std::endl << "Would you like to set a new White Balance value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << std::endl << "Choose a number set a new White Balance value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.white_balance.possible;
    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_white_balance(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << std::endl << "Choose a number set a new White Balance value:\n";

    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_WhiteBalance);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::execute_lock_property(CrInt16u code)
{
    load_properties();

    text input;
    tout << std::endl << "Would you like to execute Unlock or Lock? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip execute a new value.\n";
        return;
    }

    tout << std::endl << "Choose a number :\n";
    tout << "[-1] Cancel input\n";

    tout << "[1] Unlock" << '\n';
    tout << "[2] Lock" << '\n';

    tout << "[-1] Cancel input\n";
    tout << "Choose a number :\n";

    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    CrInt64u ptpValue = 0;
    switch (selected_index) {
    case 1:
        ptpValue = SDK::CrLockIndicator::CrLockIndicator_Unlocked;
        break;
    case 2:
        ptpValue = SDK::CrLockIndicator::CrLockIndicator_Locked;
        break;
    default:
        selected_index = -1;
        break;
    }

    if (-1 == selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(code);
    prop.SetCurrentValue((CrInt64u)(ptpValue));
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::get_af_area_position()
{
    CrInt32 num = 0;
    SDK::CrLiveViewProperty* lvProperty = nullptr;
    CrInt32u getCode = SDK::CrLiveViewPropertyCode::CrLiveViewProperty_AF_Area_Position;
    auto err = SDK::GetSelectLiveViewProperties(m_device_handle, 1, &getCode, &lvProperty, &num);
    if (CR_FAILED(err)) {
        tout << "Failed to get AF Area Position [LiveViewProperties]\n";
        return;
    }

    if (lvProperty && 1 == num) {
        // Got AF Area Position
        auto prop = lvProperty[0];
        if (SDK::CrFrameInfoType::CrFrameInfoType_FocusFrameInfo == prop.GetFrameInfoType()) {
            int sizVal = prop.GetValueSize();
            int count = sizVal / sizeof(SDK::CrFocusFrameInfo);
            SDK::CrFocusFrameInfo* pFrameInfo = (SDK::CrFocusFrameInfo*)prop.GetValue();
            if (0 == sizVal || nullptr == pFrameInfo) {
                printf("  FocusFrameInfo nothing\n");
            }
            else {
                for (std::int32_t fram = 0; fram < count; ++fram) {
                    auto lvprop = pFrameInfo[fram];
                    char buff[512];
                    memset(buff, 0, sizeof(buff));
                    sprintf(buff, "  FocusFrameInfo no[%d] pri[%d] w[%d] h[%d] Deno[%d-%d] Nume[%d-%d]",
                        fram + 1,
                        lvprop.priority,
                        lvprop.width, lvprop.height,
                        lvprop.xDenominator, lvprop.yDenominator,
                        lvprop.xNumerator, lvprop.yNumerator);
                    tout << buff << std::endl;
                }
            }
        }
        SDK::ReleaseLiveViewProperties(m_device_handle, lvProperty);
    }
}

void CameraDevice::set_af_area_position()
{
    load_properties();
    // Set, FocusArea property
    tout << "Set FocusArea to Flexible_Spot_S\n";
    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_FocusArea);
    prop.SetCurrentValue(SDK::CrFocusArea::CrFocusArea_Flexible_Spot_S);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);
    auto err_prop = SDK::SetDeviceProperty(m_device_handle, &prop);
    if (CR_FAILED(err_prop)) {
        tout << "FocusArea FAILED\n";
        return;
    }
    else {
        tout << "FocusArea SUCCESS\n";
    }

    std::this_thread::sleep_for(500ms);

    this->get_af_area_position();

    execute_pos_xy(SDK::CrDevicePropertyCode::CrDeviceProperty_AF_Area_Position);
}

void CameraDevice::set_select_media_format()
{
    bool validQuickFormat = false;
    SDK::CrCommandId ptpFormatType = SDK::CrCommandId::CrCommandId_MediaFormat;

    if ((SDK::CrMediaFormat::CrMediaFormat_Disable == m_prop.media_slot1_full_format_enable_status.current) &&
        (SDK::CrMediaFormat::CrMediaFormat_Disable == m_prop.media_slot2_full_format_enable_status.current)) {
            // Not a settable property
        tout << std::endl << "Slot1 and Slot2 is can not format\n";
        return;
    }

    if ((m_prop.media_slot1_quick_format_enable_status.writable || m_prop.media_slot2_quick_format_enable_status.writable)
        &&
         ((SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot1_quick_format_enable_status.current) ||
          (SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot2_quick_format_enable_status.current))) {
            validQuickFormat = true;
    }

    text input;
    tout << std::endl << "Would you like to format the media? (y/n):" << std::endl;
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip format.\n";
        return;
    }

    // Full or Quick
    if (validQuickFormat) {
        tout << "Choose a format type number : " << std::endl;
        tout << "[-1] Cancel input" << std::endl;
        tout << "[1] Full Format" << std::endl;
        tout << "[2] Quick Format" << std::endl;

        tout << std::endl << "input> ";
        std::getline(tin, input);
        text_stringstream sstype(input);
        int selected_type = 0;
        sstype >> selected_type;

        if ((selected_type < 1) || (2 < selected_type)) {
            tout << "Input cancelled.\n";
            return;
        }

        if (2 == selected_type) {
            ptpFormatType = SDK::CrCommandId::CrCommandId_MediaQuickFormat;
        }
    }

    tout << std::endl << "Choose a number Which media do you want to format ? \n";
    tout << "[-1] Cancel input\n";

    tout << "[1] SLOT1" << '\n';
    tout << "[2] SLOT2" << '\n';

    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if ((selected_index < 1) || (2 < selected_index)) {
        tout << "Input cancelled.\n";
        return;
    }

    CrInt64u ptpValue = 0xFFFF;
    if (SDK::CrCommandId::CrCommandId_MediaQuickFormat == ptpFormatType) {
        if ((1 == selected_index) && (SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot1_quick_format_enable_status.current)) {
        ptpValue = SDK::CrCommandParam::CrCommandParam_Up;
        }
        else if ((2 == selected_index) && (SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot2_quick_format_enable_status.current)) {
            ptpValue = SDK::CrCommandParam::CrCommandParam_Down;
        }
    }
    else
    {
        if ((1 == selected_index) && (SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot1_full_format_enable_status.current)) {
            ptpValue = SDK::CrCommandParam::CrCommandParam_Up;
        }
        else if ((2 == selected_index) && (SDK::CrMediaFormat::CrMediaFormat_Enable == m_prop.media_slot2_full_format_enable_status.current)) {
            ptpValue = SDK::CrCommandParam::CrCommandParam_Down;
        }
    }

    if (0xFFFF == ptpValue)
    {
        tout << std::endl << "The Selected slot cannot be formatted.\n";
        return;
    }

    tout << std::endl << "All data will be deleted.Is it OK ? (y/n) \n";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip format.\n";
        return;
    }

    SDK::SendCommand(m_device_handle, ptpFormatType, (SDK::CrCommandParam)ptpValue);

    tout << std::endl << "Formatting .....\n";

    int startflag = 0;
    CrInt32u getCodes = SDK::CrDevicePropertyCode::CrDeviceProperty_Media_FormatProgressRate;

    std::int32_t nprop = 0;
    SDK::CrDeviceProperty* prop_list = nullptr;

    // check of progress
    while (true)
    {
        auto status = SDK::GetSelectDeviceProperties(m_device_handle, 1, &getCodes, &prop_list, &nprop);
        if (CR_FAILED(status)) {
            tout << "Failed to get Media FormatProgressRate.\n";
            return;
        }
        if (prop_list && 1 == nprop) {
            auto prop = prop_list[0];
        
            if (getCodes == prop.GetCode())
            {
                if ((0 == startflag) && (0 < prop.GetCurrentValue()))
                {
                    startflag = 1;
                }
                if ((1 == startflag) && (0 == prop.GetCurrentValue()))
                {
                    tout << std::endl << "Format completed " << '\n';
                    SDK::ReleaseDeviceProperties(m_device_handle, prop_list);
                    prop_list = nullptr;
                    break;
                }
                tout << "\r" << "FormatProgressRate:" << prop.GetCurrentValue();
            }
        }
        std::this_thread::sleep_for(250ms);
        SDK::ReleaseDeviceProperties(m_device_handle, prop_list);
        prop_list = nullptr;
    }
}

void CameraDevice::execute_movie_rec()
{
    load_properties();

    text input;
    tout << std::endl << "Operate the movie recording button ? (y/n):";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip .\n";
        return;
    }

    tout << "Choose a number :\n";
    tout << "[-1] Cancel input\n";

    tout << "[1] Up" << '\n';
    tout << "[2] Down" << '\n';

    tout << "[-1] Cancel input\n";
    tout << "Choose a number :\n";

    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0) {
        tout << "Input cancelled.\n";
        return;
    }

    CrInt64u ptpValue = 0;
    switch (selected_index) {
    case 1:
        ptpValue = SDK::CrCommandParam::CrCommandParam_Up;
        break;
    case 2:
        ptpValue = SDK::CrCommandParam::CrCommandParam_Down;
        break;
    default:
        selected_index = -1;
        break;
    }

    if (-1 == selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::SendCommand(m_device_handle, SDK::CrCommandId::CrCommandId_MovieRecord, (SDK::CrCommandParam)ptpValue);

}

void CameraDevice::set_custom_wb()
{
    // Set, PriorityKeySettings property
    tout << std::endl << "Set camera to PC remote";
    SDK::CrDeviceProperty priority;
    priority.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_PriorityKeySettings);
    priority.SetCurrentValue(SDK::CrPriorityKeySettings::CrPriorityKey_PCRemote);
    priority.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);
    auto err_priority = SDK::SetDeviceProperty(m_device_handle, &priority);
    if (CR_FAILED(err_priority)) {
        tout << "Priority Key setting FAILED\n";
        return;
    }
    else {
        tout << "Priority Key setting SUCCESS\n";
    }
    std::this_thread::sleep_for(500ms);
    get_position_key_setting();

    // Set, ExposureProgramMode property
    tout << std::endl << "Set the Exposure Program mode to P mode";
    SDK::CrDeviceProperty expromode;
    expromode.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_ExposureProgramMode);
    expromode.SetCurrentValue(SDK::CrExposureProgram::CrExposure_P_Auto);
    expromode.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);
    auto err_expromode = SDK::SetDeviceProperty(m_device_handle, &expromode);
    if (CR_FAILED(err_expromode)) {
        tout << "Exposure Program mode FAILED\n";
        return;
    }
    else {
        tout << "Exposure Program mode SUCCESS\n";
    }
    std::this_thread::sleep_for(500ms);
    get_exposure_program_mode();

    // Set, White Balance property
    tout << std::endl << "Set the White Balance to Custom1\n";
    SDK::CrDeviceProperty wb;
    wb.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_WhiteBalance);
    wb.SetCurrentValue(SDK::CrWhiteBalanceSetting::CrWhiteBalance_Custom_1);
    wb.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);
    auto err_wb = SDK::SetDeviceProperty(m_device_handle, &wb);
    if (CR_FAILED(err_wb)) {
        tout << "White Balance FAILED\n";
        return;
    }
    else {
        tout << "White Balance SUCCESS\n";
    }
    std::this_thread::sleep_for(2000ms);
    get_white_balance();

    // Set, custom WB capture standby 
    tout << std::endl << "Set custom WB capture standby " << std::endl;

    bool execStat = false;
    int i = 0;
    while ((false == execStat)&&(i < 5))
    {
        execute_downup_property(SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture_Standby);
        std::this_thread::sleep_for(1000ms);
        tout << std::endl;
        execStat = get_custom_wb();
        i++;

    }

    if (false == execStat)
    {
        tout << std::endl << "CustomWB Capture Standby FAILED\n";
        return;
    }

    // Set, custom WB capture 
    tout << std::endl << "Set custom WB capture ";
    execute_pos_xy(SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture);

    std::this_thread::sleep_for(5000ms);

    // Set, custom WB capture standby cancel 
    text input;
    tout << std::endl << "Set custom WB capture standby cancel. Please enter something. " << std::endl;
    std::getline(tin, input);
    if (0 == input.size() || 0 < input.size()) {
        execute_downup_property(SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture_Standby_Cancel);
        get_custom_wb();
        tout << std::endl << "Finish custom WB capture\n";
    }
    else
    {
        tout << std::endl << "Did not finish normally\n";
    }
}

void CameraDevice::set_zoom_operation()
{
    load_properties();

    text input;
    tout << std::endl << "Operate the zoom ? (y/n):";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip .\n";
        return;
    }

    while (true)
    {
        CrInt64 ptpValue = 0;
        bool cancel = false;

        // Zoom Speed Range is not supported
        if (m_prop.zoom_speed_range.possible.size() < 2) {
            tout << std::endl << "Choose a number :\n";
            tout << "[-1] Cancel input\n";

            tout << "[0] Stop" << '\n';
            tout << "[1] Wide" << '\n';
            tout << "[2] Tele" << '\n';

            tout << "[-1] Cancel input\n";
            tout << "Choose a number :\n";

            tout << std::endl << "input> ";
            std::getline(tin, input);
            text_stringstream ss(input);
            int selected_index = 0;
            ss >> selected_index;

            switch (selected_index) {
            case 0:
                ptpValue = SDK::CrZoomOperation::CrZoomOperation_Stop;
                break;
            case 1:
                ptpValue = SDK::CrZoomOperation::CrZoomOperation_Wide;
                break;
            case 2:
                ptpValue = SDK::CrZoomOperation::CrZoomOperation_Tele;
                break;
            default:
                tout << "Input cancelled.\n";
                return;
                break;
            }
        }
        else{
            tout << std::endl << "Set the value of zoom speed (Out-of-range value to Cancel) :\n";
            tout << std::endl << "input> ";
            std::getline(tin, input);
            text_stringstream ss(input);
            int input_value = 0;
            ss >> input_value;

            //Stop zoom and return to the top menu when out-of-range values or non-numeric values are entered
            if (((input_value == 0) && (input != TEXT("0"))) || (input_value < (int)m_prop.zoom_speed_range.possible.at(0)) || ((int)m_prop.zoom_speed_range.possible.at(1) < input_value))
            {
                cancel = true;
                ptpValue = SDK::CrZoomOperation::CrZoomOperation_Stop;
                tout << "Input cancelled.\n";
            }
            else {
                ptpValue = (CrInt64)input_value;
            }
        }

        SDK::CrDeviceProperty prop;
        prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Operation);
        prop.SetCurrentValue((CrInt64u)ptpValue);
        prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);
        SDK::SetDeviceProperty(m_device_handle, &prop);
        if (cancel == true) {
            return;
        }
        get_zoom_operation();
    }
}

void CameraDevice::set_remocon_zoom_speed_type()
{
    if (!m_prop.remocon_zoom_speed_type.writable) {
        // Not a settable property
        tout << "Zoom speed type is not writable\n";
        return;
    }

    text input;
    tout << "Would you like to set a new zoom speed type value? (y/n): ";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip setting a new value.\n";
        return;
    }

    tout << "Choose a number set a new zoom speed type value:\n";
    tout << "[-1] Cancel input\n";

    auto& values = m_prop.remocon_zoom_speed_type.possible;

    for (std::size_t i = 0; i < values.size(); ++i) {
        tout << '[' << i << "] " << format_remocon_zoom_speed_type(values[i]) << '\n';
    }

    tout << "[-1] Cancel input\n";
    tout << "Choose a number set a new zoom speed type value:\n";

    tout << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    if (selected_index < 0 || values.size() <= selected_index) {
        tout << "Input cancelled.\n";
        return;
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(SDK::CrDevicePropertyCode::CrDeviceProperty_Remocon_Zoom_Speed_Type);
    prop.SetCurrentValue(values[selected_index]);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt32Array);

    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::execute_downup_property(CrInt16u code)
{
    SDK::CrDeviceProperty prop;
    prop.SetCode(code);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt16Array);

    // Down
    prop.SetCurrentValue(SDK::CrPropertyCustomWBCaptureButton::CrPropertyCustomWBCapture_Down);
    SDK::SetDeviceProperty(m_device_handle, &prop);

    std::this_thread::sleep_for(500ms);

    // Up
    prop.SetCurrentValue(SDK::CrPropertyCustomWBCaptureButton::CrPropertyCustomWBCapture_Up);
    SDK::SetDeviceProperty(m_device_handle, &prop);

    std::this_thread::sleep_for(500ms);
}

void CameraDevice::execute_pos_xy(CrInt16u code)
{
    load_properties();

    text input;
    tout << std::endl << "Change position ? (y/n):";
    std::getline(tin, input);
    if (input != TEXT("y")) {
        tout << "Skip.\n";
        return;
    }

    tout << std::endl << "Set the value of X (decimal)" << std::endl;
    tout << "Regarding details of usage, please check API doc." << std::endl;

    tout << std::endl << "input X> ";
    std::getline(tin, input);
    text_stringstream ss1(input);
    CrInt32u x = 0;
    ss1 >> x;

    if (x < 0 || x > 639) {
        tout << "Input cancelled.\n";
        return;
    }

    tout << "input X = " << x << '\n';

    std::this_thread::sleep_for(1000ms);

    tout << std::endl << "Set the value of Y (decimal)" << std::endl;

    tout << std::endl << "input Y> ";
    std::getline(tin, input);
    text_stringstream ss2(input);
    CrInt32u y = 0;
    ss2 >> y;

    if (y < 0 || y > 479 ) {
        tout << "Input cancelled.\n";
        return;
    }

    tout << "input Y = "<< y << '\n';

    std::this_thread::sleep_for(1000ms);

    int x_y = x << 16 | y;

    tout << std::endl << "input X_Y = 0x" << std::hex << x_y << std::dec << '\n';

    SDK::CrDeviceProperty prop;
    prop.SetCode(code);
    prop.SetCurrentValue((CrInt64u)x_y);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt32);

    SDK::SetDeviceProperty(m_device_handle, &prop);
}

void CameraDevice::execute_preset_focus()
{
    load_properties();

    auto& values_save = m_prop.save_zoom_and_focus_position.possible;
    auto& values_load = m_prop.load_zoom_and_focus_position.possible;

    if ((!m_prop.save_zoom_and_focus_position.writable) &&
        (!m_prop.load_zoom_and_focus_position.writable)){
        // Not a settable property
        tout << "Preset Focus is not supported.\n";
        return;
    }

    tout << std::endl << "Save Zoom and Focus Position Enable Preset number: " << std::endl;
    for (int i = 0; i < values_save.size(); i++)
    {
        tout << " " << (int)values_save.at(i) << std::endl;
    }

    tout << std::endl << "Load Zoom and Focus Position Enable Preset number: " << std::endl;
    for (int i = 0; i < values_load.size(); i++)
    {
        tout << " " << (int)values_load.at(i) << std::endl;
    }

    tout << std::endl << "Set the value of operation :\n";
    tout << "[-1] Cancel input\n";

    tout << "[1] Save Zoom and Focus Position\n";
    tout << "[2] Load Zoom and Focus Position\n";

    tout << "[-1] Cancel input\n";
    tout << "Choose a number :\n";

    text input;
    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss(input);
    int selected_index = 0;
    ss >> selected_index;

    CrInt32u code = 0;
    if ((1 == selected_index) && (m_prop.save_zoom_and_focus_position.writable)) {
        code = SDK::CrDevicePropertyCode::CrDeviceProperty_ZoomAndFocusPosition_Save;
    }
    else if ((2 == selected_index) && (m_prop.load_zoom_and_focus_position.writable)) {
        code = SDK::CrDevicePropertyCode::CrDeviceProperty_ZoomAndFocusPosition_Load;
    }
    else {
        tout << "The Selected operation is not supported.\n";
        return;
    }

    tout << "Set the value of Preset number :\n";

    tout << std::endl << "input> ";
    std::getline(tin, input);
    text_stringstream ss_slot(input);
    int input_value = 0;
    ss_slot >> input_value;

    if (code == SDK::CrDevicePropertyCode::CrDeviceProperty_ZoomAndFocusPosition_Save) {
        if (find(values_save.begin(), values_save.end(), input_value) == values_save.end()) {
            tout << "Input cancelled.\n";
            return;
        }
    }
    else {
        if (find(values_load.begin(), values_load.end(), input_value) == values_load.end()) {
            tout << "Input cancelled.\n";
            return;
        }
    }

    SDK::CrDeviceProperty prop;
    prop.SetCode(code);
    prop.SetCurrentValue(input_value);
    prop.SetValueType(SDK::CrDataType::CrDataType_UInt8);
    SDK::SetDeviceProperty(m_device_handle, &prop);
}
void CameraDevice::change_live_view_enable()
{
    m_lvEnbSet = !m_lvEnbSet;
    SDK::SetDeviceSetting(m_device_handle, SDK::Setting_Key_EnableLiveView, (CrInt32u)m_lvEnbSet);
}

bool CameraDevice::is_connected() const
{
    return m_connected.load();
}

std::uint32_t CameraDevice::ip_address() const
{
    if (m_conn_type == ConnectionType::NETWORK)
        return m_net_info.ip_address;
    return 0;
}

text CameraDevice::ip_address_fmt() const
{
    if (m_conn_type == ConnectionType::NETWORK)
    {
        return m_net_info.ip_address_fmt;
    }
    return text(TEXT("N/A"));
}

text CameraDevice::mac_address() const
{
    if (m_conn_type == ConnectionType::NETWORK)
        return m_net_info.mac_address;
    return text(TEXT("N/A"));
}

std::int16_t CameraDevice::pid() const
{
    if (m_conn_type == ConnectionType::USB)
        return m_usb_info.pid;
    return 0;
}

text CameraDevice::get_id()
{
    if (ConnectionType::NETWORK == m_conn_type) {
        return m_net_info.mac_address;
    }
    else
        return text((TCHAR*)m_info->GetId());
}

void CameraDevice::OnConnected(SDK::DeviceConnectionVersioin version)
{
    m_connected.store(true);
    text id(this->get_id());
    tout << "Connected to " << m_info->GetModel() << " (" << id.data() << ")\n";
}

void CameraDevice::OnDisconnected(CrInt32u error)
{
    m_connected.store(false);
    text id(this->get_id());
    tout << "Disconnected from " << m_info->GetModel() << " (" << id.data() << ")\n";
    if ((false == m_spontaneous_disconnection) && (SDK::CrSdkControlMode_ContentsTransfer == m_modeSDK))
    {
        tout << "Please input '0' to return to the TOP-MENU\n";
    }
}

void CameraDevice::OnPropertyChanged()
{
    // tout << "Property changed.\n";
}

void CameraDevice::OnLvPropertyChanged()
{
    // tout << "LvProperty changed.\n";
}

void CameraDevice::OnCompleteDownload(CrChar* filename)
{
    text file(filename);
    tout << "Complete download. File: " << file.data() << '\n';
}

void CameraDevice::OnNotifyContentsTransfer(CrInt32u notify, SDK::CrContentHandle contentHandle, CrChar* filename)
{
    // Start
    if (SDK::CrNotify_ContentsTransfer_Start == notify)
    {
        tout << "[START] Contents Handle: 0x " << std::hex << contentHandle << std::dec << std::endl;
    }
    // Complete
    else if (SDK::CrNotify_ContentsTransfer_Complete == notify)
    {
        text file(filename);
        tout << "[COMPLETE] Contents Handle: 0x" << std::hex << contentHandle << std::dec << ", File: " << file.data() << std::endl;
    }
    // Other
    else
    {
        text msg = get_message_desc(notify);
        if (msg.empty()) {
            tout << "[-] Content transfer failure. 0x" << std::hex << notify << ", handle: 0x" << contentHandle << std::dec << std::endl;
        } else {
            tout << "[-] Content transfer failure. handle: 0x" << std::hex << contentHandle  << std::dec << std::endl << "    -> ";
            tout << msg.data() << std::endl;
        }
    }
}

void CameraDevice::OnWarning(CrInt32u warning)
{
    text id(this->get_id());
    if (SDK::CrWarning_Connect_Reconnecting == warning) {
        tout << "Device Disconnected. Reconnecting... " << m_info->GetModel() << " (" << id.data() << ")\n";
        return;
    }
    switch (warning)
    {
    case SDK::CrWarning_ContentsTransferMode_Invalid:
    case SDK::CrWarning_ContentsTransferMode_DeviceBusy:
    case SDK::CrWarning_ContentsTransferMode_StatusError:
        tout << "\nThe camera is in a condition where it cannot transfer content.\n\n";
        tout << "Please input '0' to return to the TOP-MENU and connect again.\n";
        break;
    case SDK::CrWarning_ContentsTransferMode_CanceledFromCamera:
        tout << "\nContent transfer mode canceled.\n";
        tout << "If you want to continue content transfer, input '0' to return to the TOP-MENU and connect again.\n\n";
        break;
    default:
        return;
    }
}

void CameraDevice::OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes)
{
    //tout << "Property changed.  num = " << std::dec << num;
    //tout << std::hex;
    //for (std::int32_t i = 0; i < num; ++i)
    //{
    //    tout << ", 0x" << codes[i];
    //}
    //tout << std::endl << std::dec;
    load_properties(num, codes);
}

void CameraDevice::OnLvPropertyChangedCodes(CrInt32u num, CrInt32u* codes)
{
    //tout << "LvProperty changed.  num = " << std::dec << num;
    //tout << std::hex;
    //for (std::int32_t i = 0; i < num; ++i)
    //{
    //    tout << ", 0x" << codes[i];
    //}
    //tout << std::endl;
#if 0 
    SDK::CrLiveViewProperty* lvProperty = nullptr;
    int32_t nprop = 0;
    SDK::CrError err = SDK::GetSelectLiveViewProperties(m_device_handle, num, codes, &lvProperty, &nprop);
    if (CR_SUCCEEDED(err) && lvProperty) {
        for (int32_t i=0 ; i<nprop ; i++) {
            auto prop = lvProperty[i];
            if (SDK::CrFrameInfoType::CrFrameInfoType_FocusFrameInfo == prop.GetFrameInfoType()) {
                int sizVal = prop.GetValueSize();
                int count = sizVal / sizeof(SDK::CrFocusFrameInfo);
                SDK::CrFocusFrameInfo* pFrameInfo = (SDK::CrFocusFrameInfo*)prop.GetValue();
                if (0 == sizVal || nullptr == pFrameInfo) {
                    printf("  FocusFrameInfo nothing\n");
                }
                else {
                    for (std::int32_t fram = 0; fram < count; ++fram) {
                        auto lvprop = pFrameInfo[fram];
                        char buff[512];
                        memset(buff, 0, sizeof(buff));
                        sprintf(buff, "  FocusFrameInfo no[%d] pri[%d] w[%d] h[%d] Deno[%d-%d] Nume[%d-%d]",
                            fram + 1,
                            lvprop.priority,
                            lvprop.width, lvprop.height,
                            lvprop.xDenominator, lvprop.yDenominator,
                            lvprop.xNumerator, lvprop.yNumerator);
                        tout << buff << std::endl;
                    }
                }
            }
            else if (SDK::CrFrameInfoType::CrFrameInfoType_Magnifier_Position == prop.GetFrameInfoType()) {
                int sizVal = prop.GetValueSize();
                int count = sizVal / sizeof(SDK::CrMagPosInfo);
                SDK::CrMagPosInfo* pMagPosInfo = (SDK::CrMagPosInfo*)prop.GetValue();
                if (0 == sizVal || nullptr == pMagPosInfo) {
                    printf("  MagPosInfo nothing\n");
                }
                else {
                    char buff[512];
                    memset(buff, 0, sizeof(buff));
                    sprintf(buff, "  MagPosInfo w[%d] h[%d] Deno[%d-%d] Nume[%d-%d]",
                        pMagPosInfo->width, pMagPosInfo->height,
                        pMagPosInfo->xDenominator, pMagPosInfo->yDenominator,
                        pMagPosInfo->xNumerator, pMagPosInfo->yNumerator);
                    tout << buff << std::endl;
                }
            }
        }
        SDK::ReleaseLiveViewProperties(m_device_handle, lvProperty);
    }
#endif
    tout << std::dec;
}

void CameraDevice::OnError(CrInt32u error)
{
    text id(this->get_id());
    text msg = get_message_desc(error);
    if (!msg.empty()) {
        // output is 2 line
        tout << std::endl << msg.data() << std::endl;
        tout << m_info->GetModel() << " (" << id.data() << ")" << std::endl;
        if (SDK::CrError_Connect_TimeOut == error) {
            // append 1 line
            tout << "Please input '0' after Connect camera" << std::endl;
            return;
        }
        if (SDK::CrError_Connect_Disconnected == error)
        {
            return;
        }
        tout << "Please input '0' to return to the TOP-MENU\n";
    }
}

void CameraDevice::load_properties(CrInt32u num, CrInt32u* codes)
{
    std::int32_t nprop = 0;
    SDK::CrDeviceProperty* prop_list = nullptr;

    m_prop.media_slot1_quick_format_enable_status.writable = false;
    m_prop.media_slot2_quick_format_enable_status.writable = false;

    SDK::CrError status = SDK::CrError_Generic;
    if (0 == num){
        // Get all
        status = SDK::GetDeviceProperties(m_device_handle, &prop_list, &nprop);
    }
    else {
        // Get difference
        status = SDK::GetSelectDeviceProperties(m_device_handle, num, codes, &prop_list, &nprop);
    }

    if (CR_FAILED(status)) {
        tout << "Failed to get device properties.\n";
        return;
    }

    if (prop_list && nprop > 0) {
        // Got properties list
        for (std::int32_t i = 0; i < nprop; ++i) {
            auto prop = prop_list[i];
            int nval = 0;

            switch (prop.GetCode()) {
            case SDK::CrDevicePropertyCode::CrDeviceProperty_SdkControlMode:
                m_prop.sdk_mode.writable = prop.IsSetEnableCurrentValue();
                m_prop.sdk_mode.current = static_cast<std::uint32_t>(prop.GetCurrentValue());
                m_modeSDK = (SDK::CrSdkControlMode)m_prop.sdk_mode.current;
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_FNumber:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.f_number.writable = prop.IsSetEnableCurrentValue();
                m_prop.f_number.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_f_number(prop.GetValues(), nval);
                    m_prop.f_number.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_IsoSensitivity:
                nval = prop.GetValueSize() / sizeof(std::uint32_t);
                m_prop.iso_sensitivity.writable = prop.IsSetEnableCurrentValue();
                m_prop.iso_sensitivity.current = static_cast<std::uint32_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_iso_sensitivity(prop.GetValues(), nval);
                    m_prop.iso_sensitivity.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_ShutterSpeed:
                nval = prop.GetValueSize() / sizeof(std::uint32_t);
                m_prop.shutter_speed.writable = prop.IsSetEnableCurrentValue();
                m_prop.shutter_speed.current = static_cast<std::uint32_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_shutter_speed(prop.GetValues(), nval);
                    m_prop.shutter_speed.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_PriorityKeySettings:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.position_key_setting.writable = prop.IsSetEnableCurrentValue();
                m_prop.position_key_setting.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (nval != m_prop.position_key_setting.possible.size()) {
                    auto parsed_values = parse_position_key_setting(prop.GetValues(), nval);
                    m_prop.position_key_setting.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_ExposureProgramMode:
                nval = prop.GetValueSize() / sizeof(std::uint32_t);
                m_prop.exposure_program_mode.writable = prop.IsSetEnableCurrentValue();
                m_prop.exposure_program_mode.current = static_cast<std::uint32_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_exposure_program_mode(prop.GetValues(), nval);
                    m_prop.exposure_program_mode.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_DriveMode:
                nval = prop.GetValueSize() / sizeof(std::uint32_t);
                m_prop.still_capture_mode.writable = prop.IsSetEnableCurrentValue();
                m_prop.still_capture_mode.current = static_cast<std::uint32_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_still_capture_mode(prop.GetValues(), nval);
                    m_prop.still_capture_mode.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_FocusMode:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.focus_mode.writable = prop.IsSetEnableCurrentValue();
                m_prop.focus_mode.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_focus_mode(prop.GetValues(), nval);
                    m_prop.focus_mode.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_FocusArea:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.focus_area.writable = prop.IsSetEnableCurrentValue();
                m_prop.focus_area.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_focus_area(prop.GetValues(), nval);
                    m_prop.focus_area.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_LiveView_Image_Quality:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.live_view_image_quality.writable = prop.IsSetEnableCurrentValue();
                m_prop.live_view_image_quality.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto view = parse_live_view_image_quality(prop.GetValues(), nval);
                    m_prop.live_view_image_quality.possible.swap(view);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_LiveViewStatus:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.live_view_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.live_view_status.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_MediaSLOT1_FormatEnableStatus:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.media_slot1_full_format_enable_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.media_slot1_full_format_enable_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.media_slot1_full_format_enable_status.possible.size()) {
                    auto parsed_values = parse_media_slotx_format_enable_status(prop.GetValues(), nval);
                    m_prop.media_slot1_full_format_enable_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_MediaSLOT2_FormatEnableStatus:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.media_slot2_full_format_enable_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.media_slot2_full_format_enable_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.media_slot2_full_format_enable_status.possible.size()) {
                    auto parsed_values = parse_media_slotx_format_enable_status(prop.GetValues(), nval);
                    m_prop.media_slot2_full_format_enable_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_MediaSLOT1_QuickFormatEnableStatus:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.media_slot1_quick_format_enable_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.media_slot1_quick_format_enable_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.media_slot1_quick_format_enable_status.possible.size()) {
                    auto parsed_values = parse_media_slotx_format_enable_status(prop.GetValues(), nval);
                    m_prop.media_slot1_quick_format_enable_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_MediaSLOT2_QuickFormatEnableStatus:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.media_slot2_quick_format_enable_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.media_slot2_quick_format_enable_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.media_slot2_quick_format_enable_status.possible.size()) {
                    auto parsed_values = parse_media_slotx_format_enable_status(prop.GetValues(), nval);
                    m_prop.media_slot2_quick_format_enable_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_WhiteBalance:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.white_balance.writable = prop.IsSetEnableCurrentValue();
                m_prop.white_balance.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_white_balance(prop.GetValues(), nval);
                    m_prop.white_balance.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture_Standby:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.customwb_capture_stanby.writable = prop.IsSetEnableCurrentValue();
                m_prop.customwb_capture_stanby.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (nval != m_prop.white_balance.possible.size()) {
                    auto parsed_values = parse_customwb_capture_stanby(prop.GetValues(), nval);
                    m_prop.customwb_capture_stanby.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture_Standby_Cancel:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.customwb_capture_stanby_cancel.writable = prop.IsSetEnableCurrentValue();
                m_prop.customwb_capture_stanby_cancel.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (nval != m_prop.customwb_capture_stanby_cancel.possible.size()) {
                    auto parsed_values = parse_customwb_capture_stanby_cancel(prop.GetValues(), nval);
                    m_prop.customwb_capture_stanby_cancel.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Capture_Operation:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.customwb_capture_operation.writable = prop.IsSetEnableCurrentValue();
                m_prop.customwb_capture_operation.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_customwb_capture_operation(prop.GetValues(), nval);
                    m_prop.customwb_capture_operation.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_CustomWB_Execution_State:
                nval = prop.GetValueSize() / sizeof(std::uint16_t);
                m_prop.customwb_capture_execution_state.writable = prop.IsSetEnableCurrentValue();
                m_prop.customwb_capture_execution_state.current = static_cast<std::uint16_t>(prop.GetCurrentValue());
                if (nval != m_prop.customwb_capture_execution_state.possible.size()) {
                    auto parsed_values = parse_customwb_capture_execution_state(prop.GetValues(), nval);
                    m_prop.customwb_capture_execution_state.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Operation_Status:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.zoom_operation_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.zoom_operation_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.zoom_operation_status.possible.size()) {
                    auto parsed_values = parse_zoom_operation_status(prop.GetValues(), nval);
                    m_prop.zoom_operation_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Setting:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.zoom_setting_type.writable = prop.IsSetEnableCurrentValue();
                m_prop.zoom_setting_type.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_zoom_setting_type(prop.GetValues(), nval);
                    m_prop.zoom_setting_type.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Type_Status:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.zoom_types_status.writable = prop.IsSetEnableCurrentValue();
                m_prop.zoom_types_status.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (nval != m_prop.zoom_types_status.possible.size()) {
                    auto parsed_values = parse_zoom_types_status(prop.GetValues(), nval);
                    m_prop.zoom_types_status.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Operation:
                nval = prop.GetValueSize() / sizeof(std::int8_t);
                m_prop.zoom_operation.writable = prop.IsSetEnableCurrentValue();
                m_prop.zoom_operation.current = static_cast<std::int8_t>(prop.GetCurrentValue());
                if (nval != m_prop.zoom_operation.possible.size()) {
                    auto parsed_values = parse_zoom_operation(prop.GetValues(), nval);
                    m_prop.zoom_operation.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Zoom_Speed_Range:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.zoom_speed_range.writable = prop.IsSetEnableCurrentValue();
                if (0 < nval) {
                    auto parsed_values = parse_zoom_speed_range(prop.GetValues(), nval);
                    m_prop.zoom_speed_range.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_ZoomAndFocusPosition_Save:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.save_zoom_and_focus_position.writable = prop.IsSetEnableCurrentValue();
                if (0 < nval) {
                    auto parsed_values = parse_save_zoom_and_focus_position(prop.GetValues(), nval);
                    m_prop.save_zoom_and_focus_position.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_ZoomAndFocusPosition_Load:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.load_zoom_and_focus_position.writable = prop.IsSetEnableCurrentValue();
                if (0 < nval) {
                    auto parsed_values = parse_load_zoom_and_focus_position(prop.GetValues(), nval);
                    m_prop.load_zoom_and_focus_position.possible.swap(parsed_values);
                }
                break;
            case SDK::CrDevicePropertyCode::CrDeviceProperty_Remocon_Zoom_Speed_Type:
                nval = prop.GetValueSize() / sizeof(std::uint8_t);
                m_prop.remocon_zoom_speed_type.writable = prop.IsSetEnableCurrentValue();
                m_prop.remocon_zoom_speed_type.current = static_cast<std::uint8_t>(prop.GetCurrentValue());
                if (0 < nval) {
                    auto parsed_values = parse_remocon_zoom_speed_type(prop.GetValues(), nval);
                    m_prop.remocon_zoom_speed_type.possible.swap(parsed_values);
                }
                break;

            default:
                break;
            }
        }
        SDK::ReleaseDeviceProperties(m_device_handle, prop_list);
    }
}

void CameraDevice::get_property(SDK::CrDeviceProperty& prop) const
{
    SDK::CrDeviceProperty* properties = nullptr;
    int nprops = 0;
    // m_cr_lib->GetDeviceProperties(m_device_handle, &properties, &nprops);
    SDK::GetDeviceProperties(m_device_handle, &properties, &nprops);
}

bool CameraDevice::set_property(SDK::CrDeviceProperty& prop) const
{
    // m_cr_lib->SetDeviceProperty(m_device_handle, &prop);
    SDK::SetDeviceProperty(m_device_handle, &prop);
    return false;
}

void CameraDevice::getContentsList()
{
    // check status
    std::int32_t nprop = 0;
    SDK::CrDeviceProperty* prop_list = nullptr;
    CrInt32u getCode = SDK::CrDevicePropertyCode::CrDeviceProperty_ContentsTransferStatus;
    SDK::CrError res = SDK::GetSelectDeviceProperties(m_device_handle, 1, &getCode, &prop_list, &nprop);
    bool bExec = false;
    if (CR_SUCCEEDED(res) && (1 == nprop)) {
        if ((getCode == prop_list[0].GetCode()) && (SDK::CrContentsTransfer_ON == prop_list[0].GetCurrentValue()))
        {
            bExec = true;
        }
        SDK::ReleaseDeviceProperties(m_device_handle, prop_list);
    }
    if (false == bExec) {
        tout << "GetContentsListEnableStatus is Disable. Do it after it becomes Enable.\n";
        return;
    }

    for (CRFolderInfos* pF : m_foldList)
    {
        delete pF;
    }
    m_foldList.clear();
    for (SCRSDK::CrMtpContentsInfo* pC : m_contentList)
    {
        delete pC;
    }
    m_contentList.clear();

    CrInt32u f_nums = 0;
    CrInt32u c_nums = 0;
    SDK::CrMtpFolderInfo* f_list = nullptr;
    SDK::CrError err = SDK::GetDateFolderList(m_device_handle, &f_list, &f_nums);
    if (CR_SUCCEEDED(err) && 0 < f_nums)
    {
        if (f_list)
        {
            tout << "NumOfFolder [" << f_nums << "]" << std::endl;

            for (int i = 0; i < f_nums; ++i)
            {
                auto pFold = new SDK::CrMtpFolderInfo();
                pFold->handle = f_list[i].handle;
                pFold->folderNameSize = f_list[i].folderNameSize;
                CrInt32u lenByOS = sizeof(CrChar) * pFold->folderNameSize;
                pFold->folderName = new CrChar[lenByOS];
                memcpy(pFold->folderName, f_list[i].folderName, lenByOS);
                CRFolderInfos* pCRF = new CRFolderInfos(pFold, 0); // 2nd : fill in later
                m_foldList.push_back(pCRF);
            }
            SDK::ReleaseDateFolderList(m_device_handle, f_list);
        }

        if (0 == m_foldList.size())
        {
            return;
        }

        MtpFolderList::iterator it = m_foldList.begin();
        for (int fcnt = 0; it != m_foldList.end(); ++fcnt, ++it)
        {
            SDK::CrContentHandle* c_list = nullptr;
            err = SDK::GetContentsHandleList(m_device_handle, (*it)->pFolder->handle, &c_list, &c_nums);
            if (CR_SUCCEEDED(err) && 0 < c_nums)
            {
                if (c_list)
                {
                    tout << "(" << (fcnt + 1) << "/" << f_nums << ") NumOfContents [" << c_nums << "]" << std::endl;
                    (*it)->numOfContents = c_nums;
                    for (int i = 0; i < c_nums; i++)
                    {
                        SDK::CrMtpContentsInfo* pConntents = new SDK::CrMtpContentsInfo();
                        err = SDK::GetContentsDetailInfo(m_device_handle, c_list[i], pConntents);
                        if (CR_SUCCEEDED(err))
                        {
                            m_contentList.push_back(pConntents);
                            // progress
                            if (0 == ((i + 1) % 100))
                            {
                                tout << "  ... " << (i + 1) << "/" << c_nums << std::endl;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    SDK::ReleaseContentsHandleList(m_device_handle, c_list);
                }
            }
            if (CR_FAILED(err))
            {
                break;
            }
        }
    }
    else if (CR_SUCCEEDED(err) && 0 == f_nums)
    {
        tout << "No images in memory card." << std::endl;
        return;
    }
    else
    {
        // err
        tout << "Failed SDK::GetContentsList()" << std::endl;
        return;
    }

    if (CR_SUCCEEDED(err))
    {
        MtpFolderList::iterator itF = m_foldList.begin();
        for (std::int32_t f_sep = 0; itF != m_foldList.end(); ++f_sep, ++itF)
        {
            text fname((*itF)->pFolder->folderName);
            printf("===== %#3d : ", (f_sep + 1));
            tout << fname;
            printf(" (0x%08X) , contents[%d] ===== \n", (*itF)->pFolder->handle, (*itF)->numOfContents);

            MtpContentsList::iterator itC = m_contentList.begin();
            for (std::int32_t i = 0; itC != m_contentList.end(); ++i, ++itC)
            {
                if ((*itC)->parentFolderHandle == (*itF)->pFolder->handle)
                {
                    text fname((*itC)->fileName);
                    printf("  %#3d : (0x%08X), ", (i + 1), (*itC)->handle);
                    tout << fname << std::endl;
                }
            }
        }

        while (1)
        {
            if (m_connected == false) {
                break;
            }
            text input;
            tout << std::endl << "Select the number of the contents you want to download :";
            tout << std::endl << "(Returns to the previous menu for invalid numbers)" << std::endl << std::endl;
            tout << std::endl << "input> ";
            std::getline(tin, input);
            text_stringstream ss(input);
            int selected_index = 0;
            ss >> selected_index;
            if (selected_index < 1 || m_contentList.size() < selected_index)
            {
                if (m_connected != false) {
                    tout << "Input cancelled.\n";
                }
                break;
            }
            else
            {
                while (1)
                {
                    if (m_connected == false) {
                        break;
                    }
                    auto targetHandle = m_contentList[selected_index - 1]->handle;
                    printf("Selected (0x%04X) ... \n", targetHandle);
                    text input;
                    tout << std::endl << "Select the number of the content size you want to download :";
                    tout << std::endl << "[-1] Cancel input";
                    tout << std::endl << "[1] Original";
                    tout << std::endl << "[2] Thumbnail";
                    text namefull(m_contentList[selected_index - 1]->fileName);
                    text ext = namefull.substr(namefull.length() - 4, 4);
                    if ((0 == ext.compare(TEXT(".JPG"))) || 
                        (0 == ext.compare(TEXT(".ARW"))) || 
                        (0 == ext.compare(TEXT(".HIF"))))
                    {
                        tout << std::endl << "[3] 2M" << std::endl;
                    }
                    tout << std::endl << "input> ";
                    std::getline(tin, input);
                    text_stringstream ss(input);
                    int selected_contentSize = 0;
                    ss >> selected_contentSize;
                    if (m_connected == false) {
                        break;
                    }
                    if (selected_contentSize < 1 || 3 < selected_contentSize)
                    {
                        if (m_connected != false) {
                            tout << "Input cancelled.\n";
                        }
                        break;
                    }
                    switch (selected_contentSize)
                    {
                    case 1:
                        // [async] get contents
                        pullContents(targetHandle);
                        break;
                    case 2:
                        // [sync] get thumbnail jpeg
                        getThumbnail(targetHandle);
                        break;
                    case 3:
                        // [async] [only still] get screennail jpeg
                        getScreennail(targetHandle);
                        break;
                    default:
                        break;
                    }
                    std::this_thread::sleep_for(2s);
                }
            }
        }
    }
}

void CameraDevice::pullContents(SDK::CrContentHandle content)
{
    SDK::CrError err = SDK::PullContentsFile(m_device_handle, content);

    if (SDK::CrError_None != err)
    {
        //printf("[Error] err=0x%04X, handle(0x%08X)\n", err, content);
        text id(this->get_id());
        text msg = get_message_desc(err);
        if (!msg.empty()) {
            // output is 2 line
            tout << std::endl << msg.data() << ", handle=" << std::hex << content << std::dec << std::endl;
            tout << m_info->GetModel() << " (" << id.data() << ")" << std::endl;
        }
    }
}

void CameraDevice::getScreennail(SDK::CrContentHandle content)
{
    SDK::CrError err = SDK::PullContentsFile(m_device_handle, content, SDK::CrPropertyStillImageTransSize_SmallSizeJPEG);

    if (SDK::CrError_None != err)
    {
        //printf("[Error] err=0x%04X, handle(0x%08X)\n", err, content);
        text id(this->get_id());
        text msg = get_message_desc(err);
        if (!msg.empty()) {
            // output is 2 line
            tout << std::endl << msg.data() << ", handle=" << std::hex << content << std::dec << std::endl;
            tout << m_info->GetModel() << " (" << id.data() << ")" << std::endl;
        }
    }
}

void CameraDevice::getThumbnail(SDK::CrContentHandle content)
{
    CrInt32u bufSize = 0x28000; // @@@@ temp

    auto* image_data = new SDK::CrImageDataBlock();
    if (!image_data)
    {
        tout << "getThumbnail FAILED (new CrImageDataBlock class)\n";
        return;
    }
    CrInt8u* image_buff = new CrInt8u[bufSize];
    if (!image_buff)
    {
        delete image_data;
        tout << "getThumbnail FAILED (new Image buffer)\n";
        return;
    }
    image_data->SetSize(bufSize);
    image_data->SetData(image_buff);

    SDK::CrError err = SDK::GetContentsThumbnailImage(m_device_handle, content, image_data);
    if (CR_FAILED(err))
    {
        //printf("[Error] err=0x%04X, handle(0x%08X)\n", err, content);
        text id(this->get_id());
        text msg = get_message_desc(err);
        if (!msg.empty()) {
            // output is 2 line
            tout << std::endl << msg.data() << ", handle=" << std::hex << content << std::dec << std::endl;
            tout << m_info->GetModel() << " (" << id.data() << ")" << std::endl;
        }
    }
    else
    {
        if (0 < image_data->GetSize())
        {
#if defined(__APPLE__)
            char path[255]; /*MAX_PATH*/
            getcwd(path, sizeof(path) - 1);
            char filename[] = "/Thumbnail.JPG";
            strcat(path, filename);
#else
            auto path = fs::current_path();
            path.append(TEXT("Thumbnail.JPG"));
#endif
            tout << path << '\n';

            std::ofstream file(path, std::ios::out | std::ios::binary);
            if (!file.bad())
            {
                std::uint32_t len = image_data->GetImageSize();
                file.write((char*)image_data->GetImageData(), len);
                file.close();
            }
        }
    }
    delete[] image_buff; // Release
    delete image_data; // Release
}

} // namespace cli
