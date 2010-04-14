
#include "canbus.hh"
#include "canbus-hico.hh"
#include "canbus-socket.hh"

#include <stdio.h>

using namespace can;

Driver::~Driver() 
{
}

Driver *can::openCanDevice(std::string const& path)
{
    Driver *d;
    d = new DriverHico();
    if (d->open(path)) {
	fprintf(stderr,"opened can bus with hico driver\n");
	return d;
    }
    delete d;

    d = new DriverSocket();
    if (d->open(path)) {
	fprintf(stderr,"opened can bus with socket driver\n");
	return d;
    }
    delete d;
    return NULL;
}
