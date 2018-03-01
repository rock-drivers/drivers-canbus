#include <string>
#include <canbus/DriverFTDI.hpp>
#include <base/Time.hpp>

using namespace canbus;
using namespace std;

struct BAUD_RATE_ARGUMENTS { string asString; char const* cmd; };
BAUD_RATE_ARGUMENTS baud_rates[] = {
    { "10k", "S0\n" },
    { "20k", "S1\n" },
    { "50k", "S2\n" },
    { "100k", "S3\n" },
    { "125k", "S4\n" },
    { "250k", "S5\n" },
    { "500k", "S6\n" },
    { "800k", "S7\n" },
    { "1M", "S8\n" },
    { "", 0 }
};

DriverFTDI::DriverFTDI()
    : iodrivers_base::Driver(MAX_PACKET_SIZE) {}

bool DriverFTDI::open(string const& path)
{
    std::string uri;
    unsigned int colon = path.find_last_of(":");
    char const* rate_cmd = 0;
    if (colon != string::npos)
    {
        string potential_rate(path, colon + 1, path.size() - colon - 1);
        for (int i = 0; baud_rates[i].cmd; ++i)
        {
            if (baud_rates[i].asString == potential_rate) {
                rate_cmd = baud_rates[i].cmd;
            }
        }
    }
    if (rate_cmd)
        uri = string(path, 0, colon);
    else
        uri = path;

    openURI(uri);
    processSimpleCommand("Z1\n", 3);
    if (rate_cmd)
        processSimpleCommand(rate_cmd, 3);
    processSimpleCommand("O\n", 2);
    return true;
}

void DriverFTDI::setWriteTimeout(uint32_t timeout)
{
    iodrivers_base::Driver::setWriteTimeout(base::Time::fromMilliseconds(timeout));
}

uint32_t DriverFTDI::getWriteTimeout() const
{
    return iodrivers_base::Driver::getWriteTimeout().toMilliseconds();
}

void DriverFTDI::setReadTimeout(uint32_t timeout)
{
    iodrivers_base::Driver::setReadTimeout(base::Time::fromMilliseconds(timeout));
}

uint32_t DriverFTDI::getReadTimeout() const
{
    return iodrivers_base::Driver::getReadTimeout().toMilliseconds();
}

bool DriverFTDI::resetBoard()
{
    processSimpleCommand("R\n", 2);
    return true;
}

bool DriverFTDI::reset()
{
    return true;
}

static int nibbleToInt(char c)
{
    if (c <= '9')
        return c - '0';
    else
        return 0xA + (c - 'A');
}

uint8_t const* parseBytes(uint8_t* output, uint8_t const* input, int byte_size)
{
    for (int i = 0; i < byte_size; ++i)
    {
        int msn = nibbleToInt(input[i * 2]);
        int lsn = nibbleToInt(input[i * 2 + 1]);
        output[i] = msn << 4 | lsn;
    }
    return input + byte_size * 2;
}

Message DriverFTDI::read()
{
    uint8_t buffer[MAX_PACKET_SIZE];
    int size = readPacket(buffer, MAX_PACKET_SIZE);
    canbus::Message message;
    message.time = base::Time::now();

    uint8_t const* cursor;

    uint8_t raw_can_id[4] = { 0, 0, 0, 0 };
    if (buffer[0] == 'T')
        cursor = parseBytes(raw_can_id, buffer + 1, 4);
    else
    {
        buffer[0] = '0'; // round at the nibble ...
        cursor = parseBytes(raw_can_id + 2, buffer, 2);
    }
    message.can_id = 
        static_cast<int>(raw_can_id[0]) << 24 |
        static_cast<int>(raw_can_id[1]) << 16 |
        static_cast<int>(raw_can_id[2]) << 8 |
        static_cast<int>(raw_can_id[3]) << 0;

    int length = *cursor - '0';
    message.size = length;

    cursor = parseBytes(message.data, cursor + 1, length);
    uint8_t raw_can_time[2] = { 0, 0 };
    cursor = parseBytes(raw_can_time, cursor, 2);

    if (cursor != buffer + size)
        throw std::runtime_error("size mismatch while parsing a received frame");

    uint32_t can_time = 
        static_cast<int>(raw_can_time[0]) << 8 |
        static_cast<int>(raw_can_time[1]) << 0;
    message.can_time = base::Time::fromMilliseconds(can_time);
    return message;
}

static char intToNibble(int v)
{
    if (v < 0xA)
        return v + '0';
    else
        return (v - 0xA) + 'A';
}

uint8_t* dumpBytes(uint8_t* output, uint8_t const* input, int byte_size)
{
    for (int i = 0; i < byte_size; ++i)
    {
        output[i * 2]     = intToNibble((input[i] >> 4) & 0xF);
        output[i * 2 + 1] = intToNibble(input[i] & 0xF);
    }
    return output + byte_size * 2;
}

void DriverFTDI::write(Message const& msg)
{
    uint8_t raw_can_id[4];
    raw_can_id[0] = (msg.can_id >> 24) & 0xFF;
    raw_can_id[1] = (msg.can_id >> 16) & 0xFF;
    raw_can_id[2] = (msg.can_id >>  8) & 0xFF;
    raw_can_id[3] = (msg.can_id >>  0) & 0xFF;
    uint8_t buffer[MAX_PACKET_SIZE];
    uint8_t* cursor;
    if ((msg.can_id & ~0x7FF) != 0)
    {
        buffer[0] = 'T';
        cursor = dumpBytes(buffer + 1, raw_can_id, 4);
    }
    else
    {
        cursor = dumpBytes(buffer, raw_can_id + 2, 2);
        buffer[0] = 't';
    }

    *cursor = msg.size + '0';
    cursor = dumpBytes(cursor + 1, msg.data, msg.size);
    *cursor = '\n';
    processSimpleCommand(buffer, cursor - buffer + 1);
}

