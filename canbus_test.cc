#include <iostream>
#include "canbus.hh"
#include <iomanip>
#include <map>
#include <boost/lexical_cast.hpp>

using namespace std;

int main(int argc, char**argv)
{
    if (argc != 2 && argc != 3) 
    {
        cerr << "usage: canbus_test <device> [count]" << endl;
        return 1;
    }

    std::map<int, int> statistics;
    can::Driver driver;
    if (!driver.open(argv[1]))
        return 1;
    if (!driver.reset())
        return 1;

    size_t count = 1000;
    if (argc == 3)
        count = boost::lexical_cast<size_t>(argv[2]);
    cerr << "will listen to " << count << " messages" << endl;

    for (size_t i = 0; i < count; ++i)
    {
        can::Message msg = driver.read();
        statistics[msg.can_id]++;
    }

    for (map<int, int>::const_iterator it =
            statistics.begin(); it != statistics.end(); ++it)
    {
        cerr << hex << it->first << " " << dec << it->second << endl;
    }
    return 0;
}

