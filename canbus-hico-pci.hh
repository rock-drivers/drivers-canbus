#ifndef CANBUS_HICO_PCI_HH
#define CANBUS_HICO_PCI_HH
#include "canmessage.hh"
#include "iodrivers_base.hh"
#include "canbus.hh"
#include <string>
#include "hicocan.h"


namespace canbus
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class DriverHicoPCI : public IODriver, public Driver
    {
        bool sendSetupIOCTL(std::string const& name, int cmd);
        template<typename T>
        bool sendSetupIOCTL(std::string const& name, int cmd, T arg);

        uint32_t m_read_timeout;
        uint32_t m_write_timeout;

        base::Time timestampBase;

        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

    public:
        /** The default timeout value in milliseconds
         *
         * @see setTimeout, getTimeout
         */
        static const int DEFAULT_TIMEOUT = 100;

        DriverHicoPCI();
        ~DriverHicoPCI();

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
        bool reset_board();
        /** Resets the CAN board. This must be called before
	 *  any calls to reset() on any of the interfaces of the same
	 *  board
         *
         * @return false on error, true on success
         */
        static bool reset_board(int fd);
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
 
  private:
    bool setBaudRate(int fd, int Rate);
	
public : 
        canParam canbusParameters;
        
        int can_node;
        	
	
    };
}

#endif

