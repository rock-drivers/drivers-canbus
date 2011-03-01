
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
      			fprintf(stderr,"no hico-can card found... could not open HicoCan device\n");
    		delete d;                
		break;

    case HICO_PCI:
		d = new DriverHicoPCI();
    		if (d->open(path)) {
			fprintf(stderr,"opened can bus with hico pci driver\n");
			return d;
    		}
    		else
      			fprintf(stderr,"no hico-can PCI card found... could not open HicoCan PCI device\n");
    
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
        		fprintf(stderr,"no socket can driver found... could not open socket CAN device!\n");
    		delete d;
		break;		
		//#endif
  default : return NULL; 
  }
  return NULL;
}
