#include <canbus/Driver.hpp>
#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "canbus_reset can_device device_type" << std::endl;
        return 1;
    }
    std::unique_ptr<canbus::Driver> driver(
            canbus::openCanDevice(std::string(argv[1]), std::string(argv[2])));
    
    if(!driver.get())
    {
        std::cerr << "Failed to open can device of type " << argv[2] << " with path " << argv[1] << std::endl;
        return 1;
    }
    driver->reset_board();
}
