#include <canbus/DriverHico.hpp>
#include "vendor/hico_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

using namespace canbus;

DriverHico::DriverHico()
    : iodrivers_base::Driver(sizeof(can_msg))
    , m_read_timeout(DEFAULT_TIMEOUT)
    , m_write_timeout(DEFAULT_TIMEOUT) {}

#define SEND_IOCTL(cmd) {\
    int ret = ioctl(fd, cmd); \
    if (ret == -1) \
    { \
        perror(#cmd); \
        return false; \
    } \
}
#define SEND_IOCTL_2(cmd, arg) {\
    int ret = ioctl(fd, cmd, arg); \
    if (ret == -1) \
    { \
        perror(#cmd); \
        return false; \
    } \
}

bool DriverHico::resetBoard()
{
    if (!DriverHico::resetBoard(getFileDescriptor()))
        return false;

    /* Leave this in this order. For some reason it is more accurate */
    timestampBase = base::Time::now();
    int fd = getFileDescriptor(); // for SEND_IOCTL
    SEND_IOCTL(IOC_RESET_TIMESTAMP);
    return true;
}
bool DriverHico::resetBoard(int fd)
{
    SEND_IOCTL(IOC_RESET_BOARD);
    return true;
}

bool DriverHico::reset()
{
    if (!DriverHico::reset(getFileDescriptor()))
        return false;

    timestampBase = base::Time::fromSeconds(0);
    return true;
}
bool DriverHico::reset(int fd)
{
    int bitrate = static_cast<int>(BITRATE_1000k);
    SEND_IOCTL_2(IOC_SET_BITRATE, &bitrate);
    SEND_IOCTL(IOC_STOP);
    SEND_IOCTL(IOC_START);
    return true;
}

void DriverHico::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }
uint32_t DriverHico::getReadTimeout() const
{ return m_read_timeout; }
void DriverHico::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }
uint32_t DriverHico::getWriteTimeout() const
{ return m_write_timeout; }

bool DriverHico::open(std::string const& path)
{
    if (isValid())
        close();

    int fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK);
    if (fd == INVALID_FD)
        return false;

    iodrivers_base::FileGuard guard(fd);
    if (!reset(fd))
        return false;

    setFileDescriptor(guard.release());
    return true;
}

Message DriverHico::read()
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

void DriverHico::write(Message const& msg)
{
    can_msg out;
    memset(&out, 0, sizeof(can_msg));
    out.id = msg.can_id;
    memcpy(out.data, msg.data, 8);
    out.dlc   = msg.size;
    writePacket(reinterpret_cast<uint8_t*>(&out), sizeof(can_msg), m_write_timeout);
    //printf("drivers/canbus wrote: %d %d %d %d %d %d %d %d\n", out.data[0], out.data[1],out.data[2],out.data[3],out.data[4],out.data[5],out.data[6],out.data[7]);
}

int DriverHico::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    if (buffer_size == sizeof(can_msg))
        return sizeof(can_msg);
    else
        return -buffer_size;
}

int DriverHico::getPendingMessagesCount()
{
    int count = 0;
    ioctl(getFileDescriptor(), IOC_MSGS_IN_RXBUF, &count);
    return count;
}

bool DriverHico::checkBusOk() 
{
  uint32_t status;
  int fd = getFileDescriptor();
  SEND_IOCTL_2(IOC_GET_CAN_STATUS, &status);

  if((status & CS_ERROR_PASSIVE) || 
     (status & CS_ERROR_BUS_OFF)){
    return false;
  }

  return true;
}

void DriverHico::clear()
{
    int count = getPendingMessagesCount();
    for (int i = 0; i < count; ++i)
    {
        can_msg msg;
        readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(can_msg), m_read_timeout);
    }
}

int DriverHico::getFileDescriptor() const 
{
    return iodrivers_base::Driver::getFileDescriptor();
}

/** True if a valid file descriptor is assigned to this object */
bool DriverHico::isValid() const
{
    return iodrivers_base::Driver::isValid();
}

/** Closes the file descriptor */
void DriverHico::close()
{
    iodrivers_base::Driver::close();
}

