#include "ConnectionInfo.h"
#include <cstddef>
#include <cstring>
#include "CRSDK/CameraRemote_SDK.h"

namespace impl
{
constexpr std::size_t const MACADDRESSBYTE    = 6;
constexpr std::size_t const CAMERANAMELENGMAX = 256;
constexpr std::size_t const CAMERADESCLENGMAX = 256;

struct NetworkDeviceInfo
{
    CrInt32u idsize;
    CrInt32u ipaddress;
    CrInt8u  name[CAMERANAMELENGMAX];
    CrInt8u  desc[CAMERADESCLENGMAX];
    CrInt8u  MACaddress[MACADDRESSBYTE];
    CrInt32u urlsize;
};

} // namespace impl

namespace cli
{
ConnectionType parse_connection_type(text conn_type)
{
    ConnectionType type(ConnectionType::UNKNOWN);

    if (TEXT("IP") == conn_type) {
        type = ConnectionType::NETWORK;
    }
    else if (TEXT("USB") == conn_type) {
        type = ConnectionType::USB;
    }
    return type;
}

NetworkInfo parse_ip_info(unsigned char const* buf, std::uint32_t size)
{
    NetworkInfo info;
    impl::NetworkDeviceInfo raw_info;
    std::memcpy(&raw_info, buf, sizeof raw_info);

    // IP address
    info.ip_address = raw_info.ipaddress; // 4byte

    // IP address string
    char pip[32] = { 0 };
    sprintf(pip, "%d.%d.%d.%d",
        (raw_info.ipaddress & 0x000000FF),
        ((raw_info.ipaddress >> 8) & 0x000000FF),
        ((raw_info.ipaddress >> 16) & 0x000000FF),
        ((raw_info.ipaddress >> 24) & 0x000000FF));
#if defined(WIN32) || defined(_WIN64)
    wchar_t wpip[16];
    mbstowcs(wpip, pip, 16);
    info.ip_address_fmt.append(wpip, 16); // format
#else
    info.ip_address_fmt = (TCHAR*)pip; // format
#endif

    // Mac address string
    char chData[32] = { 0 };
    sprintf(chData, "%02X:%02X:%02X:%02X:%02X:%02X",
        raw_info.MACaddress[0], raw_info.MACaddress[1],
        raw_info.MACaddress[2], raw_info.MACaddress[3],
        raw_info.MACaddress[4], raw_info.MACaddress[5]);
#if defined(WIN32) || defined(_WIN64)
    wchar_t wchData[20] = { 0 };
    mbstowcs(wchData, chData, 20);
    info.mac_address.append(wchData, 20);
#else
    info.mac_address = (TCHAR*)chData;
#endif
    return info;
}

} // namespace cli
