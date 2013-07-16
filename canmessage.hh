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
	VS_CAN,
	CAN2WEB
    };
    
    enum BAUD_RATE{
      br5	= 5,
      br10 	= 10,
      br20 	= 20,
      br50 	= 50,
      br100 = 100,
      br125 = 125,
      br250 = 250,
      br500 = 500,
      br1000 = 1000
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

