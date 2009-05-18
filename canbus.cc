#include "canbus.hh"
#include "hico_api.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

using namespace can;

Driver::Driver()
    : IODriver(sizeof(can_msg))
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

bool Driver::reset()
{ return Driver::reset(m_fd); }
bool Driver::reset(int fd)
{
    SEND_IOCTL(IOC_RESET_BOARD);
    int bitrate = static_cast<int>(BITRATE_1000k);
    SEND_IOCTL_2(IOC_SET_BITRATE, &bitrate);
    SEND_IOCTL(IOC_STOP);
    SEND_IOCTL(IOC_START);
    return true;
}

void Driver::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }
uint32_t Driver::getReadTimeout() const
{ return m_read_timeout; }
void Driver::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }
uint32_t Driver::getWriteTimeout() const
{ return m_write_timeout; }

bool Driver::open(std::string const& path)
{
    if (isValid())
        close();

    int fd = ::open(path.c_str(), O_RDWR);
    if (fd == INVALID_FD)
        return false;

    file_guard guard(fd);
    if (!reset(fd))
        return false;

    setFileDescriptor(guard.release());
    return true;
}

Message Driver::read()
{
    can_msg msg;
    readPacket(reinterpret_cast<uint8_t*>(&msg), sizeof(can_msg), m_read_timeout);

    Message result;
    result.timestamp = DFKI::Time::now();
    result.can_id = msg.id;
    memcpy(result.data, msg.data, 8);
    result.size   = msg.dlc;
    return result;
}

void Driver::write(Message const& msg)
{
    can_msg out;
    memset(&out, 0, sizeof(can_msg));
    out.id = msg.can_id;
    memcpy(out.data, msg.data, 8);
    out.dlc   = msg.size;
    writePacket(reinterpret_cast<uint8_t*>(&out), sizeof(can_msg), m_write_timeout);
}

int Driver::extractPacket(uint8_t const* buffer, size_t buffer_size) const
{
    if (buffer_size == sizeof(can_msg))
        return sizeof(can_msg);
    else
        return -buffer_size;
}

