#ifndef CANBUS_MESSAGE_HH
#define CANBUS_MESSAGE_HH

#include <base/Time.hpp>
#include <stdint.h>

#define CAN_ERR_OK              0x00
#define CAN_ERR_XMTFULL         0x01
#define CAN_ERR_OVERRUN         0x02
#define CAN_ERR_BUSERR          0x04
#define CAN_ERR_BUSOFF          0x08
#define CAN_ERR_RX_OVERFLOW     0x10
#define CAN_ERR_TX_OVERFLOW     0x20

namespace canbus
{
    enum DRIVER_TYPE
    {
        SOCKET,
        HICO,
        HICO_PCI,
        VS_CAN,
        CAN2WEB,
        NET_GATEWAY,
        EASY_SYNC
    };

    /** Values used to encode specific flags in the can_id field of Message
     */
    enum MessageFlags {
        FLAG_ERROR = (1 << 29),
        FLAG_REMOTE_TRANSMISSION_REQUEST = (1 << 30),
        FLAG_EXTENDED_FRAME = (1 << 31)
    };

    /** A decoded CAN frame */
    struct Message
    {
        base::Time time;
        base::Time can_time;

        /** CAN ID and special flags
         *
         * When sending, it follows Linux' format for the can_id field, that is:
         * - ID: bits 0-11 for standard frame, or 0-28 for extended frame
         * - Error: bit 29
         * - RTR bit: bit 30
         * - Frame format: bit 30, 0 for standard, 1 for extended
         *
         * On reception, the drivers should report an error through the API's
         * error mechanism on error. They return a RTR with the flag set in the
         * ID. and do not report anything specific for an extended frame.
         *
         * Not all drivers support these flags. The only one that is currently
         * guaranteed to is the Socket-CAN driver.
         */
        uint32_t can_id;

        /** Actual data in the frame */
        uint8_t  data[8];

        /** How many valid bytes there is in data */
        uint8_t  size;
    };

    struct Status
    {
      base::Time time;
      uint8_t error;
    };

}

#endif

