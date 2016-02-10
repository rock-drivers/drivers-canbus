#include "canbus-2web.hh"
#include "can2web_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

#define CANSTR_XMTFULL          "controller send buffer is full, can be ignored"
#define CANSTR_OVERRUN          "receive buffer overrun"
#define CANSTR_BUSERR           "error count at limit"
#define CANSTR_BUSOFF           "bus error, controller switched to bus-off"
#define CANSTR_RX_OVERFLOW      "receive buffer overflow"
#define CANSTR_TX_OVERFLOW      "transmit buffer overflow"


using namespace canbus;
using namespace std;
using namespace can2web;

//#define CAN_TIME

Driver2Web::Driver2Web()
    : iodrivers_base::Driver(CAN_MSG_SIZE_MIN + 16),
      m_read_timeout(DEFAULT_TIMEOUT),
      m_write_timeout(DEFAULT_TIMEOUT),
      m_baudrate(br125)
{
    m_status.error = 0;
}

bool Driver2Web::reset()
{
    char *sz = new char[128];
    sprintf(sz, "can_baudrate %i\r", (int) m_baudrate);
    write((uint8_t*) sz);
    usleep(1000);
    sprintf(sz, "can_extended 2\r");
    write((uint8_t*) sz);
    usleep(1000);
#ifdef CAN_TIME
    sprintf(sz,"can_time 1\r");
    write((uint8_t*)sz);
    usleep(1000);
#endif
    sprintf(sz, "can_fast 1\r");
    write((uint8_t*) sz);
    usleep(1000);
    sprintf(sz, "can_stream 1\r");
    write((uint8_t*) sz);
    return true;
}

bool Driver2Web::reset(int fd)
{
    return false;
}

void Driver2Web::setWriteTimeout(uint32_t timeout)
{
    m_write_timeout = timeout;
}

uint32_t Driver2Web::getWriteTimeout() const
{
    return m_write_timeout;
}

void Driver2Web::setReadTimeout(uint32_t timeout)
{
    m_read_timeout = timeout;
}

uint32_t Driver2Web::getReadTimeout() const
{
    return m_read_timeout;
}

void Driver2Web::setBaudrate(BAUD_RATE br)
{
    m_baudrate = br;
}

void Driver2Web::setBaudrate(int br)
{
    switch (br) {
    case 5:
        m_baudrate = br5;
        break;
    case 10:
        m_baudrate = br10;
        break;
    case 20:
        m_baudrate = br20;
        break;
    case 50:
        m_baudrate = br50;
        break;
    case 100:
        m_baudrate = br100;
        break;
    case 125:
        m_baudrate = br125;
        break;
    case 250:
        m_baudrate = br250;
        break;
    case 500:
        m_baudrate = br500;
        break;
    case 1000:
        m_baudrate = br1000;
        break;
    default:
        m_baudrate = br125;
    }
}

Status Driver2Web::getStatus() const
{
    return m_status;
}

bool Driver2Web::open(std::string const& path)
{
    string::size_type marker = path.find('@');
    string device = path;
    if (marker != string::npos) {
        int brate = boost::lexical_cast<int>(path.substr(marker + 1));
        setBaudrate(brate);
        device = device.substr(0, marker);
    }

    cout << "Driver2Web::open " << device << "  @" << m_baudrate << "kBd" << endl;
    if (isValid())
        close();
    openURI(device);
    return true;
}

bool Driver2Web::resetBoard()
{
    return true;
}

Message Driver2Web::read()
{
    vector<uint8_t> msg(CAN_MSG_SIZE_MIN + 16);
    msg[0] = 0;
    Message result;
    try {
        while (msg.at(0) < CAN_START || msg.at(0) > CAN_START_TIME) {
            readPacket(&msg[0], CAN_MSG_SIZE_MIN + 16, m_read_timeout);
        }
    } catch (iodrivers_base::TimeoutError& ) {

    }
    can_msg canMsg;
    canMsg << &(msg[0]);
    if (!canMsg.rtr_mode_len) {
        canMsg.rtr_mode_len = 8;
    }
    m_status.time = base::Time::now();
    m_status.error = canMsg.status;
    result.time = base::Time::now();
    result.can_id = 0;
    result.size = 0;
    if (msg.at(0) == CAN_START_STAT) {
        return result;
    }
    result.can_time = base::Time::fromMicroseconds(
            canMsg.secs * 1000000 + canMsg.u_secs);
    result.can_id = canMsg.can_id;
    memcpy(result.data, canMsg.data, 8);
    result.size = canMsg.rtr_mode_len & 0x0F;
    ;
    statusCheck(m_status);
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
    out >> s;
    int len = CAN_MSG_SIZE_MIN + (out.rtr_mode_len & 0xF);
    writePacket(s, len, m_write_timeout);
}

void Driver2Web::write(uint8_t *sz)
{
    writePacket(sz, strlen((const char*) sz), m_write_timeout);
}

int Driver2Web::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    for (size_t i = 0; i < buffer_size; i++) {
        if (buffer[i] >= 0x81 && buffer[i] <= 0x83) {
            if (i) {
                return -i;
            }
            size_t len = 2;
            if (*buffer == 0x81 || *buffer == 0x83) {
                if (buffer_size < CAN_MSG_SIZE_MIN) {
                    return 0;
                }
                len = (buffer[6] & 0xF) + CAN_MSG_SIZE_MIN;
                if (*buffer == 0x83) {
                    len += 8;
                }
            }
            if (buffer_size < len) {
                return 0;
            }
            return len;
        }
    }
    return -buffer_size;
}

int Driver2Web::getPendingMessagesCount()
{
    int bytes;
    if (getFileDescriptor() != INVALID_FD) {
        if (ioctl(getFileDescriptor(), FIONREAD, &bytes) == 0) {
            if (bytes >= CAN_MSG_SIZE_MIN)
                return 1;
        }
    }
    return 0;
}

bool Driver2Web::checkBusOk()
{
    return true;
}

void Driver2Web::clear()
{
    while (hasPacket()) {
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

void Driver2Web::statusCheck(const Status& status)
{
    bool bCanError = false;
    string errorString;
    if(status.error & CAN_ERR_XMTFULL){
        cout << CANSTR_XMTFULL << endl;
    }
    if(status.error & CAN_ERR_OVERRUN){
        bCanError = true;
        errorString = CANSTR_OVERRUN;
        cout << CANSTR_OVERRUN << endl;
    }
    if(status.error & CAN_ERR_BUSERR){
        bCanError = true;
        errorString = CANSTR_BUSERR;
        cout << CANSTR_BUSERR << endl;
    }
    if(status.error & CAN_ERR_BUSOFF){
        bCanError = true;
        errorString = CANSTR_BUSOFF;
        cout << CANSTR_BUSOFF << endl;
    }
    if(status.error & CAN_ERR_RX_OVERFLOW){
        bCanError = true;
        errorString = CANSTR_RX_OVERFLOW;
        cout << CANSTR_RX_OVERFLOW << endl;
    }
    if(status.error & CAN_ERR_TX_OVERFLOW){
        bCanError = true;
        errorString = CANSTR_TX_OVERFLOW;
        cout << CANSTR_TX_OVERFLOW << endl;
    }
    if (bCanError) {
        throw runtime_error(errorString);
    }
}
