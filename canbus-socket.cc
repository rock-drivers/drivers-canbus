#include "canbus-socket.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>

//#include <iodrivers_base.hh>

using namespace canbus;
using iodrivers_base::UnixError;
using iodrivers_base::TimeoutError;
using iodrivers_base::Timeout;

DriverSocket::DriverSocket()
    : m_read_timeout(DEFAULT_TIMEOUT)
    , m_write_timeout(DEFAULT_TIMEOUT)
    , m_fd(-1)
    , m_error(false)
    , err_counter(0){}

bool DriverSocket::reset()
{
    err_counter=0;
    return DriverSocket::reset(m_fd); 
}
bool DriverSocket::reset(int fd)
{
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &on,
		   sizeof(on)) != 0) {
	perror("setsockopt");
	return false;
    }

    //CAN_ERR_CTRL is not requested because those are mostly long-term
    //bus quality flags
    can_err_mask_t err_mask = ( CAN_ERR_TX_TIMEOUT |
				CAN_ERR_LOSTARB |
				//CAN_ERR_CRTL |
				CAN_ERR_PROT |
				CAN_ERR_TRX |
				CAN_ERR_ACK |
				CAN_ERR_BUSOFF |
				CAN_ERR_BUSERROR |
				CAN_ERR_RESTARTED
	);
    //the peak can adapter seems to not report many errors, even when there
    //is something wrong with the bus, and there are no acks..

    if (setsockopt(fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
		   &err_mask, sizeof(err_mask)) != 0) {
	perror("setsockopt");
	return false;
    }
    return true;
}

void DriverSocket::setReadTimeout(uint32_t timeout)
{ m_read_timeout = timeout; }
uint32_t DriverSocket::getReadTimeout() const
{ return m_read_timeout; }
void DriverSocket::setWriteTimeout(uint32_t timeout)
{ m_write_timeout = timeout; }
uint32_t DriverSocket::getWriteTimeout() const
{ return m_write_timeout; }

bool DriverSocket::open(std::string const& path)
{
    this->path = path;
    if (isValid())
        close();

    int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd == -1)
      return false;

    iodrivers_base::FileGuard guard(fd);

    long fd_flags = fcntl(fd, F_GETFL);
    if (fd_flags == -1)
      return false;
    if (fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK) == -1)
      return false;

    struct sockaddr_can addr;
    struct ifreq ifr;
    
    strcpy(ifr.ifr_name, path.c_str() );
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
      return false;
    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	    return false;

    if (!reset(fd))
      return false;

    m_fd = guard.release();
    return true;
}

