#ifndef CANBUS_HH
#define CANBUS_HH
#include "hico_api.h"
#include "canmessage.hh"
#include "iodrivers_base.hh"
#include <string>


namespace can
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class Driver : public IODriver
    {
        bool sendSetupIOCTL(std::string const& name, int cmd);
        template<typename T>
        bool sendSetupIOCTL(std::string const& name, int cmd, T arg);

        uint32_t m_timeout;

        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

    public:
        /** The default timeout value in milliseconds
         *
         * @see setTimeout, getTimeout
         */
        static const int DEFAULT_TIMEOUT = 100;

        Driver();

        /** Opens the given device and resets the CAN interface. It returns
         * true if the initialization was successful and false otherwise
         */
        bool open(std::string const& path);

        /** Resets the CAN interface
         *
         * @return false on error, true on success
         */
        bool reset();
        /** Resets the given CAN interface
         *
         * @return true on success, false on error.
         */
        static bool reset(int fd);

        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        void     setTimeout(uint32_t timeout);
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        uint32_t getTimeout() const;

        /** Reads the next message. It is guaranteed to not block longer than the
         * timeout provided in setTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        Message read();

        /** Writes a message. It is guaranteed to not block longer than the
         * timeout provided in setTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        void write(Message const& msg);
    };
}

#endif

