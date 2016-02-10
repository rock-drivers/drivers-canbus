#ifndef _CAN2WEB_API_H
#define _CAN2WEB_API_H

#include <string.h>

#define CAN_MSG_SIZE_MIN    7
#define CAN_START           0x81
#define CAN_START_STAT      0x82
#define CAN_START_TIME      0x83
#define CAN_MODE            0x40

namespace can2web
{

    struct can_msg
    {

        uint8_t start;
        uint8_t status;
        uint32_t can_id;
        uint8_t rtr_mode_len;
        /* CAN message data */
        uint8_t data[8];
        uint32_t secs;
        uint32_t u_secs;

        can_msg()
                : start(CAN_START), status(0), can_id(0), rtr_mode_len(
                        CAN_MODE), secs(0), u_secs(0)
        {
        }

        inline can_msg& operator<<(uint8_t const* s)
        {
            start = s[0];
            if (start < CAN_START || start > CAN_START_TIME) {
                return (*this);
            }
            status = s[1];
            if (start == 0x82) {
                return (*this);
            }
            can_id = s[5];
            can_id |= s[4] << 8;
            can_id |= s[3] << 16;
            can_id |= s[2] << 24;
            rtr_mode_len = s[6];
            uint8_t len = rtr_mode_len & 0x0F;
            for (int i = 0; i < len; i++) {
                data[i] = s[7 + i];
            }
            if (start == 0x81) {
                return (*this);
            }
            for (int i = 0; i < 4; i++) {
                int shift = 8 * (3 - i);
                secs |= s[7 + len + i] << shift;
                u_secs |= s[11 + len + i] << shift;
            }
            return (*this);
        }

        inline can_msg& operator>>(uint8_t *& s)
        {
            int msg_len = CAN_MSG_SIZE_MIN + (rtr_mode_len & 0xF);
            uint8_t* sz = new uint8_t[msg_len];
            memset(sz, 0, msg_len);
            s = sz;
            s[0] = start;
            s[1] = status;
            if (start == CAN_START_STAT) {
                return (*this);
            }
            s[5] = can_id & 0xFF;
            s[4] = (can_id & 0xFF00) >> 8;
            s[3] = (can_id & 0xFF0000) >> 16;
            s[2] = (can_id & 0xFF000000) >> 24;
            s[6] = rtr_mode_len;
            uint8_t len = rtr_mode_len & 0x0F;
            for (int i = 0; i < len; i++) {
                s[7 + i] = data[i];
            }
            if (start == 0x81) {
                return (*this);
            }
            return (*this);
        }
    };

}
#endif
