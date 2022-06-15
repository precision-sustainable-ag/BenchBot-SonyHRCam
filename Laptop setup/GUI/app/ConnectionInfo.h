#ifndef DEVICEID_H
#define DEVICEID_H

#include "Text.h"

namespace cli
{

enum class ConnectionType
{
    UNKNOWN,
    NETWORK,
    USB
};

struct NetworkInfo
{
    std::uint32_t ip_address;
    text ip_address_fmt;  // "192.168.0.n" dot delimiter
    text mac_address;     // "AB:CD:EF:01:23:45" colon delimiter
};

struct UsbInfo
{
    std::int16_t pid;
};

ConnectionType parse_connection_type(text conn_type);
NetworkInfo parse_ip_info(unsigned char const* buf, std::uint32_t size);

} // namespace cli

#endif // !DEVICEID_H
