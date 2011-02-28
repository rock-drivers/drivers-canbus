
#include "canbus.hh"
#include "canbus-hico.hh"
#include "canbus-hico-pci.hh"

#ifdef HAVE_CAN_H
#include "canbus-socket.hh"
#endif

#include <stdio.h>

using namespace canbus;

Driver::~Driver() 
{
}

Driver *canbus::openCanDevice(std::string const& path, driverType dType)
{
   
  Driver *d;
  switch(dType)
   {
    case HICO: 
    		d = new DriverHico();
    		if (d->open(path)) {
			fprintf(stderr,"opened can bus with hico driver\n");
			return d;
    		}
    		else
      			fprintf(stderr,"no hico-can card found... trying to reach PCI card\n");
    		delete d;                
		break;

    case HICO_PCI:
		d = new DriverHicoPCI();
    		if (d->open(path)) {
			fprintf(stderr,"opened can bus with hico pci driver\n");
			return d;
    		}
    		else
      			fprintf(stderr,"no hico-can PCI card found... trying to reach socket can driver\n");
    
  		delete d;
		break;
    case SOCKET:
		//#ifdef HAVE_CAN_H
		d = new DriverSocket();
    		if (d->open(path)) {
			fprintf(stderr,"opened can bus with socket driver\n");
			return d;
    		}
    		else
        		fprintf(stderr,"no socket can driver found--> your PC does not have a working CANbus Interface!\n");
    		delete d;
		break;		
		//#endif
  default : return NULL; 
  }
  return NULL;
}
