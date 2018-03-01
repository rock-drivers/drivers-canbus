#include <iostream>
#include <canbus/DriverFTDI.hpp>
#include <iomanip>
#include <map>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace canbus;

static std::string stateToString(DriverFTDI::BUS_STATE bus_state)
{
    switch(bus_state)
    {
        case DriverFTDI::OK:
            return "OK";
        case DriverFTDI::WARNING:
            return "WARNING";
        case DriverFTDI::PASSIVE:
            return "PASSIVE";
        case DriverFTDI::OFF:
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
            << "usage: canbus-ftdi <uri> COMMAND\n"
            << endl;
        return 1;
    }

    string uri = argv[1];
    string command = argv[2];

    DriverFTDI* driver = new DriverFTDI();
    driver->open(uri);

    if (command == "status")
    {
        DriverFTDI::Status status = driver->getStatus();
        std::cout << "RX state: " << stateToString(status.rx_state) << "\n";
        std::cout << "TX state: " << stateToString(status.tx_state) << "\n";
        std::cout << "RX buffer 0 overflow: " << status.rx_buffer0_overflow << "\n";
        std::cout << "RX buffer 1 overflow: " << status.rx_buffer1_overflow << "\n";
    }
    return 0;
}


