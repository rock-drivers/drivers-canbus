#ifndef CANBUS_MESSAGE_HH
#define CANBUS_MESSAGE_HH

#include <base/base_types.h>

#ifndef __orogen
#include <stdint.h>
#endif

namespace can
{
    struct Message
    {
        base::Time timestamp;
        base::Time can_timestamp;

        uint32_t can_id;
        uint8_t  data[8];
        uint8_t  size;
    };
}

#endif

