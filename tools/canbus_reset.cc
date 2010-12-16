#include "canbus.hh"
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "canbus_reset can_device" << std::endl;
        return 1;
    }
    std::auto_ptr<canbus::Driver> driver(canbus::openCanDevice(argv[1]));
    driver->reset_board();
}
