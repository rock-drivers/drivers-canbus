#include <iostream>
#include <canbus/DriverEasySYNC.hpp>
#include <iomanip>
#include <map>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace canbus;

static std::string stateToString(DriverEasySYNC::BUS_STATE bus_state)
{
    switch(bus_state)
    {
        case DriverEasySYNC::OK:
            return "OK";
        case DriverEasySYNC::WARNING:
            return "WARNING";
        case DriverEasySYNC::PASSIVE:
            return "PASSIVE";
        case DriverEasySYNC::OFF:
            return "OFF";
        default:
            throw std::invalid_argument("invalid bus state");
    }
}

int main(int argc, char**argv)
{
    if (argc < 3)
    {
        cerr
            << "usage: canbus-easysync <uri> COMMAND\n"
            << "   canbus-easysync <uri> status\n"
            << "   canbus-easysync <uri> send id length byte0 [byte1]... [COUNT] [PERIOD_MS]\n"
            << endl;
        return 1;
    }

    string uri = argv[1];
    string command = argv[2];

    DriverEasySYNC* driver = new DriverEasySYNC();

    if (command == "status")
    {
        driver->open(uri);
        DriverEasySYNC::Status status = driver->getStatus();
        std::cout << "RX state: " << stateToString(status.rx_state) << "\n";
        std::cout << "TX state: " << stateToString(status.tx_state) << "\n";
        std::cout << "RX buffer 0 overflow: " << status.rx_buffer0_overflow << "\n";
        std::cout << "RX buffer 1 overflow: " << status.rx_buffer1_overflow << "\n";
    }
    else if (command == "send")
    {
        driver->open(uri);

        int id = strtol(argv[3], NULL, 16);
        if(id < 0 || id > (1<<11))
        {
            std::cerr << std::endl << "ID must between 0 and 2^11" << std::endl;
            return 1;
        }

        int length = strtol(argv[4], NULL, 0);
        std::cout << "id: 0x" << std::hex << id << " length: " << length << " data :";
        if(length < 0 || length > 8)
        {
            std::cerr << std::endl << "length must between 0 and 8" << std::endl;
            return 1;
        }
        
        int count = 1;
        int period = 0;
        if(argc >= length + 6)
        {
            count = strtol(argv[5 + length], NULL, 10);
            if (argc == length + 7) {
                period = strtol(argv[6 + length], NULL, 10);
            }
            else if (argc != length + 6) {
                std::cerr << std::endl << "Error, number of parameters does not match length" << std::endl;
                return 1;
            }
        }
        else if(argc != length + 5)
        {
            std::cerr << std::endl << "Error, number of parameters does not match length" << std::endl;
            return 1;
        }

        canbus::Message msg;
        msg.can_id = id;
        msg.size = length;
        int tmp;
        for(int i = 0; i < length; i++) 
        {
            tmp = strtol(argv[5 + i], NULL, 16);
            if(tmp < 0 || tmp > 255)
            {
                std::cerr << "Error, given value nr " << i << " is wrong : " << tmp << std::endl;
                return 1;
            }
            msg.data[i] = tmp;
            std::cout << " 0x" << tmp; 
        }
        std::cout << std::endl;
        std::cout << "sending " << std::dec << count << " packets at a period of " << period << "ms" << std::endl;

        for (int i = 0; i < count; ++i)
        {
            driver->write(msg);
        }
    }
    else if (command == "reset")
    {
        driver->openURI(uri);
        driver->writePacket(reinterpret_cast<uint8_t const*>("C\r"), 2);
        driver->resetBoard();
    }

    return 0;
}


