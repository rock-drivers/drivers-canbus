#ifndef CANBUS_MESSAGE_HH
#define CANBUS_MESSAGE_HH

#include <dfki/base_types.h>
namespace can
{
    struct Message
    {
        DFKI::Time timestamp;

        uint32_t can_id;
        uint8_t  data[8];
        uint8_t  size;
    };
}

#endif

