#ifndef CANBUS_2WEB_HH
#define CANBUS_2WEB_HH
#include "canmessage.hh"
#include <iodrivers_base/Driver.hpp>
#include "canbus.hh"
#include <string>


namespace canbus
{
    /** This class allows to (i) setup a CAN interface and (ii) having read and
     * write access to it.
     */
    class Driver2Web : public iodrivers_base::Driver, public Driver
    {
        //bool sendSetupIOCTL(std::string const& name, int cmd);
        //template<typename T>
        //bool sendSetupIOCTL(std::string const& name, int cmd, T arg);

        uint32_t m_read_timeout;
        uint32_t m_write_timeout;
	BAUD_RATE m_baudrate;
	Status m_status;

        int extractPacket(uint8_t const* buffer, size_t buffer_size) const;

    public:
        /** The default timeout value in milliseconds
         *
         * @see setTimeout, getTimeout
         */
        static const int DEFAULT_TIMEOUT = 100;

        Driver2Web();
	
	/** Opens the given device and resets the CAN interface. It returns
         * true if the initialization was successful and false otherwise
         */
        bool open(std::string const& path);
	
	/** method does nothing, has to be implemented because its abstract in base class
         *
         * @return true 
         */
        bool resetBoard();
        

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
	
	/** Sets the baudrate
	 * */
	void setBaudrate(BAUD_RATE);
	
	/** Gets the Controller Status
	 * */
	Status getStatus() const;

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
	
	/** Writes a ascii string. It is guaranteed to not block longer than the
         * timeout provided in setWriteTimeout().
         *
         * The default timeout value is given by DEFAULT_TIMEOUT
         */
        void write(uint8_t *s);
	
	int getPendingMessagesCount();

        /** method does nothing, has to be implemented because its abstract in base class
         *
         * @return true 
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

