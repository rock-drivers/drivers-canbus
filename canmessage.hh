#ifndef CANBUS_MESSAGE_HH
#define CANBUS_MESSAGE_HH

#include <base/time.h>
#include <stdint.h>

namespace canbus
{
    enum DRIVER_TYPE
    {
	SOCKET,
	HICO,
	HICO_PCI,
	VS_CAN
    };
    
    struct Message
    {
        base::Time time;
        base::Time can_time;

        uint32_t can_id;
        uint8_t  data[8];
        uint8_t  size;
    };
}

#endif

