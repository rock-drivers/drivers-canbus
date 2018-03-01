#include <iostream>
#include <canbus/Driver.hpp>
#include <iomanip>
#include <map>
#include <boost/lexical_cast.hpp>


int main(int argc, char** argv)
{
    if (argc < 5 || argc > 13) 
    {
        std::cerr
            << "usage: canbus-send device device_type id length [value1] ... [value8] [COUNT] [PERIOD_IN_MS]\n"
            << std::endl;
        return 1;
    }

    std::map<int, int> statistics;
    canbus::Driver * driver = canbus::openCanDevice(argv[1], argv[2]);
    if (!driver)
    {
        std::cerr << "failed to open the CAN device" << std::endl;
        return 1;
    }
    if (!driver->reset())
    {
        std::cerr << "failed to reset the CAN device" << std::endl;
        return 1;
    }

    int id;
    int lenght;
    id = strtol(argv[3], NULL, 16);
    if(id < 0 || id > (1<<11))
    {
        std::cerr << std::endl << "ID must between 0 and 2^11" << std::endl;
        return 1;
    }


    lenght = strtol(argv[4], NULL, 0);

    std::cout << "id: 0x" << std::hex << id << " lenght: " << lenght << " data :";

    if(lenght < 0 || lenght > 8)
    {
        std::cerr << std::endl << "Lenght must between 0 and 8" << std::endl;
        return 1;
    }
    
    int count = 1;
    int period = 0;
    if(argc >= lenght + 6)
    {
        count = strtol(argv[5 + lenght], NULL, 10);
        if (argc == lenght + 7) {
            period = strtol(argv[6 + lenght], NULL, 10);
        }
        else if (argc != lenght + 6) {
            std::cerr << std::endl << "Error, number of parameters does not match length" << std::endl;
            return 1;
        }
    }
    else if(argc != lenght + 5)
    {
        std::cerr << std::endl << "Error, number of parameters does not match length" << std::endl;
        return 1;
    }

    canbus::Message msg;
    msg.can_id = id;
    msg.size = lenght;
    int tmp;
    for(int i = 0; i < lenght; i++) 
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
        if (period)
            usleep(period * 1000);
    }

    return 0;
    
}
