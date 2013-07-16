#include "canbus-2web.hh"
#include "can2web_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>

using namespace canbus;
using namespace std;
using namespace can2web;

Driver2Web::Driver2Web()
    : iodrivers_base::Driver(CAN_MSG_SIZE_MIN+8)
    , m_read_timeout(DEFAULT_TIMEOUT)
    , m_write_timeout(DEFAULT_TIMEOUT) 
    , m_baudrate(br125) {}

bool Driver2Web::reset()
{
    char *sz = new char[128];
    sprintf(sz,"can_baudrate %i\r",(int)m_baudrate);
    write((uint8_t*)sz);
    usleep(1000);
    strcpy(sz,"can_fast 1\r");
    write((uint8_t*)sz);
    usleep(1000);
    strcpy(sz,"can_stream 1\r");
    write((uint8_t*)sz);
    return true;
}

bool Driver2Web::reset(int fd)
{
    return false;
}

void Driver2Web::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }

uint32_t Driver2Web::getWriteTimeout() const
{ return m_write_timeout; }

void Driver2Web::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }

uint32_t Driver2Web::getReadTimeout() const
{ return m_read_timeout; }

void Driver2Web::setBaudrate(BAUD_RATE br)
{
  m_baudrate = br;
}

bool Driver2Web::open(std::string const& path)
{
    cout <<"Driver2Web::open " <<path <<endl;
    if (isValid())
        close();
    openURI(path);
    return true;
}

bool Driver2Web::resetBoard()
{
    return true;
}


Message Driver2Web::read()
{
    uint8_t *msg = new uint8_t[CAN_MSG_SIZE_MIN+8];
    (*msg) = 0;
    Message result;
    while(*msg < 0x81 || *msg > 0x83){
      readPacket(msg,CAN_MSG_SIZE_MIN+8/* sizeof(msg)*/, m_read_timeout);
    }
    can_msg canMsg;
    canMsg << msg;
    result.time     = base::Time::now();
    result.can_time = base::Time::fromMicroseconds(canMsg.secs*1000000+canMsg.u_secs);
    result.can_id = canMsg.can_id;
    memcpy(result.data,canMsg.data,8);
    result.size = canMsg.rtr_mode_len && 0x0F;;
    return result;
}

void Driver2Web::write(Message const& msg)
{
    can_msg out;
    memset(&out, 0, sizeof(can_msg));
    out.start = CAN_START;
    out.status = 0;
    out.can_id = msg.can_id;
    out.rtr_mode_len = CAN_MODE | msg.size;
    memcpy(out.data, msg.data, 8);
    uint8_t* s;
    out>>s;
    int len = CAN_MSG_SIZE_MIN+(out.rtr_mode_len&0xF);
    writePacket(s, len, m_write_timeout);
    /*char szOut[512];
    sprintf(szOut,"len %i msg.size %i ",len,msg.size);
    for (int i=0;i<len;i++){
      sprintf(szOut+strlen(szOut),"%02x | ",s[i]);
    }
    cout <<"drivers/canbus wrote: " <<szOut <<endl;*/
}

void Driver2Web::write(uint8_t *sz)
{
  writePacket(sz, strlen((const char*)sz), m_write_timeout);
}

int Driver2Web::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    //cout <<"Driver2Web::extractPacket " <<buffer_size <<endl;
    /*char szOut[512];
    *szOut = 0;
    for (int i=0;i<buffer_size;i++){
      sprintf(szOut+strlen(szOut),"%02x | ",buffer[i]);
    }
    cout <<szOut <<endl;*/
    for (int i=0;i<buffer_size;i++){
      if(buffer[i]>=0x81 && buffer[i]<=0x83){
	if(i){
	  return -i;
	}
	int len = 2;
	if(*buffer == 0x81 || *buffer == 0x83){
	  if(buffer_size<CAN_MSG_SIZE_MIN){
	    return 0;
	  }
	  len = (buffer[6] & 0xF) + CAN_MSG_SIZE_MIN;
	  if(*buffer == 0x83){
	    len += 8;
	  }
	}
	if(buffer_size < len){
	  return 0;
	}
	return len;
      }
    }
    return -buffer_size;
}

int Driver2Web::getPendingMessagesCount()
{
  return 1;
}

bool Driver2Web::checkBusOk() 
{
  return true;
}

void Driver2Web::clear()
{
    while(hasPacket())
    {
	read();
    }
}

int Driver2Web::getFileDescriptor() const 
{
    return iodrivers_base::Driver::getFileDescriptor();
}

bool Driver2Web::isValid() const
{
    return iodrivers_base::Driver::isValid();
}

/** Closes the file descriptor */
void Driver2Web::close()
{
    iodrivers_base::Driver::close();
}

