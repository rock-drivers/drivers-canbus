#ifndef CANBUS_SOCKET_HH
#define CANBUS_SOCKET_HH
#include "canmessage.hh"
#include <string>
#include <deque>
#include "canbus.hh"


class Timeout;

namespace can
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class DriverSocket : public Driver
    {
    private:
        uint32_t m_read_timeout;
        uint32_t m_write_timeout;

	int m_fd;

	bool checkInput(Timeout timeout);

	std::deque<Message> rx_queue;
	bool m_error;
    public:
        /** The default timeout value in milliseconds
         *
         * @see setTimeout, getTimeout
         */
        static const int DEFAULT_TIMEOUT = 100;

        DriverSocket();

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
        bool reset_board() { return true; }
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
    };
}

#endif

