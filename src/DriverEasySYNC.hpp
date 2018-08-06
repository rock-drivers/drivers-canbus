#ifndef CANBUS_DRIVER_EASY_SYNC_HH
#define CANBUS_DRIVER_EASY_SYNC_HH

#include <iodrivers_base/Driver.hpp>
#include <canbus/Driver.hpp>

namespace canbus
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class DriverEasySYNC : public iodrivers_base::Driver, public Driver
    {
    public:
        static const int MAX_PACKET_SIZE = 1024;
        static const int DEFAULT_QUEUE_SIZE = 20;

    public:
        struct FailedCommand : std::runtime_error {
            FailedCommand(std::string const& msg)
                : std::runtime_error(msg) {}
        };

        enum BUS_STATE
        {
            OK,
            WARNING,
            PASSIVE,
            OFF
        };

        struct Status
        {
            base::Time time;

            BUS_STATE rx_state;
            BUS_STATE tx_state;
            bool rx_buffer0_overflow;
            bool rx_buffer1_overflow;
        };

        DriverEasySYNC(int queue_size = DEFAULT_QUEUE_SIZE);

        /** Opens the device
         *
         * Append a CAN rate after a colon to configure the bus to this rate.
         * The rate must be one of 10k, 20k, 50k, 100k, 125k, 250k, 500k, 800k,
         * 1M.
         *
         * For instance, serial:///dev/ttyUSB0:115200:10k will open the device
         * on /dev/ttyUSB0 with a serial baud rate of 115200, using 10k CAN
         * rate.
         */
        virtual bool open(std::string const& path);

        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for write access is write()
         */
        virtual void     setWriteTimeout(uint32_t timeout);
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        virtual uint32_t getWriteTimeout() const;
        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read()
         */
        virtual void     setReadTimeout(uint32_t timeout);
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        virtual uint32_t getReadTimeout() const;

        /** Resets the chip */
        virtual bool resetBoard();

        /** Resets the CAN interface
         *
         * @return false on error, true on success
         */
        virtual bool reset();

        virtual Message read();

        Message read(int timeout_ms);

        virtual void write(Message const& msg);

        int readWriteReply(int timeout = 0);

        /** Read the CAN timestamps from the board
         *
         * They are fairly unreliable in full-duplex situations, so they are
         * disabled by default
         *
         * @see setUseBoardTimestamps
         */
        bool useBoardTimestamps() const;

        /** Change whether the driver will attempt to use the board-reported timestamps
         *
         * They are fairly unreliable in full-duplex situations, so they are
         * disabled by default
         *
         * @see useBoardTimestamps
         */
        void setUseBoardTimestamps(bool use);

        /** Removes all pending messages from the RX queue
         */
        virtual void clear();

        /** Returns the file descriptor associated with this object. If no file
         * descriptor is assigned, returns INVALID_FD
         */
        virtual int getFileDescriptor() const;

        /** True if a valid file descriptor is assigned to this object */
        virtual bool isValid() const;

        /** Closes the file descriptor */
        virtual void close();

        /** Gets the controller status bitfield
         */
        Status getStatus();

        /** Whether the controller is OK
         */
        bool checkBusOk();

        /** Returns the number of messages queued in the board's RX queue
         */
        virtual int getPendingMessagesCount();

    private:
        char mCurrentCommand;
        bool mUseBoardTimestamps = false;
        void writeCommand(char const* cmd, int commandSize);
        void writeCommand(uint8_t const* cmd, int commandSize);
        int readReply(char cmd, uint8_t* buffer);
        int readReply(char cmd, uint8_t* buffer, int timeout);
        void processSimpleCommand(char const* cmd, int commandSize);
        void processSimpleCommand(uint8_t const* cmd, int commandSize);
        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

        std::vector<canbus::Message> mQueue;
        Message readFromIO(int timeout_ms);
    };
}

#endif
