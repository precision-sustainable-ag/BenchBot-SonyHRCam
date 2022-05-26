#include <cstdlib>
#if defined(USE_EXPERIMENTAL_FS)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#if defined(__APPLE__)
#include <unistd.h>
#endif
#endif
#include <cstdint>
#include <iomanip>
#include "CRSDK/CameraRemote_SDK.h"
#include "CameraDevice.h"
#include "Text.h"

//#define LIVEVIEW_ENB

namespace SDK = SCRSDK;

// Global dll object
// cli::CRLibInterface* cr_lib = nullptr;

int main()
{
    // Change global locale to native locale
    std::locale::global(std::locale(""));

    // Make the stream's locale the same as the current global locale
    cli::tin.imbue(std::locale());
    cli::tout.imbue(std::locale());

    cli::tout << "RemoteSampleApp v1.05.00 running...\n\n";

    CrInt32u version = SDK::GetSDKVersion();
    int major = (version & 0xFF000000) >> 24;
    int minor = (version & 0x00FF0000) >> 16;
    int patch = (version & 0x0000FF00) >> 8;
    // int reserved = (version & 0x000000FF);

    cli::tout << "Remote SDK version: ";
    cli::tout << major << "." << minor << "." << std::setfill(TEXT('0')) << std::setw(2) << patch << "\n";

    // Load the library dynamically
    // cr_lib = cli::load_cr_lib();

    cli::tout << "Initialize Remote SDK...\n";
    
#if defined(__APPLE__)
        char path[255]; /*MAX_PATH*/
        getcwd(path, sizeof(path) -1);
        cli::tout << "Working directory: " << path << '\n';
#else
        cli::tout << "Working directory: " << fs::current_path() << '\n';
#endif
    // auto init_success = cr_lib->Init(0);
    auto init_success = SDK::Init();
    if (!init_success) {
        cli::tout << "Failed to initialize Remote SDK. Terminating.\n";
        // cr_lib->Release();
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    cli::tout << "Remote SDK successfully initialized.\n\n";

    cli::tout << "Enumerate connected camera devices...\n";
    SDK::ICrEnumCameraObjectInfo* camera_list = nullptr;
    // auto enum_status = cr_lib->EnumCameraObjects(&camera_list, 3);
    auto enum_status = SDK::EnumCameraObjects(&camera_list);
    if (CR_FAILED(enum_status) || camera_list == nullptr) {
        cli::tout << "No cameras detected. Connect a camera and retry.\n";
        // cr_lib->Release();
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }
    auto ncams = camera_list->GetCount();
    cli::tout << "Camera enumeration successful. " << ncams << " detected.\n\n";

    for (CrInt32u i = 0; i < ncams; ++i) {
        auto camera_info = camera_list->GetCameraObjectInfo(i);
        cli::text conn_type(camera_info->GetConnectionTypeName());
        cli::text model(camera_info->GetModel());
        cli::text id = TEXT("");
        if (TEXT("IP") == conn_type) {
            cli::NetworkInfo ni = cli::parse_ip_info(camera_info->GetId(), camera_info->GetIdSize());
            id = ni.mac_address;
        }
        else id = ((TCHAR*)camera_info->GetId());
        cli::tout << '[' << i + 1 << "] " << model.data() << " (" << id.data() << ")\n";
    }

    cli::tout << std::endl << "Connect to camera with input number...\n";
    cli::tout << "input> ";
    cli::text connectNo;
    std::getline(cli::tin, connectNo);
    cli::tout << '\n';

    cli::tsmatch smatch;
    CrInt32u no = 0;
    while (true) {
        no = 0;
        if (std::regex_search(connectNo, smatch, cli::tregex(TEXT("[0-9]")))) {
            no = stoi(connectNo);
            if (no == 0)
                break; // finish

            if (camera_list->GetCount() < no) {
                cli::tout << "input value over \n";
                cli::tout << "input> "; // Redo
                std::getline(cli::tin, connectNo);
                continue;
            }
            else
                break; // ok
        }
        else
            break; // not number
    }

    if (no == 0) {
        cli::tout << "Invalid Number. Finish App.\n";
        SDK::Release();
        std::exit(EXIT_FAILURE);
    }

    typedef std::shared_ptr<cli::CameraDevice> CameraDevicePtr;
    typedef std::vector<CameraDevicePtr> CameraDeviceList;
    CameraDeviceList cameraList; // all
    std::int32_t cameraNumUniq = 1;
    std::int32_t selectCamera = 1;

    cli::tout << "Connect to selected camera...\n";
    auto* camera_info = camera_list->GetCameraObjectInfo(no - 1);

    cli::tout << "Create camera SDK camera callback object.\n";
    CameraDevicePtr camera = CameraDevicePtr(new cli::CameraDevice(cameraNumUniq, nullptr, camera_info));
    cameraList.push_back(camera); // add 1st

    cli::tout << "Release enumerated camera list.\n";
    camera_list->Release();

    // Overview
    //   loop-A : main loop
    //   loop-B : TOP-MENU ____ Switching between 'Remote Control Mode' connection and 'Contents Transfer Mode (MTP)' connection.
    //   loop-C : REMOTE-MENU
    //   loop-D : MTP-MENU

    // loop-A
    for (;;) {

        // 1,2   = break of TOP-MENU, continue to REMOTE-MENU or MTP-MENU
        // x     = quit the app
        // other = ignore
        bool bQuit = false;
        // loop-B
        while (true)
        {
            cli::tout << "<< TOP-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
            cli::tout
                << "(1) Connect (Remote Control Mode)\n"
                << "(2) Connect (Contents Transfer Mode)\n"
                // << "(f) Release Device \n"
                << "(x) Exit\n";

            cli::tout << "input> ";
            cli::text action;
            std::getline(cli::tin, action);
            cli::tout << '\n';

            if (action == TEXT("x")) { /* Exit app */
                bQuit = true;
                CameraDeviceList::const_iterator it = cameraList.begin();
                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it) {
                    if ((*it)->is_connected()) {
                        cli::tout << "Initiate disconnect sequence.\n";
                        auto disconnect_status = (*it)->disconnect();
                        if (!disconnect_status) {
                            // try again
                            disconnect_status = (*it)->disconnect();
                        }
                        if (!disconnect_status)
                            cli::tout << "Disconnect failed to initiate.\n";
                        else
                            cli::tout << "Disconnect successfully initiated!\n\n";
                    }
                    (*it)->release();
                }
                break; // exit loop-B
            }
            //else if (action == TEXT("f")) { /* ReleaseDevice */
            //    if (2 > cameraList.size()) {
            //        cli::tout << std::endl << "Does not execute if there is no camera to switch control after the release device of the specified camera." << std::endl;
            //    }
            //    else {
            //        CameraDeviceList::const_iterator it = cameraList.begin();
            //        for (; it != cameraList.end(); ++it) {
            //            if ((*it)->get_number() == camera->get_number()) {
            //                (*it)->release();
            //                cameraList.erase(it);
            //                break;
            //            }
            //        }
            //        it = cameraList.begin(); // switch to top of list
            //        camera = (*it);
            //        selectCamera = camera->get_number();
            //    }
            //}
            else if (action == TEXT("1")) { /* connect Remote */
                if (camera->is_connected()) {
                    cli::tout << "Please disconnect\n";
                }
                else {
                    camera->connect(SDK::CrSdkControlMode_Remote);
                }
                break; // exit loop-B
            }
            else if (action == TEXT("2")) { /* connect MTP */
                if (camera->is_connected()) {
                    cli::tout << "Please disconnect\n";
                }
                else {
                    camera->connect(SDK::CrSdkControlMode_ContentsTransfer);
                }
                break; // exit loop-B
            }
            cli::tout << std::endl;

        } // end of loop-B

        // check quit
        if (bQuit) break; // exit loop-A

        // ------------------------
        // Remote
        // ------------------------
        if (SDK::CrSdkControlMode_Remote == camera->get_sdkmode())
        {
            // loop-C
            while (true)
            {
                cli::tout << "<< REMOTE-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
                cli::tout
                    << "(s) Status display and camera switching \n"
                    << "(0) Disconnect and return to the top menu\n"
                    << "(1) Shutter Release \n"
                    << "(2) Shutter Half Release in AF mode \n"
                    << "(3) Shutter Half and Full Release in AF mode \n"
                    << "(4) Continuous Shooting \n"
                    << "(5) Aperture \n"
                    << "(6) ISO \n"
                    << "(7) Shutter Speed \n"
                    << "(8) Live View \n"
                    << "(9) Live View Image Quality \n"
                    << "(a) Position Key Setting \n"
                    << "(b) Exposure Program Mode \n"
                    << "(c) Still Capture Mode(Drive mode) \n"
                    << "(d) Focus Mode \n"
                    << "(e) Focus Area \n"
#if defined(LIVEVIEW_ENB)
                    << "(lv) LiveView Enable \n"
#endif
                    << "(11) FELock \n"
                    << "(12) AWBLock \n"
                    << "(13) AF Area Position(x,y) \n"
                    << "(14) Selected MediaFormat \n"
                    << "(15) Movie Rec Button \n"
                    << "(16) White Balance \n"
                    << "(17) Custom WB \n"
                    << "(18) Zoom Operation \n"
                    << "(19) Zoom Speed Type \n"
                    << "(20) Preset Focus \n";

                cli::tout << "input> ";
                cli::text action;
                std::getline(cli::tin, action);
                cli::tout << '\n';

                if (action == TEXT("s")) { /* status display and device selection */
                    cli::tout << "Status display and camera switching.\n";
#if defined(LIVEVIEW_ENB)
                    cli::tout << "number - connected - lvEnb - model - id\n";
#else
                    cli::tout << "number - connected - model - id\n";
#endif
                    CameraDeviceList::const_iterator it = cameraList.begin();
                    for (std::int32_t i = 0; it != cameraList.end(); ++i, ++it)
                    {
                        cli::text model = (*it)->get_model();
                        if (model.size() < 10) {
                            int32_t apendCnt = 10 - model.size();
                            model.append(apendCnt, TEXT(' '));
                        }
                        cli::text id = (*it)->get_id();
                        std::uint32_t num = (*it)->get_number();
                        if (selectCamera == num) { cli::tout << "* "; }
                        else { cli::tout << "  "; }
                        cli::tout << std::setfill(TEXT(' ')) << std::setw(4) << std::left << num
                            << " - " << ((*it)->is_connected() ? "true " : "false")
#if defined(LIVEVIEW_ENB)
                            << " - " << ((*it)->is_live_view_enable() ? "true " : "false")
#endif
                            << " - " << model.data()
                            << " - " << id.data() << std::endl;
                    }

                    cli::tout << std::endl << "Selected camera number = [" << selectCamera << "]" << std::endl << std::endl;

                    cli::tout << "Choose a number :\n";
                    cli::tout << "[-1] Cancel input\n";
                    cli::tout << "[0]  Create new CameraDevice\n";
                    cli::tout << "[1]  Switch cameras for controls\n";
                    cli::tout << std::endl << "input> ";

                    cli::text input;
                    std::getline(cli::tin, input);
                    cli::text_stringstream ss(input);
                    int selected_index = 0;
                    ss >> selected_index;

                    if (selected_index < 0 || 1 < selected_index) {
                        cli::tout << "Input cancelled.\n";
                    }

                    // new camera connect
                    if (0 == selected_index) 
                    {

                        enum_status = SDK::EnumCameraObjects(&camera_list);
                        if (CR_FAILED(enum_status) || camera_list == nullptr) {
                            cli::tout << "No cameras detected. Connect a camera and retry.\n";
                        }
                        else
                        {
                            cli::tout << "[-1] Cancel input\n";
                            ncams = camera_list->GetCount();
                            for (CrInt32u i = 0; i < ncams; ++i) {
                                auto camera_info = camera_list->GetCameraObjectInfo(i);
                                cli::text conn_type(camera_info->GetConnectionTypeName());
                                cli::text model(camera_info->GetModel());
                                cli::text id = TEXT("");
                                if (TEXT("IP") == conn_type) {
                                    cli::NetworkInfo ni = cli::parse_ip_info(camera_info->GetId(), camera_info->GetIdSize());
                                    id = ni.mac_address;
                                }
                                else id = ((TCHAR*)camera_info->GetId());
                                cli::tout << '[' << i + 1 << "] " << model.data() << " (" << id.data() << ") ";
                                CameraDeviceList::const_iterator it = cameraList.begin();
                                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it){
                                    cli::text alreadyId = (*it)->get_id();
                                    if (0 == id.compare(alreadyId)) {
                                        cli::tout << "*";
                                        break;
                                    }
                                }
                                cli::tout << std::endl;
                            }

                            cli::tout << std::endl << "idx input> ";
                            std::getline(cli::tin, input);
                            cli::text_stringstream ss2(input);
                            int selected_no = 0;
                            ss2 >> selected_no;

                            if (selected_no < 1 || (std::int32_t)ncams < selected_no) {
                                cli::tout << "Input cancelled.\n";
                            }
                            else {
                                camera_info = camera_list->GetCameraObjectInfo(selected_no - 1);
                                cli::text conn_type(camera_info->GetConnectionTypeName());
                                cli::text model_select(camera_info->GetModel());
                                cli::text id_select = TEXT("");
                                if (TEXT("IP") == conn_type) {
                                    cli::NetworkInfo ni = cli::parse_ip_info(camera_info->GetId(), camera_info->GetIdSize());
                                    id_select = ni.mac_address;
                                }
                                else id_select = ((TCHAR*)camera_info->GetId());
                                bool findAlready = false;
                                CameraDeviceList::const_iterator it = cameraList.begin();
                                for (std::int32_t j = 0; it != cameraList.end(); ++j, ++it) {
                                    if ((0 == (*it)->get_model().compare(model_select)) &&
                                        (0 == (*it)->get_id().compare(id_select))) {
                                        findAlready = true;
                                        cli::tout << "Already connected!\n";
                                        break;
                                    }
                                }
                                if (false == findAlready) {
                                    std::int32_t newNum = cameraNumUniq + 1;
                                    CameraDevicePtr newCam = CameraDevicePtr(new cli::CameraDevice(newNum, nullptr, camera_info));
                                    cameraNumUniq = newNum;
                                    cameraList.push_back(newCam); // add
                                    camera = newCam; // switch target
                                    selectCamera = cameraNumUniq; // latest
                                    break; // exit loop-C
                                }
                            }
                        }
                        camera_list->Release();
                    }
                    // switch device
                    else if (1 == selected_index) {
                        cli::tout << std::endl << "number input> ";
                        std::getline(cli::tin, input);
                        cli::text_stringstream ss3(input);
                        int input_no = 0;
                        ss3 >> input_no;

                        if (input_no < 1) {
                            cli::tout << "Input cancelled.\n";
                        }
                        else {
                            bool findTarget = false;
                            CameraDeviceList::const_iterator it = cameraList.begin();
                            for (; it != cameraList.end(); ++it) {
                                if ((*it)->get_number() == input_no) {
                                    findTarget = true;
                                    camera = (*it);
                                    selectCamera = input_no;
                                    break;
                                }
                            }
                            if (!findTarget) {
                                cli::tout << "The specified camera cannot be found!\n";
                            }
                        }
                    }
                } // end menu-s

                else if (action == TEXT("0")) { /* Return top menu */
                    if (camera->is_connected()) {
                        camera->disconnect();
                    }
                    break; // exit loop-B
                }
                else if (action == TEXT("1")) { /* Take photo */
                    camera->capture_image();
                }
                else if (action == TEXT("2")) { /* S1 Shooting */
                    camera->s1_shooting();
                }
                else if (action == TEXT("3")) { /* AF Shutter */
                    camera->af_shutter();
                }
                else if (action == TEXT("4")) { /* Continuous Shooting */
                    camera->continuous_shooting();
                }
                else if (action == TEXT("5")) { /* Aperture. */
                    camera->get_aperture();
                    camera->set_aperture();
                }
                else if (action == TEXT("6")) { /* ISO */
                    camera->get_iso();
                    camera->set_iso();
                }
                else if (action == TEXT("7")) { /* Shutter Speed */
                    camera->get_shutter_speed();
                    camera->set_shutter_speed();
                }
                else if (action == TEXT("8")) { /* Live View */
                    camera->get_live_view();
                }
                else if (action == TEXT("9")) { /* Live View Image Quality */
                    camera->get_live_view_image_quality();
                    camera->set_live_view_image_quality();
                }
                else if (action == TEXT("10")) { /* Live View Image Status */
                    camera->get_live_view_status();
                    camera->set_live_view_status();
                }
                else if (action == TEXT("a")) { /* Position Key Setting */
                    camera->get_position_key_setting();
                    camera->set_position_key_setting();
                }
                else if (action == TEXT("b")) { /* Exposure Program Mode */
                    camera->get_exposure_program_mode();
                    camera->set_exposure_program_mode();
                }
                else if (action == TEXT("c")) { /* Still Capture Mode(Drive mode) */
                    camera->get_still_capture_mode();
                    camera->set_still_capture_mode();
                }
                else if (action == TEXT("d")) { /* Focus Mode */
                    camera->get_focus_mode();
                    camera->set_focus_mode();
                }
                else if (action == TEXT("e")) { /* Focus Area */
                    camera->get_focus_area();
                    camera->set_focus_area();
                }
                else if (action == TEXT("11")) { /* FELock */
                    cli::tout << "Flash device required.";
                    camera->execute_lock_property((CrInt16u)SDK::CrDevicePropertyCode::CrDeviceProperty_FEL);
                }
                else if (action == TEXT("12")) { /* AWBLock */
                    camera->execute_lock_property((CrInt16u)SDK::CrDevicePropertyCode::CrDeviceProperty_AWBL);
                }
                else if (action == TEXT("13")) { /* AF Area Position(x,y) */
                    camera->set_af_area_position();
                }
                else if (action == TEXT("14")) { /* Selected MediaFormat */
                    camera->get_select_media_format();
                    camera->set_select_media_format();
                }
                else if (action == TEXT("15")) { /* Movie Rec Button */
                    camera->execute_movie_rec();
                }
                else if (action == TEXT("16")) { /* White Balance */
                    camera->get_white_balance();
                    camera->set_white_balance();
                }
                else if (action == TEXT("17")) { /* Custom WB */
                    camera->get_custom_wb();
                    camera->set_custom_wb();
                }
                else if (action == TEXT("18")) { /* Zoom Operation */
                    camera->get_zoom_operation();
                    camera->set_zoom_operation();
                }
                else if (action == TEXT("19")) { /* Remocon Zoom Speed Type */
                    camera->get_remocon_zoom_speed_type();
                    camera->set_remocon_zoom_speed_type();
                }
                else if (action == TEXT("20")) { /* Preset Focus */
                    camera->execute_preset_focus();
                }
#if defined(LIVEVIEW_ENB)
                else if (action == TEXT("lv")) { /* LiveView Enable */
                    camera->change_live_view_enable();
                }
#endif
                cli::tout << std::endl;
            } // end of loop-C
            cli::tout << std::endl;
        }
        // ------------------------
        // Contents Transfer
        // ------------------------
        else
        {
            // loop-D
            while (true)
            {
                cli::tout << "<< MTP-MENU >>\nWhat would you like to do? Enter the corresponding number.\n";
                cli::tout
                    << "(0) Disconnect and return to the top menu\n"
                    << "(1) Get contents list \n";
                cli::tout << "input> ";
                cli::text action;
                std::getline(cli::tin, action);
                cli::tout << '\n';

                if (action == TEXT("0")) { /* Return top menu */
                    if (camera->is_connected()) {
                        camera->disconnect();
                    }
                    break; // exit loop-D
                }
                else if (action == TEXT("1")) { /* GetContentsList() */
                    if (camera->is_connected()) {
                        camera->getContentsList();
                    }
                    else
                    {
                        cli::tout << "Disconnected\n";
                        break;
                    }
                }
                if (!camera->is_connected()) {
                    break;
                }
                cli::tout << std::endl;
            } // end of loop-D
            cli::tout << std::endl;
        }

    }// end of loop-A

    cli::tout << "Release SDK resources.\n";
    // cr_lib->Release();
    SDK::Release();

    // cli::free_cr_lib(&cr_lib);

    cli::tout << "Exiting application.\n";
    std::exit(EXIT_SUCCESS);
}
