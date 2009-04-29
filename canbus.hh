#ifndef CANBUS_HH
#define CANBUS_HH
#include "hico_api.h"
#include "canmessage.hh"
#include "iodrivers_base.hh"
#include <string>


namespace can
{
    class Driver : public IODriver
    {
        static const int INVALID_FD = -1;

        bool sendSetupIOCTL(std::string const& name, int cmd);
        template<typename T>
        bool sendSetupIOCTL(std::string const& name, int cmd, T arg);

        uint32_t m_timeout;

        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

    public:
        Driver();

        bool open(std::string const& path);
        bool isReady() const;

        bool reset();
        static bool reset(int fd);

        void     setTimeout(uint32_t timeout);
        uint32_t getTimeout() const;

        Message read();
        void write(Message const& msg);
    };
}

#endif

