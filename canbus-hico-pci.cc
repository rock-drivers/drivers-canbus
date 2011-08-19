#include "canbus-hico-pci.hh"


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#define BAUDRATE HiCOCAN_BAUD1M

using namespace canbus;

DriverHicoPCI::DriverHicoPCI()
    : iodrivers_base::Driver(sizeof(canMsg))
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
/*
DriverHicoPCI::~DriverHicoPCI()
{ // done
  int fd = getFileDescriptor();
  // stop the node
  SEND_IOCTL(IOC_STOP);
  
  // close the node
  ::close();
  
}*/

bool DriverHicoPCI::resetBoard()
{//done ?
    if (!DriverHicoPCI::resetBoard(m_fd))
        return false;

    /* Leave this in this order. For some reason it is more accurate */
    timestampBase = base::Time::now();
   // int fd = m_fd; // for SEND_IOCTL
    
    return true;
}
bool DriverHicoPCI::resetBoard(int fd)
{//done
	SEND_IOCTL(IOC_RESET_BOARD);
  
    return true;
}

bool DriverHicoPCI::reset()
{ //done
    if (!DriverHicoPCI::reset(m_fd))
        return false;

    timestampBase = base::Time::fromSeconds(0);
    return true;
}
bool DriverHicoPCI::reset(int fd)
{//done
     
    SEND_IOCTL(IOC_STOP); 
    SEND_IOCTL(IOC_START);
    
    return true;
}

bool DriverHicoPCI::setBaudRate(int fd, int Rate)
{//done
   canParam parameters;
   
   parameters.baud = Rate;
   	
   SEND_IOCTL_2(IOC_SET_BAUD, &parameters);
  
   return true;
      	
}

void DriverHicoPCI::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }
uint32_t DriverHicoPCI::getReadTimeout() const
{ return m_read_timeout; }
void DriverHicoPCI::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }
uint32_t DriverHicoPCI::getWriteTimeout() const
{ return m_write_timeout; }

bool DriverHicoPCI::open(std::string const& path)
{ // done
    if (isValid())
        close();

    int fd = ::open(path.c_str(), O_RDWR);
   
    if (fd == INVALID_FD)
        return false;
  
    // set the baudrate
    bool ret = false;
    
    ret = setBaudRate(fd, BAUDRATE);    
        
    if(!ret)
        return false;  
        
    //deactivate filtering, activate non blocking    
    int flags = fcntl(fd,F_GETFL);
        
    fcntl(fd, F_SETFL, ( flags | O_NONBLOCK));      
        
    iodrivers_base::FileGuard guard(fd);
    
    if (!reset(fd))
        return false;

    setFileDescriptor(guard.release());
    return true;
}

Message DriverHicoPCI::read()
{//done (have a look at the timestamp)
    canMsg msg;
    readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(canMsg), m_read_timeout);

    Message result;
    
    result.time     = base::Time::now();
    result.can_time = base::Time::fromMicroseconds(msg.ts.us) +
      timestampBase;
    result.can_id        = msg.id;
    memcpy(result.data, msg.data, 8);
    result.size          = msg.dlc;
    return result;
}

void DriverHicoPCI::write(Message const& msg)
{//done
    canMsg out;
    memset(&out, 0, sizeof(canMsg));
    out.id  = msg.can_id;
    out.rtr = HiCOCAN_NORMAL_FRAME;
    out.ff  = HiCOCAN_FORMAT_BASIC;
    memcpy(out.data, msg.data, 8);
    out.dlc = msg.size;
  
    writePacket(reinterpret_cast<uint8_t*>(&out), sizeof(canMsg), m_write_timeout);
}

int DriverHicoPCI::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{ //what is this for?
    if (buffer_size == sizeof(canMsg))
        return sizeof(canMsg);
    else
        return -buffer_size;
}

int DriverHicoPCI::getPendingMessagesCount()
{//done
    int count = 0;
    canState curstat;
    int fd = getFileDescriptor();
    
    SEND_IOCTL_2(IOC_GET_CAN_STATE, &curstat);
    
    count = curstat.recBuf; //maybe .recQ
  
    return count;
}

bool DriverHicoPCI::checkBusOk() 
{//done
  uint8_t status;
  canState curstat;
  
  int fd = getFileDescriptor();
  
  SEND_IOCTL_2(IOC_GET_CAN_STATE, &curstat);
  
  status = curstat.state;
  
  if((status & C_ERR_PSV) || 
     (status & C_BUSOFF)){
    return false;
  }

  return true;
}

void DriverHicoPCI::clear()
{//done
    int count = getPendingMessagesCount();
    for (int i = 0; i < count; ++i)
    {
        canMsg msg;
        readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(canMsg), m_read_timeout);
    }
}

int DriverHicoPCI::getFileDescriptor() const 
{// not touched
    return iodrivers_base::Driver::getFileDescriptor();
}

/** True if a valid file descriptor is assigned to this object */
bool DriverHicoPCI::isValid() const
{// not touched
    return iodrivers_base::Driver::isValid();
}

/** Closes the file descriptor */
void DriverHicoPCI::close()
{// not touched
    iodrivers_base::Driver::close();
}

