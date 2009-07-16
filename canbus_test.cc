#include <iostream>
#include "canbus.hh"
#include <iomanip>
#include <map>
#include <boost/lexical_cast.hpp>

using namespace std;

int main(int argc, char**argv)
{
    if (argc < 2 || argc > 5) 
    {
        cerr
            << "usage: canbus_test <device> [count] [id] [mask]\n"
            << "  count is the count of messages to listen to, or the nolimit keyword\n"
            << "  the id/mask combination filters the CAN IDs to the ones that match can_id & mask == id\n"
            << endl;
        return 1;
    }

    std::map<int, int> statistics;
    can::Driver driver;
    if (!driver.open(argv[1]))
        return 1;
    if (!driver.reset())
        return 1;

    int64_t count = -1;
    if (argc >= 3)
    {
        string count_s = argv[2];
        if (count_s == "nolimit")
            count = -1;
        else
            count = boost::lexical_cast<size_t>(argv[2]);
    }

    int id   = 0;
    int mask = 0;
    if (argc >= 4)
    {
        id = strtol(argv[3], NULL, 0);
        mask = 0x7FF;
    }
    if (argc >= 5)
        mask = strtol(argv[4], NULL, 0);

    cerr << "id: " << hex << id << " mask: " << hex << mask << endl;

    int i = 0;
    while(count == -1 || i < count)
    {
        can::Message msg = driver.read();
        if ((msg.can_id & mask) != id)
            continue;

        ++i;

        cout << setw(10) << i << " " << setw(5) << msg.can_id;
        for (int i = 0; i < msg.size; ++i)
            cout << " " << setw(3) << (int)msg.data[i];
        cout << endl;
        statistics[msg.can_id]++;
    }

    cerr << "message statistics:\n"
        << "ID count\n";
    for (map<int, int>::const_iterator it =
            statistics.begin(); it != statistics.end(); ++it)
    {
        cerr << hex << it->first << " " << dec << it->second << endl;
    }
    return 0;
}

