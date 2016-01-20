#ifndef CANBUS_MESSAGE_HH
#define CANBUS_MESSAGE_HH

#include <base/time.h>
#include <stdint.h>

#define CAN_ERR_OK  		0x00
#define CAN_ERR_XMTFULL  	0x01
#define CAN_ERR_OVERRUN  	0x02
#define CAN_ERR_BUSERR 		0x04
#define CAN_ERR_BUSOFF  	0x08
#define CAN_ERR_RX_OVERFLOW	0x10
#define CAN_ERR_TX_OVERFLOW 	0x20

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
    
    struct Message
    {
        base::Time time;
        base::Time can_time;

        uint32_t can_id;
        uint8_t  data[8];
        uint8_t  size;
    };
    
    struct Status
    {
      base::Time time;
      uint8_t error;
    };
      
}

#endif

