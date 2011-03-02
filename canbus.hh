#ifndef CANBUS_HH
#define CANBUS_HH
#include "canmessage.hh"
#include <string>
#define CANBUS_VERSION 101

namespace canbus
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class Driver
    {
    public:
	virtual ~Driver();
        /** The default timeout value in milliseconds
         *
         * @see setTimeout, getTimeout
         */
        static const int DEFAULT_TIMEOUT = 100;

        /** Opens the given device and resets the CAN interface. It returns
         * true if the initialization was successful and false otherwise
         */
        virtual bool open(std::string const& path) = 0;

        /** Resets the CAN board. This must be called before
	 *  any calls to reset() on any of the interfaces of the same
	 *  board
         *
         * @return false on error, true on success
         */
        virtual bool reset_board() = 0;

        /** Resets the CAN interface
         *
         * @return false on error, true on success
         */
        virtual bool reset() = 0;

        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for write access is write()
         */
        virtual void     setWriteTimeout(uint32_t timeout) = 0;
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        virtual uint32_t getWriteTimeout() const = 0;
        /** Sets the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read()
         */
        virtual void     setReadTimeout(uint32_t timeout) = 0;
        /** Returns the timeout, in milliseconds, for which we are allowed to wait
         * for packets in read() or for write access in write()
         */
        virtual uint32_t getReadTimeout() const = 0;

        /** Reads the next message. It is guaranteed to not block longer than the
         * timeout provided by setReadTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        virtual Message read() = 0;

        /** Writes a message. It is guaranteed to not block longer than the
         * timeout provided in setWriteTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        virtual void write(Message const& msg) = 0;

        /** Returns the number of messages queued in the board's RX queue
         */
        virtual int getPendingMessagesCount() = 0;

	/** Checks if bus reports error, this may indicate a disconnected cable
	 *  this method will only report an error after an message was written
	 *  to the bus
	 */
	virtual bool checkBusOk() = 0;
      

        /** Removes all pending messages from the RX queue
         */
        virtual void clear() = 0;

	/** Returns the file descriptor associated with this object. If no file
	 * descriptor is assigned, returns INVALID_FD
	 */
	virtual int getFileDescriptor() const = 0;

	/** True if a valid file descriptor is assigned to this object */
	virtual bool isValid() const = 0;

	/** Closes the file descriptor */
	virtual void close() = 0;
    };

    Driver *openCanDevice(std::string const& path, DRIVER_TYPE dType = SOCKET);
}

#endif