static void printErrorFrame(struct can_frame const & frame, std::string const& path) 
{
    printf("read(%s): CAN error frame\n",path.c_str());
    if (frame.can_id & CAN_ERR_TX_TIMEOUT)
	printf("\tTX timeout\n");
    if (frame.can_id & CAN_ERR_LOSTARB)
	printf("\tlost arbitration at bit %d\n",frame.data[0]);
    if (frame.can_id & CAN_ERR_CRTL) {
	printf("\tcontroller problem:\n");
	if (frame.data[1] & CAN_ERR_CRTL_UNSPEC)
	    printf("\t\tunspecified\n");
	if (frame.data[1] & CAN_ERR_CRTL_RX_OVERFLOW)
	    printf("\t\tRX overflow\n");
	if (frame.data[1] & CAN_ERR_CRTL_TX_OVERFLOW)
	    printf("\t\tTX overflow\n");
	if (frame.data[1] & CAN_ERR_CRTL_RX_WARNING)
	    printf("\t\tRX warning\n");
	if (frame.data[1] & CAN_ERR_CRTL_TX_WARNING)
	    printf("\t\tTX warning\n");
	if (frame.data[1] & CAN_ERR_CRTL_RX_PASSIVE)
	    printf("\t\tRX passive\n");
	if (frame.data[1] & CAN_ERR_CRTL_TX_PASSIVE)
	    printf("\t\tTX passive\n");
    }
    if (frame.can_id & CAN_ERR_PROT) {
	printf("\tprotocol violation:\n");
	if (frame.data[2] & CAN_ERR_PROT_UNSPEC)
	    printf("\t\tunspecified\n");
	if (frame.data[2] & CAN_ERR_PROT_BIT)
	    printf("\t\tsingle bit error\n");
	if (frame.data[2] & CAN_ERR_PROT_FORM)
	    printf("\t\tframe format error\n");
	if (frame.data[2] & CAN_ERR_PROT_STUFF)
	    printf("\t\tbit stuffing error\n");
	if (frame.data[2] & CAN_ERR_PROT_BIT0)
	    printf("\t\tunable to send dominant bit\n");
	if (frame.data[2] & CAN_ERR_PROT_BIT1)
	    printf("\t\tunable to send recessive bit\n");
	if (frame.data[2] & CAN_ERR_PROT_OVERLOAD)
	    printf("\t\tbus overload\n");
	if (frame.data[2] & CAN_ERR_PROT_ACTIVE)
	    printf("\t\tactive error announcement\n");
	if (frame.data[2] & CAN_ERR_PROT_TX)
	    printf("\t\terror on transmission\n");
	switch(frame.data[3]) {
	case CAN_ERR_PROT_LOC_UNSPEC:
	    printf("\t\tlocation: unspecified\n"); break;
	case CAN_ERR_PROT_LOC_SOF:
	    printf("\t\tlocation: start of frame\n"); break;
	case CAN_ERR_PROT_LOC_ID28_21:
	    printf("\t\tlocation: id bits 28-21(eff)/10-3(sff)\n"); break;
	case CAN_ERR_PROT_LOC_ID20_18:
	    printf("\t\tlocation: id bits 20-18(eff)/2-0(sff)\n"); break;
	case CAN_ERR_PROT_LOC_SRTR:
	    printf("\t\tlocation: substitute rtr\n"); break;
	case CAN_ERR_PROT_LOC_IDE:
	    printf("\t\tlocation: identifier extension\n"); break;
	case CAN_ERR_PROT_LOC_ID17_13:
	    printf("\t\tlocation: id bits 17-13(eff)\n"); break;
	case CAN_ERR_PROT_LOC_ID12_05:
	    printf("\t\tlocation: id bits 12-5(eff)\n"); break;
	case CAN_ERR_PROT_LOC_ID04_00:
	    printf("\t\tlocation: id bits 4-0(eff)\n"); break;
	case CAN_ERR_PROT_LOC_RTR:
	    printf("\t\tlocation: RTR\n"); break;
	case CAN_ERR_PROT_LOC_RES1:
	    printf("\t\tlocation: reserved bit 1\n"); break;
	case CAN_ERR_PROT_LOC_RES0:
	    printf("\t\tlocation: reserved bit 0\n"); break;
	case CAN_ERR_PROT_LOC_DLC:
	    printf("\t\tlocation: data length code\n"); break;
	case CAN_ERR_PROT_LOC_DATA:
	    printf("\t\tlocation: data section\n"); break;
	case CAN_ERR_PROT_LOC_CRC_SEQ:
	    printf("\t\tlocation: crc sequence\n"); break;
	case CAN_ERR_PROT_LOC_CRC_DEL:
	    printf("\t\tlocation: crc delimiter\n"); break;
	case CAN_ERR_PROT_LOC_ACK:
	    printf("\t\tlocation: ack slot\n"); break;
	case CAN_ERR_PROT_LOC_ACK_DEL:
	    printf("\t\tlocation: ack delimiter\n"); break;
	case CAN_ERR_PROT_LOC_EOF:
	    printf("\t\tlocation: end of frame\n"); break;
	case CAN_ERR_PROT_LOC_INTERM:
	    printf("\t\tlocation: intermission\n"); break;
	default:
	    printf("\t\tlocation: unknown\n"); break;
	}
    }
    if (frame.can_id & CAN_ERR_TRX) {
	printf("\ttranscevier status:\n");
	switch(frame.data[4] & 0x0f) {
	case CAN_ERR_TRX_UNSPEC:
	    printf("\t\tcanh: unspecified\n"); break;
	case CAN_ERR_TRX_CANH_NO_WIRE:
	    printf("\t\tcanh: no wire\n"); break;
	case CAN_ERR_TRX_CANH_SHORT_TO_BAT:
	    printf("\t\tcanh: short to bat\n"); break;
	case CAN_ERR_TRX_CANH_SHORT_TO_VCC:
	    printf("\t\tcanh: short to vcc\n"); break;
	case CAN_ERR_TRX_CANH_SHORT_TO_GND:
	    printf("\t\tcanh: short to gnd\n"); break;
	default:
	    printf("\t\tcanh: unknown\n"); break;
	}
	switch(frame.data[4] & 0xf0) {
	case CAN_ERR_TRX_UNSPEC:
	    printf("\t\tcanl: unspecified\n"); break;
	case CAN_ERR_TRX_CANL_NO_WIRE:
	    printf("\t\tcanl: no wire\n"); break;
	case CAN_ERR_TRX_CANL_SHORT_TO_BAT:
	    printf("\t\tcanl: short to bat\n"); break;
	case CAN_ERR_TRX_CANL_SHORT_TO_VCC:
	    printf("\t\tcanl: short to vcc\n"); break;
	case CAN_ERR_TRX_CANL_SHORT_TO_GND:
	    printf("\t\tcanl: short to gnd\n"); break;
	case CAN_ERR_TRX_CANL_SHORT_TO_CANH:
	    printf("\t\tcanl: short to canh\n"); break;
	default:
	    printf("\t\tcanl: unknown\n"); break;
	}	    
    }
    if (frame.can_id & CAN_ERR_ACK)
	printf("\treceived no ACK\n");
    if (frame.can_id & CAN_ERR_BUSOFF)
	printf("\tbus off\n");
    if (frame.can_id & CAN_ERR_BUSERROR)
	printf("\tbus error\n");
    if (frame.can_id & CAN_ERR_RESTARTED)
	printf("\tcontroller restarted\n");
}

