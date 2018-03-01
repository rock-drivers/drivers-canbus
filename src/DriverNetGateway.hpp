/*
 * canbus-netgw.hh
 *
 *  Created on: 26.02.2014
 *      Author: jrenken
 */

#ifndef CANBUS_NETGW_HH
#define CANBUS_NETGW_HH

#include <deque>
#include <canbus/Message.hpp>
#include <canbus/Driver.hpp>
#include <iodrivers_base/Driver.hpp>

namespace canbus
{

    struct CanFrame {
        uint32_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
        uint8_t    can_dlc; /* data length code: 0 .. 8 */
        uint8_t    data[8] __attribute__((aligned(8)));
    };


    /*
     *
     */
    class DriverNetGateway : public iodrivers_base::Driver, public Driver
    {
        uint32_t mErrorCounter;
        bool    mError;

        std::deque<Message> rx_queue;


        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

        void readOneMessage();
        int bufferMessages();

    public:
        static const int DEFAULT_TIMEOUT = 100;


        DriverNetGateway();

        /** Opens the given device and resets the CAN interface. It returns
         * true if the initialization was successful and false otherwise
         */
        bool open(std::string const& path);

        /** Resets the CAN board. This must be called before
         *  any calls to reset() on any of the interfaces of the same
         *  board
         *
         * @return false on error, true on success
         */
        bool resetBoard() { return true; }
        /** Resets the CAN interface
         *
         * @return false on error, true on success
         */
        bool reset();

        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for write access is write()
         */
        void     setWriteTimeout(uint32_t timeout);
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        uint32_t getWriteTimeout() const;
        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read()
         */
        void     setReadTimeout(uint32_t timeout);
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        uint32_t getReadTimeout() const;

        /** Reads the next message. It is guaranteed to not block longer than the
         * timeout provided by setReadTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        Message read();

        /** Reads the next message. It is guaranteed to not block longer than the
         * timeout provided by setReadTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         *
         * @param msg   The Message will be put here, if any
         * @return   true if msg was filled in, false on timeout.
         */
        bool read(Message &msg);

        /** Writes a message. It is guaranteed to not block longer than the
         * timeout provided in setWriteTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        void write(Message const& msg);

        /** Returns the number of messages queued in the board's RX queue
         */
        int getPendingMessagesCount();

        /** Checks if bus reports error, this may indicate a disconnected cable
         *  this method will only report an error after an message was written
         *  to the bus
         */
        bool checkBusOk();


        /** Removes all pending messages from the RX queue
         */
        void clear();

        /** Returns the file descriptor associated with this object. If no file
         * descriptor is assigned, returns INVALID_FD
         */
        int getFileDescriptor() const;

        /** True if a valid file descriptor is assigned to this object */
        bool isValid() const;

        /** Closes the file descriptor */
        void close();

        virtual uint32_t getErrorCount() const;



    };

} /* namespace iocard_kontron */
#endif /* CANBUS_NETGW_HH */