int DriverFTDI::getFileDescriptor() const
{
    return iodrivers_base::Driver::getFileDescriptor();
}

bool DriverFTDI::isValid() const
{
    return iodrivers_base::Driver::isValid();
}

void DriverFTDI::close()
{
    processSimpleCommand("C\n", 2);
    iodrivers_base::Driver::close();
}

DriverFTDI::Status DriverFTDI::getStatus()
{
    uint8_t buffer[MAX_PACKET_SIZE];
    processCommand("F\n", 2, buffer);

    uint8_t raw_status;
    parseBytes(&raw_status, buffer, 2);
    Status status;
    status.time = base::Time::now();
    if (raw_status & 0x20) {
        status.tx_state = OFF;
    }
    else if (raw_status & 0x10) {
        status.tx_state = PASSIVE;
    }
    else if (raw_status & 0x04) {
        status.tx_state = WARNING;
    }
    else {
        status.tx_state = OK;
    }

    if (raw_status & 0x08) {
        status.rx_state = PASSIVE;
    }
    else if (raw_status & 0x02) {
        status.rx_state = WARNING;
    }
    else {
        status.rx_state = OK;
    }

    status.rx_buffer0_overflow = raw_status & 0x40;
    status.rx_buffer1_overflow = raw_status & 0x80;
    return status;
}

bool DriverFTDI::checkBusOk()
{
    Status status = getStatus();
    return status.rx_state == OK &&
        status.tx_state == OK;
}

void DriverFTDI::clear()
{
    processSimpleCommand("E\n", 2);
    iodrivers_base::Driver::clear();
}

int DriverFTDI::processCommand(char const* cmd, int commandSize, uint8_t* buffer)
{
    return processCommand(reinterpret_cast<uint8_t const*>(cmd), commandSize, buffer);
}

int DriverFTDI::processCommand(uint8_t const* cmd, int commandSize, uint8_t* buffer)
{
    mCurrentCommand = cmd[0];
    writePacket(cmd, commandSize);

    int size = readPacket(buffer, MAX_PACKET_SIZE);
    if (buffer[0] == '\x7')
        throw FailedCommand(string(reinterpret_cast<char const*>(cmd), commandSize) + " command failed");
    return size;
}

void DriverFTDI::processSimpleCommand(char const* cmd, int commandSize)
{
    return processSimpleCommand(reinterpret_cast<uint8_t const*>(cmd), commandSize);
}

void DriverFTDI::processSimpleCommand(uint8_t const* cmd, int commandSize)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    processCommand(cmd, commandSize, buffer);
}

static bool isNibble(char c)
{
    return (c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'F');
}

static int checkNibbleSequence(uint8_t const* buffer, int bufferSize, int offset, int expectedSize)
{
    int size = std::min(offset + expectedSize, bufferSize);
    for (int i = offset; i < size; ++i)
        if (!isNibble(buffer[i]))
            return -i;
    return 0;
}

int DriverFTDI::extractPacket(uint8_t const* buffer, size_t bufferSize) const
{
    if (bufferSize == 0)
        return 0;
    else if (buffer[0] == '\x7')
        return 1;
    else if (buffer[0] == 't')
    {
        // Reception of a standard frame
        int r = checkNibbleSequence(buffer, bufferSize, 1, 4);
        if (r)
            return r;
        if (bufferSize < 5)
            return 0;
    
        // remaining N bytes per packet and 4 for timestamp
        int remainingLength = (buffer[4] - '0') * 2 + 4;
        size_t expectedLength = remainingLength + 5;
        r = checkNibbleSequence(buffer, bufferSize, 5, remainingLength);
        if (r)
            return r;
        else if (bufferSize < expectedLength)
            return 0;
        else
            return expectedLength;
    }

    switch(mCurrentCommand)
    {
        case 'S':
        case 's':
        case 'm':
        case 'M':
        case 'Z':
        case 'O':
        case 'L':
        case 'C':
        case 'R':
        {
            if (buffer[0] == '\n')
                return 1;
        }

        case 'F': // replies with XX\n
        {
            int r = checkNibbleSequence(
                buffer, bufferSize, 0, 2);
            if (r) return r;
            if (bufferSize < 3)
                return 0;
            return (buffer[2] == '\n') ? 3 : -3;
        }

        case 'V': // replies with Vxxyy\n
        case 'N': // replies with Nxxyy\n
        {
            if (buffer[0] != mCurrentCommand)
                return -1;

            int r = checkNibbleSequence(
                buffer, bufferSize, 1, 4);
            if (r) return r;
            if (bufferSize < 6)
                return 0;
            return (buffer[5] == '\n') ? 3 : -3;
        }
        case 'E': // replies with 'E\n'
        {
            if (buffer[0] != 'E')
                return -1;
            else if (bufferSize < 2)
                return 0;
            else if (buffer[1] == '\n')
                return 2;
            else return -2;
        }
        case 't':
            if (bufferSize >= 1 && buffer[0] == '\x7')
                return 1;
            else if (bufferSize < 2)
                return 0;
            else if (buffer[0] != 'z')
                return -1;
            else if (buffer[1] != '\n')
                return -2;
            else
                return 2;
        case 'T':
            if (bufferSize >= 1 && buffer[0] == '\x7')
                return 1;
            else if (bufferSize < 2)
                return 0;
            else if (buffer[0] != 'Z')
                return -1;
            else if (buffer[1] != '\n')
                return -2;
            else
                return 2;
    }
    throw std::runtime_error("intenral error: unknown command in extractPacket");
}
