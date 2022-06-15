#include "PropertyValueTable.h"
#include <cmath>
#include <cstring>


namespace SDK = SCRSDK;

namespace impl
{
template <typename T>
std::vector<T> parse(unsigned char const* buf, std::uint32_t nval)
{
    constexpr std::size_t const type_size = sizeof(T);
    std::vector<T> result(nval);
    std::uint32_t i = 0; T* source = static_cast<T*>(buf);
    for (; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
}

inline double Round(double value, int figure)
{
    bool isNagative = ( value < 0 );
    if (isNagative == true) value = -value;

    double rate = std::pow(10.0, figure);
    //double value1 = value * rate + 0.5;

    long tmp = (long)(value * rate + 0.5);
    value = tmp/rate;

    if (isNagative == true) value = -value;

    return value;
}

// Retrieve the hi-order word (16-bit value) of a dword (32-bit value)
constexpr std::uint16_t HIWORD(std::uint32_t dword)
{
    return static_cast<std::uint16_t>((dword >> 16) & 0xFFFF);
}

// Retrieve the low-order word (16-bit value) of a dword (32-bit value)
constexpr std::uint16_t LOWORD(std::uint32_t dword)
{
    return static_cast<std::uint16_t>(dword & 0xFFFF);
}
} // namespace impl

namespace cli
{
std::vector<std::uint16_t> parse_f_number(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint32_t> parse_iso_sensitivity(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint32_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint32_t> parse_shutter_speed(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint32_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_position_key_setting(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint32_t> parse_exposure_program_mode(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint32_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint32_t> parse_still_capture_mode(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint32_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_focus_mode(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_focus_area(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_live_view_image_quality(unsigned char const* buf, std::uint32_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_media_slotx_format_enable_status(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_white_balance(unsigned char const* buf, std::uint16_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_customwb_capture_stanby(unsigned char const* buf, std::uint16_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_customwb_capture_stanby_cancel(unsigned char const* buf, std::uint16_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_customwb_capture_operation(unsigned char const* buf, std::uint16_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint16_t> parse_customwb_capture_execution_state(unsigned char const* buf, std::uint16_t nval)
{
    using TargetType = std::uint16_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_zoom_operation_status(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_zoom_setting_type(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_zoom_types_status(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::int8_t> parse_zoom_operation(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::int8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::int8_t> parse_zoom_speed_range(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::int8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_save_zoom_and_focus_position(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_load_zoom_and_focus_position(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

std::vector<std::uint8_t> parse_remocon_zoom_speed_type(unsigned char const* buf, std::uint8_t nval)
{
    using TargetType = std::uint8_t;
    constexpr std::size_t const type_size = sizeof(TargetType);
    TargetType const* source = reinterpret_cast<TargetType const*>(buf);
    std::vector<TargetType> result(nval);
    for (std::uint32_t i = 0; i < nval; ++i, ++source) {
        std::memcpy(&result[i], source, type_size);
    }
    return result;
}

text format_f_number(std::uint16_t f_number)
{
    text_stringstream ts;
    if ((0x0000 == f_number) || (SDK::CrFnumber_Unknown == f_number)) {
        ts << TEXT("--");
    }
    else if(SDK::CrFnumber_Nothing == f_number) {
        // Do nothing
    }
    else {
        auto modValue = static_cast<std::uint16_t>(f_number % 100);
        if (modValue > 0) {
            ts << TEXT('F') << impl::Round((f_number / 100.0), 1);
        }
        else {
            ts << TEXT('F') << f_number / 100;
        }
    }

    return ts.str();
}

text format_iso_sensitivity(std::uint32_t iso)
{
    text_stringstream ts;

    std::uint32_t iso_ext = (iso >> 24) & 0x000000F0;  // bit 28-31
    std::uint32_t iso_mode = (iso >> 24) & 0x0000000F; // bit 24-27
    std::uint32_t iso_value = (iso & 0x00FFFFFF);      // bit  0-23

    if (iso_mode == SDK::CrISO_MultiFrameNR) {
        ts << TEXT("Multi Frame NR ");
    }
    else if (iso_mode == SDK::CrISO_MultiFrameNR_High) {
        ts << TEXT("Multi Frame NR High ");
    }

    if (iso_value == SDK::CrISO_AUTO) {
        ts << TEXT("ISO AUTO");
    }
    else {
        ts << TEXT("ISO ") << iso_value;
    }

    //if (iso_ext == SDK::CrISO_Ext) {
    //    ts << TEXT(" (EXT)");
    //}

    return ts.str();
}

text format_shutter_speed(std::uint32_t shutter_speed)
{
    text_stringstream ts;

    CrInt16u numerator = impl::HIWORD(shutter_speed);
    CrInt16u denominator = impl::LOWORD(shutter_speed);

    if (0 == shutter_speed) {
        ts << TEXT("Bulb");
    }
    else if (1 == numerator) {
        ts << numerator << '/' << denominator;
    }
    else if (0 == numerator % denominator) {
        ts << (numerator / denominator) << '"';
    }
    else {
        CrInt32 numdevision = numerator / denominator;
        CrInt32 numremainder = numerator % denominator;
        ts << numdevision << '.' << numremainder << '"';
    }
    return ts.str();
}

text format_position_key_setting(std::uint16_t position_key_setting)
{
    text_stringstream ts;

    switch (position_key_setting) {
    case SDK::CrPriorityKeySettings::CrPriorityKey_CameraPosition:
        ts << "Camera Position";
        break;
    case SDK::CrPriorityKeySettings::CrPriorityKey_PCRemote:
        ts << "PC Remote Setting";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_exposure_program_mode(std::uint32_t exposure_program_mode)
{
    text_stringstream ts;

    switch (exposure_program_mode) {
    case SDK::CrExposureProgram::CrExposure_M_Manual:
        ts << "M_Manual";
        break;
    case SDK::CrExposureProgram::CrExposure_P_Auto:
        ts << "P_Auto";
        break;
    case SDK::CrExposureProgram::CrExposure_A_AperturePriority:
        ts << "A_AperturePriority";
        break;
    case SDK::CrExposureProgram::CrExposure_S_ShutterSpeedPriority:
        ts << "S_ShutterSpeedPriority";
        break;
     case SDK::CrExposureProgram::CrExposure_Program_Creative:
         ts << "ProgramCreative";
         break;
     case SDK::CrExposureProgram::CrExposure_Program_Action:
         ts << "ProgramAction";
         break;
    case SDK::CrExposureProgram::CrExposure_Portrait:
        ts << "Portrait";
        break;
    case SDK::CrExposureProgram::CrExposure_Auto:
        ts << "Auto";
        break;
    case SDK::CrExposureProgram::CrExposure_Auto_Plus:
        ts << "Auto_Plus";
        break;
    case SDK::CrExposureProgram::CrExposure_P_A:
        ts << "P_A";
        break;
    case SDK::CrExposureProgram::CrExposure_P_S:
        ts << "P_S";
        break;
    case SDK::CrExposureProgram::CrExposure_Sports_Action:
        ts << "Sports_Action";
        break;
    case SDK::CrExposureProgram::CrExposure_Sunset:
        ts << "Sunset";
        break;
    case SDK::CrExposureProgram::CrExposure_Night:
        ts << "Night";
        break;
    case SDK::CrExposureProgram::CrExposure_Landscape:
        ts << "Landscape";
        break;
    case SDK::CrExposureProgram::CrExposure_Macro:
        ts << "Macro";
        break;
    case SDK::CrExposureProgram::CrExposure_HandheldTwilight:
        ts << "HandheldTwilight";
        break;
    case SDK::CrExposureProgram::CrExposure_NightPortrait:
        ts << "NightPortrait";
        break;
    case SDK::CrExposureProgram::CrExposure_AntiMotionBlur:
        ts << "AntiMotionBlur";
        break;
    case SDK::CrExposureProgram::CrExposure_Pet:
        ts << "Pet";
        break;
    case SDK::CrExposureProgram::CrExposure_Gourmet:
        ts << "Gourmet";
        break;
    case SDK::CrExposureProgram::CrExposure_Fireworks:
        ts << "Fireworks";
        break;
    case SDK::CrExposureProgram::CrExposure_HighSensitivity:
        ts << "HighSensitivity";
        break;
    case SDK::CrExposureProgram::CrExposure_MemoryRecall:
        ts << "MemoryRecall";
        break;
    case SDK::CrExposureProgram::CrExposure_ContinuousPriority_AE_8pics:
        ts << "ContinuousPriority_AE_8pics";
        break;
    case SDK::CrExposureProgram::CrExposure_ContinuousPriority_AE_10pics:
        ts << "ContinuousPriority_AE_10pics";
        break;
    case SDK::CrExposureProgram::CrExposure_ContinuousPriority_AE_12pics:
        ts << "ContinuousPriority_AE_12pics";
        break;
    case SDK::CrExposureProgram::CrExposure_3D_SweepPanorama:
        ts << "3D_SweepPanorama";
        break;
    case SDK::CrExposureProgram::CrExposure_SweepPanorama:
        ts << "SweepPanorama";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_P:
        ts << "Movie_P";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_A:
        ts << "Movie_A";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_S:
        ts << "Movie_S";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_M:
        ts << "Movie_M";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_Auto:
        ts << "Movie_Auto";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_SQMotion_P:
        ts << "Movie_SQMotion_P";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_SQMotion_A:
        ts << "Movie_SQMotion_A";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_SQMotion_S:
        ts << "Movie_SQMotion_S";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_SQMotion_M:
        ts << "Movie_SQMotion_M";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_SQMotion_AUTO:
        ts << "Movie_SQMotion_AUTO";
        break;
     case SDK::CrExposureProgram::CrExposure_Flash_Off:
         ts << "FlashOff";
         break;
     case SDK::CrExposureProgram::CrExposure_PictureEffect:
         ts << "PictureEffect";
         break;
    case SDK::CrExposureProgram::CrExposure_HiFrameRate_P:
        ts << "HiFrameRate_P";
        break;
    case SDK::CrExposureProgram::CrExposure_HiFrameRate_A:
        ts << "HiFrameRate_A";
        break;
    case SDK::CrExposureProgram::CrExposure_HiFrameRate_S:
        ts << "HiFrameRate_S";
        break;
    case SDK::CrExposureProgram::CrExposure_HiFrameRate_M:
        ts << "HiFrameRate_M";
        break;
    case SDK::CrExposureProgram::CrExposure_SQMotion_P:
        ts << "SQMotion_P";
        break;
    case SDK::CrExposureProgram::CrExposure_SQMotion_A:
        ts << "SQMotion_A";
        break;
    case SDK::CrExposureProgram::CrExposure_SQMotion_S:
        ts << "SQMotion_S";
        break;
    case SDK::CrExposureProgram::CrExposure_SQMotion_M:
        ts << "SQMotion_M";
        break;
    case SDK::CrExposureProgram::CrExposure_MOVIE:
        ts << "MOVIE";
        break;
    case SDK::CrExposureProgram::CrExposure_STILL:
        ts << "STILL";
        break;
    case SDK::CrExposureProgram::CrExposure_Movie_F_Mode:
        ts << "Movie_F_Mode";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_still_capture_mode(std::uint32_t still_capture_mode)
{
    text_stringstream ts;
    switch (still_capture_mode) {
    case SDK::CrDriveMode::CrDrive_Single:
        ts << "CrDrive_Single";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Hi:
        ts << "CrDrive_Continuous_Hi";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Hi_Plus:
        ts << "CrDrive_Continuous_Hi_Plus";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Hi_Live:
        ts << "CrDrive_Continuous_Hi_Live";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Lo:
        ts << "CrDrive_Continuous_Lo";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous:
        ts << "CrDrive_Continuous";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_SpeedPriority:
        ts << "CrDrive_Continuous_SpeedPriority";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Mid:
        ts << "CrDrive_Continuous_Mid";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Mid_Live:
        ts << "CrDrive_Continuous_Mid_Live";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Lo_Live:
        ts << "CrDrive_Continuous_Lo_Live";
        break;
    case SDK::CrDriveMode::CrDrive_SingleBurstShooting_lo:
        ts << "CrDrive_SingleBurstShooting_lo";
        break;
    case SDK::CrDriveMode::CrDrive_SingleBurstShooting_mid:
        ts << "CrDrive_SingleBurstShooting_mid";
        break;
    case SDK::CrDriveMode::CrDrive_SingleBurstShooting_hi:
        ts << "CrDrive_SingleBurstShooting_hi";
        break;
    case SDK::CrDriveMode::CrDrive_Timelapse:
        ts << "CrDrive_Timelapse";
        break;
    case SDK::CrDriveMode::CrDrive_Timer_2s:
        ts << "CrDrive_Timer_2s";
        break;
    case SDK::CrDriveMode::CrDrive_Timer_5s:
        ts << "CrDrive_Timer_5s";
        break;
    case SDK::CrDriveMode::CrDrive_Timer_10s:
        ts << "CrDrive_Timer_10s";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_03Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_03Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_03Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_03Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_03Ev_9pics:
        ts << "CrDrive_Continuous_Bracket_03Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_05Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_05Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_05Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_05Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_05Ev_9pics:
        ts << "CrDrive_Continuous_Bracket_05Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_07Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_07Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_07Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_07Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_07Ev_9pics:
        ts << "CrDrive_Continuous_Bracket_07Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_10Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_10Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_10Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_10Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_10Ev_9pics:
        ts << "CrDrive_Continuous_Bracket_10Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_20Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_20Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_20Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_20Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_30Ev_3pics:
        ts << "CrDrive_Continuous_Bracket_30Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Bracket_30Ev_5pics:
        ts << "CrDrive_Continuous_Bracket_30Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_03Ev_3pics:
        ts << "CrDrive_Single_Bracket_03Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_03Ev_5pics:
        ts << "CrDrive_Single_Bracket_03Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_03Ev_9pics:
        ts << "CrDrive_Single_Bracket_03Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_05Ev_3pics:
        ts << "CrDrive_Single_Bracket_05Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_05Ev_5pics:
        ts << "CrDrive_Single_Bracket_05Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_05Ev_9pics:
        ts << "CrDrive_Single_Bracket_05Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_07Ev_3pics:
        ts << "CrDrive_Single_Bracket_07Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_07Ev_5pics:
        ts << "CrDrive_Single_Bracket_07Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_07Ev_9pics:
        ts << "CrDrive_Single_Bracket_07Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_10Ev_3pics:
        ts << "CrDrive_Single_Bracket_10Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_10Ev_5pics:
        ts << "CrDrive_Single_Bracket_10Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_10Ev_9pics:
        ts << "CrDrive_Single_Bracket_10Ev_9pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_20Ev_3pics:
        ts << "CrDrive_Single_Bracket_20Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_20Ev_5pics:
        ts << "CrDrive_Single_Bracket_20Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_30Ev_3pics:
        ts << "CrDrive_Single_Bracket_30Ev_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Single_Bracket_30Ev_5pics:
        ts << "CrDrive_Single_Bracket_30Ev_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_WB_Bracket_Lo:
        ts << "CrDrive_WB_Bracket_Lo";
        break;
    case SDK::CrDriveMode::CrDrive_WB_Bracket_Hi:
        ts << "CrDrive_WB_Bracket_Hi";
        break;
    case SDK::CrDriveMode::CrDrive_DRO_Bracket_Lo:
        ts << "CrDrive_DRO_Bracket_Lo";
        break;
    case SDK::CrDriveMode::CrDrive_DRO_Bracket_Hi:
        ts << "CrDrive_DRO_Bracket_Hi";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_3pics:
        ts << "CrDrive_Continuous_Timer_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_5pics:
        ts << "CrDrive_Continuous_Timer_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_2s_3pics:
        ts << "CrDrive_Continuous_Timer_2s_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_2s_5pics:
        ts << "CrDrive_Continuous_Timer_2s_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_5s_3pics:
        ts << "CrDrive_Continuous_Timer_5s_3pics";
        break;
    case SDK::CrDriveMode::CrDrive_Continuous_Timer_5s_5pics:
        ts << "CrDrive_Continuous_Timer_5s_5pics";
        break;
    case SDK::CrDriveMode::CrDrive_LPF_Bracket:
        ts << "CrDrive_LPF_Bracket";
        break;
    case SDK::CrDriveMode::CrDrive_RemoteCommander:
        ts << "CrDrive_RemoteCommander";
        break;
    case SDK::CrDriveMode::CrDrive_MirrorUp:
        ts << "CrDrive_MirrorUp";
        break;
    case SDK::CrDriveMode::CrDrive_SelfPortrait_1:
        ts << "CrDrive_SelfPortrait_1";
        break;
    case SDK::CrDriveMode::CrDrive_SelfPortrait_2:
        ts << "CrDrive_SelfPortrait_2";
        break;
    default:
        break;
    }
    return ts.str();
}

text format_focus_mode(std::uint16_t focus_mode)
{
    text_stringstream ts;

    switch (focus_mode) {
    case SDK::CrFocusMode::CrFocus_MF:
        ts << "MF";
        break;
    case SDK::CrFocusMode::CrFocus_AF_S:
        ts << "AF_S";
        break;
    case SDK::CrFocusMode::CrFocus_AF_C:
        ts << "AF_C";
        break;
    case SDK::CrFocusMode::CrFocus_AF_A:
        ts << "AF_A";
        break;
    case SDK::CrFocusMode::CrFocus_DMF:
        ts << "DMF";
        break;
    case SDK::CrFocusMode::CrFocus_AF_D:
        ts << "AF_D";
        break;
    case SDK::CrFocusMode::CrFocus_PF:
        ts << "PF";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_focus_area(std::uint16_t focus_area)
{
    text_stringstream ts;

    switch (focus_area) {
    case SDK::CrFocusArea::CrFocusArea_Unknown:
        ts << "Unknown";
        break;
    case SDK::CrFocusArea::CrFocusArea_Wide:
        ts << "Wide";
        break;
    case SDK::CrFocusArea::CrFocusArea_Zone:
        ts << "Zone";
        break;
    case SDK::CrFocusArea::CrFocusArea_Center:
        ts << "Center";
        break;
    case SDK::CrFocusArea::CrFocusArea_Flexible_Spot_S:
        ts << "Flexible Spot S";
        break;
    case SDK::CrFocusArea::CrFocusArea_Flexible_Spot_M:
        ts << "Flexible Spot M";
        break;
    case SDK::CrFocusArea::CrFocusArea_Flexible_Spot_L:
        ts << "Flexible Spot L";
        break;
    case SDK::CrFocusArea::CrFocusArea_Expand_Flexible_Spot:
        ts << "Expand Flexible Spot";
        break;
    case SDK::CrFocusArea::CrFocusArea_Flexible_Spot:
        ts << "Flexible Spot";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Wide:
        ts << "Tracking Wide";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Zone:
        ts << "Tracking Zone";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Center:
        ts << "Tracking Center";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Flexible_Spot_S:
        ts << "Tracking Flexible Spot S";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Flexible_Spot_M:
        ts << "Tracking Flexible Spot M";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Flexible_Spot_L:
        ts << "Tracking Flexible Spot L";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Expand_Flexible_Spot:
        ts << "Tracking Expand Flexible Spot";
        break;
    case SDK::CrFocusArea::CrFocusArea_Tracking_Flexible_Spot:
        ts << "Tracking Flexible Spot";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_live_view_image_quality(std::uint16_t live_view_image_quality)
{
    text_stringstream ts;

    switch (live_view_image_quality) {
    case SDK::CrPropertyLiveViewImageQuality::CrPropertyLiveViewImageQuality_High:
        ts << "High";
        break;
    case SDK::CrPropertyLiveViewImageQuality::CrPropertyLiveViewImageQuality_Low:
        ts << "Low";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_live_view_status(std::uint16_t live_view_status)
{
    text_stringstream ts;

    switch (live_view_status) {
    case SDK::CrLiveViewStatus::CrLiveView_Disable:
        ts << "Disable";
        break;
    case SDK::CrLiveViewStatus::CrLiveView_Enable:
        ts << "Enabled";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_media_slotx_format_enable_status(std::uint8_t media_slotx_format_enable_status)
{
    text_stringstream ts;

    switch (media_slotx_format_enable_status) {
    case SDK::CrMediaFormat::CrMediaFormat_Disable:
        ts << "Disable";
        break;
    case SDK::CrMediaFormat::CrMediaFormat_Enable:
        ts << "Enabled";
        break;
    default:
        break;
    }

    return ts.str();
}
text format_white_balance(std::uint16_t value)
{
    text_stringstream ts;

    switch (value) {
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_AWB:
        ts << "AWB";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Underwater_Auto:
        ts << "Underwater_Auto";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Daylight:
        ts << "Daylight";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Shadow:
        ts << "Shadow";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Cloudy:
        ts << "Cloudy";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Tungsten:
        ts << "Tungsten";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Fluorescent:
        ts << "Fluorescent";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Fluorescent_WarmWhite:
        ts << "Fluorescent_WarmWhite";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Fluorescent_CoolWhite:
        ts << "Fluorescent_CoolWhite";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Fluorescent_DayWhite:
        ts << "Fluorescent_DayWhite";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Fluorescent_Daylight:
        ts << "Fluorescent_Daylight";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Flush:
        ts << "Flush";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_ColorTemp:
        ts << "ColorTemp";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Custom_1:
        ts << "Custom_1";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Custom_2:
        ts << "Custom_2";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Custom_3:
        ts << "Custom_3";
        break;
    case SDK::CrWhiteBalanceSetting::CrWhiteBalance_Custom:
        ts << "Custom";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_customwb_capture_stanby(std::uint16_t customwb_capture_stanby)
{
    text_stringstream ts;

    switch (customwb_capture_stanby) {
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Disable:
        ts << "Disable";
        break;
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Enable:
        ts << "Enable";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_customwb_capture_stanby_cancel(std::uint16_t customwb_capture_stanby_cancel)
{
    text_stringstream ts;

    switch (customwb_capture_stanby_cancel) {
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Disable:
        ts << "Disable";
        break;
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Enable:
        ts << "Enable";
        break;

    default:
        break;
    }

    return ts.str();
}

text format_customwb_capture_operation(std::uint16_t customwb_capture_operation)
{
    text_stringstream ts;

    switch (customwb_capture_operation) {
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Disable:
        ts << "Disable";
        break;
    case SDK::CrPropertyCustomWBOperation::CrPropertyCustomWBOperation_Enable:
        ts << "Enabled";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_customwb_capture_execution_state(std::uint16_t customwb_capture_execution_state)
{
    text_stringstream ts;

    switch (customwb_capture_execution_state) {
    case SDK::CrPropertyCustomWBExecutionState::CrPropertyCustomWBExecutionState_Invalid:
        ts << "Invalid";
        break;
    case SDK::CrPropertyCustomWBExecutionState::CrPropertyCustomWBExecutionState_Standby:
        ts << "Standby";
        break;
    case SDK::CrPropertyCustomWBExecutionState::CrPropertyCustomWBExecutionState_Capturing:
        ts << "Capturing";
        break;
    case SDK::CrPropertyCustomWBExecutionState::CrPropertyCustomWBExecutionState_OperatingCamera:
        ts << "OperatingCamera";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_zoom_operation_status(std::uint8_t zoom_operation_status)
{
    text_stringstream ts;

    switch (zoom_operation_status) {
    case SDK::CrZoomOperationEnableStatus::CrZoomOperationEnableStatus_Disable:
        ts << "Disable";
        break;
    case SDK::CrZoomOperationEnableStatus::CrZoomOperationEnableStatus_Enable:
        ts << "Enable";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_zoom_setting_type(std::uint8_t zoom_setting_type)
{
    text_stringstream ts;

    switch (zoom_setting_type) {
    case SDK::CrZoomSettingType::CrZoomSetting_OpticalZoomOnly:
        ts << "OpticalZoom";
        break;
    case SDK::CrZoomSettingType::CrZoomSetting_SmartZoomOnly:
        ts << "SmartZoom";
        break;
    case SDK::CrZoomSettingType::CrZoomSetting_On_ClearImageZoom:
        ts << "ClearImageZoom";
        break;
    case SDK::CrZoomSettingType::CrZoomSetting_On_DigitalZoom:
        ts << "DigitalZoom";
        break;
    default:
        break;
    }

    return ts.str();
}

text format_zoom_types_status(std::uint8_t zoom_types_status)
{
    text_stringstream ts;

    switch (zoom_types_status) {
    case SDK::CrZoomTypeStatus::CrZoomTypeStatus_OpticalZoom:
        ts << "OpticalZoom";
        break;
    case SDK::CrZoomTypeStatus::CrZoomTypeStatus_SmartZoom:
        ts << "SmartZoom";
        break;
    case SDK::CrZoomTypeStatus::CrZoomTypeStatus_ClearImageZoom:
        ts << "ClearImageZoom";
        break;
    case SDK::CrZoomTypeStatus::CrZoomTypeStatus_DigitalZoom:
        ts << "DigitalZoom";
        break;
    default:
        break;
    }

    return ts.str();
}
text format_zoom_operation(std::int8_t zoom_operation)
{
    text_stringstream ts;

    if (0 == zoom_operation) {
        ts << "Stop";
    }
    else if (1 <= zoom_operation && zoom_operation <= 8){
        ts << "Tele";
    }
    else if (-8 <= zoom_operation && zoom_operation <= -1) {
        ts << "Wide";
    }
    else {
        ts << "-";
    }

    return ts.str();
}

text format_remocon_zoom_speed_type(std::uint8_t remocon_zoom_speed_type)
{
    text_stringstream ts;

    switch (remocon_zoom_speed_type) {
    case SDK::CrRemoconZoomSpeedType::CrRemoconZoomSpeedType_Invalid:
        ts << "Invalid";
        break;
    case SDK::CrRemoconZoomSpeedType::CrRemoconZoomSpeedType_Variable:
        ts << "Variable";
        break;
    case SDK::CrRemoconZoomSpeedType::CrRemoconZoomSpeedType_Fixed:
        ts << "Fixed";
        break;
    default:
        break;
    }

    return ts.str();
}
} // namespace cli
