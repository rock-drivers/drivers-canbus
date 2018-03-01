#include <canbus/Driver.hpp>
#include <canbus/DriverHico.hpp>
#include <canbus/DriverHicoPCI.hpp>
#include <canbus/Driver2Web.hpp>
#include <canbus/DriverFTDI.hpp>
#include <canbus/DriverSocket.hpp>
#include <canbus/DriverNetGateway.hpp>
#include <base-logging/Logging.hpp>

#include <stdio.h>
#include <algorithm>
#include <string>
#include <memory>

using namespace canbus;

Driver::~Driver() 
{
}

Driver *canbus::openCanDevice(std::string const& path, DRIVER_TYPE dType)
{
    std::auto_ptr<Driver> driver;
    switch(dType)
    {
        case HICO: 
            driver.reset(new DriverHico());
            break;

        case HICO_PCI:
            driver.reset(new DriverHicoPCI());
            break;

        case SOCKET:
            driver.reset(new DriverSocket());
            break;          

        case CAN2WEB:
            driver.reset(new Driver2Web());
            break;          

        case NET_GATEWAY:
            driver.reset(new DriverNetGateway());
            break;

        case FTDI:
            driver.reset(new DriverFTDI());
            break;

        default:
            return NULL; 
    }

    if (driver->open(path)) {
        LOG_INFO("opened CAN device %s", path.c_str());
        return driver.release();
    }
    else
        LOG_WARN("failed to open CAN device %s", path.c_str());

    return NULL;
}

Driver *canbus::openCanDevice(std::string const& path, std::string const& type_upper)
{
    std::string type = type_upper;
    
    std::transform(type_upper.begin(), type_upper.end(), type.begin(), ::tolower);
    
    if(type == std::string("hico"))
    {
        return openCanDevice(path, HICO);
    }

    if(type == std::string("hico_pci"))
    {
        return openCanDevice(path, HICO_PCI);
    }

    if(type == std::string("socket"))
    {
        return openCanDevice(path, SOCKET);
    }

    if (type == std::string("net_gateway")) {
        return openCanDevice(path, NET_GATEWAY);
    }

    if (type == std::string("ftdi")) {
        return openCanDevice(path, FTDI);
    }

    return NULL;
}

Interface::Interface(){
}

Interface::~Interface(){
}
