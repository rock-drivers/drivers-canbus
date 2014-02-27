/*
 * canbus-netgw.cc
 *
 *  Created on: 26.02.2014
 *      Author: jrenken
 */

#include <string.h>
#include <sys/ioctl.h>
#include "canbus-netgw.hh"
#include <base/time.h>


#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

#define CAN_ERR_RESTARTED 0x00000100U;

using namespace canbus;

DriverNetGateway::DriverNetGateway()
    : iodrivers_base::Driver(sizeof(CanFrame))
    , mErrorCounter(0)
    , mError(false)
{
    m_read_timeout = base::Time::fromMilliseconds(DEFAULT_TIMEOUT);
    m_write_timeout = base::Time::fromMilliseconds(DEFAULT_TIMEOUT);
}

bool DriverNetGateway::open(std::string const& path)
{
    openURI(path);
    return true;
}

int DriverNetGateway::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    if (buffer_size >= sizeof(CanFrame))
        return sizeof(CanFrame);
    else
        return -buffer_size;
}

void DriverNetGateway::readOneMessage()
{
    CanFrame    frame;
    if (readPacket(reinterpret_cast<uint8_t*>(&frame), sizeof(CanFrame)) == sizeof(CanFrame)) {
        if (frame.can_id & CAN_ERR_FLAG) {
            if (frame.can_id & CAN_ERR_MASK) {
                mErrorCounter++;
                mError = true;
            } else {
                mError = false;
            }
        } else {
            Message msg;
            msg.time = base::Time::now();
            msg.can_time = msg.time;
            msg.can_id = frame.can_id;
            memcpy(msg.data, frame.data, 8);
            msg.size = frame.can_dlc;
            rx_queue.push_back(msg);
        }
    }
}

int DriverNetGateway::bufferMessages()
{
    int bytes;
    if (ioctl(getFileDescriptor(), FIONREAD, &bytes) == 0) {
        for (size_t i = 0; i < bytes / sizeof(CanFrame); i++) {
            readOneMessage();
        }
    }
    return rx_queue.size();
}

Message DriverNetGateway::read()
{
    while (rx_queue.empty()) {
        readOneMessage();
    }
    Message result = rx_queue.front();
    rx_queue.pop_front();
    return result;
}


bool DriverNetGateway::read(Message& msg)
{
    if (bufferMessages() == 0)
        return false;
    msg = read();
    return true;
}

void DriverNetGateway::write(const Message& msg)
{
    CanFrame frame;

    frame.can_id = msg.can_id;
    frame.can_dlc = msg.size;
    memcpy(frame.data, msg.data, 8);
    writePacket(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
}

int DriverNetGateway::getPendingMessagesCount()
{
    return bufferMessages();
}

bool DriverNetGateway::checkBusOk()
{
    bufferMessages();
    return !mError;
}

void DriverNetGateway::clear()
{
    bufferMessages();
    rx_queue.clear();
    mError = false;
}

int DriverNetGateway::getFileDescriptor() const
{
    return iodrivers_base::Driver::getFileDescriptor();
}

bool DriverNetGateway::isValid() const
{
    return iodrivers_base::Driver::isValid();
}

void DriverNetGateway::close()
{
    iodrivers_base::Driver::close();
}

uint32_t DriverNetGateway::getErrorCount() const
{
    return mErrorCounter;
}

void DriverNetGateway::setWriteTimeout(uint32_t timeout)
{
    iodrivers_base::Driver::setWriteTimeout(base::Time::fromMilliseconds(timeout));
}

uint32_t DriverNetGateway::getWriteTimeout() const
{
    return iodrivers_base::Driver::getWriteTimeout().toMilliseconds();
}

void DriverNetGateway::setReadTimeout(uint32_t timeout)
{
    iodrivers_base::Driver::setReadTimeout(base::Time::fromMilliseconds(timeout));
}

uint32_t DriverNetGateway::getReadTimeout() const
{
    return iodrivers_base::Driver::getReadTimeout().toMilliseconds();
}

bool DriverNetGateway::reset()
{
    CanFrame frame;
    frame.can_id = CAN_ERR_FLAG | CAN_ERR_RESTARTED;
    frame.can_dlc = 8;
    memset(frame.data, 0, 8);
    writePacket(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
    return true;
}
