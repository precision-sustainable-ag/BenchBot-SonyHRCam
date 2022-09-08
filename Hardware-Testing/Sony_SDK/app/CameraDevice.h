#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <atomic>
#include <cstdint>
#include "CRSDK/CameraRemote_SDK.h"
#include "CRSDK/IDeviceCallback.h"
#include "ConnectionInfo.h"
#include "PropertyValueTable.h"
#include "Text.h"
#include "MessageDefine.h"

namespace cli
{

class CRFolderInfos
{
public:
    CRFolderInfos(SCRSDK::CrMtpFolderInfo* info, int32_t nums)
        : pFolder(info)
        , numOfContents(nums)
    {};
    ~CRFolderInfos()
    {
        delete[] pFolder->folderName;
        pFolder->folderName = NULL;
        delete pFolder;
        pFolder = NULL;
    };
public:
    SCRSDK::CrMtpFolderInfo* pFolder;
    int32_t numOfContents;
};

typedef std::vector<CRFolderInfos*> MtpFolderList;
typedef std::vector<SCRSDK::CrMtpContentsInfo*> MtpContentsList;

// Forward declarations
struct CRLibInterface;

class CameraDevice : public SCRSDK::IDeviceCallback
{
public:
    CameraDevice() = delete;
    CameraDevice(std::int32_t no, CRLibInterface const* cr_lib, SCRSDK::ICrCameraObjectInfo const* camera_info);
    ~CameraDevice();

    // Try to connect to the device
    bool connect(SCRSDK::CrSdkControlMode openMode);

    // Disconnect from the device
    bool disconnect();

    // Release from the device
    bool release();

    /*** Shooting operations ***/

    void capture_image() const;
    void s1_shooting() const;
    void af_shutter() const;
    void continuous_shooting() const;

    /*** Property operations ***/
    // Should be const functions, but requires load property, which is not

    void get_aperture();
    void get_iso();
    void get_shutter_speed();
    void get_position_key_setting();
    void get_exposure_program_mode();
    void get_still_capture_mode();
    void get_focus_mode();
    void get_focus_area();
    void get_live_view();
    void get_live_view_image_quality();
    void get_live_view_status();
    void get_af_area_position();
    void get_select_media_format();
    void get_white_balance();
    bool get_custom_wb();
    void get_zoom_operation();
    void get_remocon_zoom_speed_type();

    void set_aperture();
    void set_iso();
    bool set_save_info() const;
    void set_shutter_speed();
    void set_position_key_setting();
    void set_exposure_program_mode();
    void set_still_capture_mode();
    void set_focus_mode();
    void set_focus_area();
    void set_live_view_image_quality();
    void set_live_view_status();
    void set_af_area_position();
    void set_white_balance();
    void set_custom_wb();
    void set_zoom_operation();
    void set_remocon_zoom_speed_type();

    void execute_lock_property(CrInt16u code);
    void set_select_media_format();
    void execute_movie_rec();
    void execute_downup_property(CrInt16u code);
    void execute_pos_xy(CrInt16u code);
    void change_live_view_enable();
    bool is_live_view_enable() { return m_lvEnbSet; };
    void execute_preset_focus();

    std::int32_t get_number() { return m_number; }
    text get_model() { return text(m_info->GetModel()); }
    text get_id();

    // Check if this device is connected
    bool is_connected() const;
    std::uint32_t ip_address() const;
    text ip_address_fmt() const;
    text mac_address() const;
    std::int16_t pid() const;

    void getContentsList();
    void pullContents(SCRSDK::CrContentHandle content);
    void getScreennail(SCRSDK::CrContentHandle content);
    void getThumbnail(SCRSDK::CrContentHandle content);

    SCRSDK::CrSdkControlMode get_sdkmode();


public:
    // Inherited via IDeviceCallback
    virtual void OnConnected(SCRSDK::DeviceConnectionVersioin version) override;
    virtual void OnDisconnected(CrInt32u error) override;
    virtual void OnPropertyChanged() override;
    virtual void OnLvPropertyChanged() override;
    virtual void OnCompleteDownload(CrChar* filename) override;
    virtual void OnWarning(CrInt32u warning) override;
    virtual void OnError(CrInt32u error) override;
    virtual void OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override;
    virtual void OnLvPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override;
    virtual void OnNotifyContentsTransfer(CrInt32u notify, SCRSDK::CrContentHandle contentHandle, CrChar* filename) override;

private:
    void load_properties(CrInt32u num = 0, CrInt32u* codes = nullptr);
    void get_property(SCRSDK::CrDeviceProperty& prop) const;
    bool set_property(SCRSDK::CrDeviceProperty& prop) const;

private:
    CRLibInterface const* m_cr_lib;
    std::int32_t m_number;
    SCRSDK::ICrCameraObjectInfo* m_info;
    std::int64_t m_device_handle;
    std::atomic<bool> m_connected;
    ConnectionType m_conn_type;
    NetworkInfo m_net_info;
    UsbInfo m_usb_info;
    PropertyValueTable m_prop;
    bool m_lvEnbSet;
    SCRSDK::CrSdkControlMode m_modeSDK;
    MtpFolderList   m_foldList;
    MtpContentsList m_contentList;
    bool m_spontaneous_disconnection;
};
} // namespace cli


#endif // !CAMERADEVICE_H