bool DriverSocket::checkInput(Timeout timeout)
{
    struct can_frame frame;
    struct timeval tv = {0};
    bool haveTimestamp = false;

    while(true) {
  
	int res;
	char control[1024];
	struct iovec    iov;
	struct msghdr msgh = {0};
	struct cmsghdr *cmsg;
	iov.iov_base = &frame;
	iov.iov_len = sizeof(frame);
	msgh.msg_control = control;
	msgh.msg_controllen = sizeof(control);
	msgh.msg_iovlen = 1;
	msgh.msg_iov = &iov;

	res = recvmsg(m_fd, &msgh, 0);
	if (res == -1 && errno != EAGAIN) 
            throw iodrivers_base::UnixError("read(): error in recvmsg()");
	if (res >= 0) {
	    /* Receive auxiliary data in msgh */
	    for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
		 cmsg = CMSG_NXTHDR(&msgh,cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET
		    && cmsg->cmsg_type == SO_TIMESTAMP) {
		    tv = *(struct timeval*)(void*)CMSG_DATA(cmsg);
		    haveTimestamp = true;
		    break;
		}
	    }
	    if (cmsg == NULL) {
		/*
		 * Error: no timestamp in control buffer
		 */
	    }

	    if (iov.iov_len != 0)
		break;
	}

        if (timeout.elapsed())
	    return false;

	struct pollfd pfd;
	pfd.fd = m_fd;
	pfd.events = POLLIN;
	res = poll(&pfd,1,timeout.timeLeft());
	if (res == -1)
            throw UnixError("read(): error in poll()");
        else if (res == 0)
	    return false;
    }

    if (frame.can_id & CAN_ERR_FLAG) {
        //Do not handle LOSTARB, this should not be critical
        //Lostarb ist more or less an collision on the bus
        //An resend should be done by the kernel -- hopefully
        if(frame.can_id & ~(CAN_ERR_LOSTARB | CAN_ERR_FLAG))
            m_error = true;

        err_counter++;
	printErrorFrame(frame, path);
	return true;
    }
    if (frame.can_id & CAN_RTR_FLAG) {
	printf("read: CAN RTR frame\n");
	return true;
    }

    /* do something with the received CAN frame */
    Message result;
    if (haveTimestamp)
	result.time     = base::Time::fromSeconds(tv.tv_sec,tv.tv_usec);
    else
	throw "timestamp missing";
//	result.time     = base::Time::now();
    result.can_time = result.time;
    result.can_id        = frame.can_id & CAN_ERR_MASK;
    memcpy(result.data, frame.data, 8);
    result.size          = frame.can_dlc;
    
    rx_queue.push_back(result);
    return true;
}

Message DriverSocket::read()
{
    Timeout timeout(m_read_timeout);
    while (rx_queue.empty()) {
	if (!checkInput(timeout))
            throw iodrivers_base::TimeoutError(iodrivers_base::TimeoutError::PACKET, "read(): timeout");
    }
    
    Message result = rx_queue.front();
    rx_queue.pop_front();
    return result;
}

bool DriverSocket::read(Message &result) {
    Timeout timeout(m_read_timeout);
    while (rx_queue.empty()) {
	if (!checkInput(timeout))
	    return false;
    }
    
    result = rx_queue.front();
    rx_queue.pop_front();
    return true;
}

void DriverSocket::write(Message const& msg)
{
    struct can_frame frame;
    
    memset(&frame, 0, sizeof(can_frame));
    frame.can_id = msg.can_id;
    frame.can_dlc = msg.size;
    memcpy(frame.data,msg.data,8);

    Timeout timeout(m_write_timeout);
    while(true) {
	int c = send(m_fd,reinterpret_cast<char*>(&frame), sizeof(can_frame), 0);
        if (c == -1 && errno != EAGAIN && errno != ENOBUFS)
            throw UnixError("write(): error during write");
	if (c > 0)
	    return;
	
        if (timeout.elapsed())
            throw TimeoutError(TimeoutError::PACKET, "write(): timeout");

	struct pollfd pfd;
	pfd.fd = m_fd;
	pfd.events = POLLOUT;
	int res = poll(&pfd,1,timeout.timeLeft());
	if (res == -1)
            throw UnixError("writePacket(): error in poll()");
        else if (res == 0)
            throw TimeoutError(TimeoutError::PACKET, "write(): timeout");
    }
}

int DriverSocket::getPendingMessagesCount()
{
    Timeout t(0);
    while(checkInput(t)) {}
    return rx_queue.size();
}

bool DriverSocket::checkBusOk()
{
    Timeout t(0);
    while(checkInput(t)) {}
    return !m_error;
}

void DriverSocket::clear()
{
    Timeout t(0);
    while(checkInput(t)) {}
    rx_queue.clear();
    m_error = false;
    return;
}

int DriverSocket::getFileDescriptor() const
{
    return m_fd;
}

bool DriverSocket::isValid() const
{
    return m_fd != -1;
}

void DriverSocket::close()
{
    ::close(m_fd);
    m_fd = -1;
}

uint32_t DriverSocket::getErrorCount() const{
    return err_counter;
}
