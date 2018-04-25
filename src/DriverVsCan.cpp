#include <canbus/DriverVsCan.hpp>
#include "vendor/vs_can_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

using namespace canbus;

DriverVsCan::DriverVsCan()
    : m_read_timeout(DEFAULT_TIMEOUT)
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
    /* Leave this in this order. For some reason it is more accurate */
    timestampBase = base::Time::now();
    
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
    if((handle = VSCAN_Open(VSCAN_FIRST_FOUND,  VSCAN_MODE_NORMAL)) == -1)
        return false;
    
    SEND_IOCTL_2(VSCAN_IOCTL_SET_SPEED, VSCAN_SPEED_1M);

    return true;
}

bool DriverVsCan::checkForMessages(iodrivers_base::Timeout& timeout)
{
#define BUFFER_SIZE 100
    VSCAN_MSG buffer[BUFFER_SIZE];
    DWORD read_cnt;
    VSCAN_STATUS ret;

    int rx_cnt = 0;
    
    while(rx_cnt < 1)
    {
        if(timeout.elapsed())
            return false;

        if((ret = VSCAN_Read(handle, buffer, BUFFER_SIZE, &read_cnt) != VSCAN_ERR_OK))
            throw iodrivers_base::UnixError("read(): error in reading can messages");

        for(unsigned int i = 0; i < read_cnt; i++)
        {        
            VSCAN_MSG *msg = buffer + i;
            Message result;
            result.time     = base::Time::now();
            unsigned int time = msg->Timestamp;
            result.can_time = base::Time::fromMicroseconds(time * 1000) +
            timestampBase;
            result.can_id        = msg->Id;
            memcpy(result.data, msg->Data, 8);
            result.size          = msg->Size;
            rx_queue.push_back(result);
        }
        
        rx_cnt += read_cnt;        
    }
    
    return true;
}


Message DriverVsCan::read()
{
    iodrivers_base::Timeout timeout(m_read_timeout);

    if(!checkForMessages(timeout))
        throw iodrivers_base::TimeoutError(
                iodrivers_base::TimeoutError::PACKET, "read(): timeout");
    
    Message msg = rx_queue.front();
    rx_queue.pop_front();
    
    return msg;
}

void DriverVsCan::write(Message const& msg)
{
    VSCAN_MSG out;
    DWORD out_cnt;
    memset(&out, 0, sizeof(VSCAN_MSG));
    out.Id = msg.can_id;
    memcpy(out.Data, msg.data, 8);
    out.Size   = msg.size;
    if(VSCAN_Write(handle, &out, 1, &out_cnt) != VSCAN_ERR_OK)
        throw iodrivers_base::UnixError("write(): error during write");
}

int DriverVsCan::getPendingMessagesCount()
{
    iodrivers_base::Timeout t(0);
    checkForMessages(t);
    return rx_queue.size();
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
    rx_queue.clear();
}

int DriverVsCan::getFileDescriptor() const
{
    return iodrivers_base::Driver::INVALID_FD;
}

bool DriverVsCan::isValid() const
{
    return m_error;
}

/** Closes the file descriptor */
void DriverVsCan::close()
{
    VSCAN_Close(handle);
}

