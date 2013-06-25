
#include "canbus.hh"
#include "canbus-hico.hh"
#include "canbus-hico-pci.hh"

#ifdef HAVE_CAN_H
#include "canbus-socket.hh"
#endif

#include <stdio.h>
#include <algorithm>
#include <string>

using namespace canbus;

Driver::~Driver() 
{
}

Driver *canbus::openCanDevice(std::string const& path, DRIVER_TYPE dType)
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

    return NULL;
}

Interface::Interface(){
}

Interface::~Interface(){
}
