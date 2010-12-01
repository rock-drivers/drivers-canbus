
#include "canbus.hh"
#include "canbus-hico.hh"
#ifdef HAVE_CAN_H
#include "canbus-socket.hh"
#endif

#include <stdio.h>

using namespace canbus;

Driver::~Driver() 
{
}

Driver *canbus::openCanDevice(std::string const& path)
{
    Driver *d;
    d = new DriverHico();
    if (d->open(path)) {
	fprintf(stderr,"opened can bus with hico driver\n");
	return d;
    }
    delete d;

#ifdef HAVE_CAN_H
    d = new DriverSocket();
    if (d->open(path)) {
	fprintf(stderr,"opened can bus with socket driver\n");
	return d;
    }
    delete d;
#endif
    return NULL;
}
