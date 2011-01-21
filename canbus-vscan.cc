#include "canbus-hico.hh"
#include "vs_can_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

using namespace canbus;

DriverVsCan::DriverVsCan()
    : IODriver(sizeof(can_msg))
    , m_read_timeout(DEFAULT_TIMEOUT)
    , m_write_timeout(DEFAULT_TIMEOUT) {}

#define SEND_IOCTL(cmd) {\
    VSCAN_STATUS ret = ioctl(handle, cmd); \
    if (ret != VSCAN_ERR_OK) \
    { \
        perror(#cmd); \
        return false; \
    } \
}
#define SEND_IOCTL_2(cmd, arg) {\
    VSCAN_STATUS ret = VSCAN_Ioctl(handle, cmd, arg); \
    if (ret != VSCAN_ERR_OK) \
    { \
        perror(#cmd); \
        return false; \
    } \
}

bool DriverVsCan::reset_board()
{
   
    return true;
}
bool DriverVsCan::reset_board(int handle)
{
    return true;
}

bool DriverVsCan::reset()
{
    VSCAN_STATUS retVal = VSCAN_Ioctl(handle, VSCAN_IOCTL_SET_TIMESTAMP, VSCAN_TIMESTAMP_ON);
    if (retVal != VSCAN_ERR_OK)
        return false;

    timestampBase = base::Time::fromSeconds(0);
    return true;
}
bool DriverVsCan::reset(int handle)
{
    SEND_IOCTL_2(VSCAN_IOCTL_SET_SPEED, VSCAN_SPEED_1M);
    return true;
}

void DriverVsCan::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }
uint32_t DriverVsCan::getReadTimeout() const
{ return m_read_timeout; }
void DriverVsCan::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }
uint32_t DriverVsCan::getWriteTimeout() const
{ return m_write_timeout; }

bool DriverVsCan::open(std::string const& path)
{
    if (isValid())
        close();

    int handle = ::open(path.c_str(), O_RDWR | O_NONBLOCK);
    if (handle == INVALID_handle)
        return false;

    file_guard guard(handle);
    if (!reset(handle))
        return false;

    setFileDescriptor(guard.release());
    return true;
}

Message DriverVsCan::read()
{
    can_msg msg;
    
    readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(can_msg), m_read_timeout);

    Message result;
    result.time     = base::Time::now();
    result.can_time = base::Time::fromMicroseconds(msg.ts) +
      timestampBase;
    result.can_id        = msg.id;
    memcpy(result.data, msg.data, 8);
    result.size          = msg.dlc;
    return result;
}

void DriverVsCan::write(Message const& msg)
{
    can_msg out;
    memset(&out, 0, sizeof(can_msg));
    out.id = msg.can_id;
    memcpy(out.data, msg.data, 8);
    out.dlc   = msg.size;
    writePacket(reinterpret_cast<uint8_t*>(&out), sizeof(can_msg), m_write_timeout);
}

int DriverVsCan::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    if (buffer_size == sizeof(can_msg))
        return sizeof(can_msg);
    else
        return -buffer_size;
}

int DriverVsCan::getPendingMessagesCount()
{
    int count = 0;
    ioctl(getFileDescriptor(), IOC_MSGS_IN_RXBUF, &count);
    return count;
}

bool DriverVsCan::checkBusOk() 
{
  uint32_t status;
  SEND_IOCTL_2(VSCAN_IOCTL_GET_FLAGS, &status);

  if((status & VSCAN_IOCTL_FLAG_ERR_PASSIVE) || 
     (status & VSCAN_IOCTL_FLAG_BUS_ERROR)){
    return false;
  }

  return true;
}

void DriverVsCan::clear()
{
    int count = getPendingMessagesCount();
    for (int i = 0; i < count; ++i)
    {
        can_msg msg;
        readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(can_msg), m_read_timeout);
    }
}

int DriverVsCan::getFileDescriptor() const 
{
    return IODriver::getFileDescriptor();
}

/** True if a valid file descriptor is assigned to this object */
bool DriverVsCan::isValid() const
{
    return IODriver::isValid();
}

/** Closes the file descriptor */
void DriverVsCan::close()
{
    IODriver::close();
}

