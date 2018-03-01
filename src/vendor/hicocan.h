/*
 * Copyright emtrion GmbH (+49 721 627250, support@emtrion.de). All
 * rights reserved.
 *
 * $Id: hicocan.h 858 2007-09-21 12:53:02Z ny $
 * Author: Martin Nylund
 */
#ifndef _HICOCAN_H
#define _HICOCAN_H

//#if !defined (__KERNEL__)
//#include <stdint.h>
//#endif

#ifdef __QNX__
 #define IOC_MAGIC _DCMD_MISC
 #include <ioctl.h>
 #include <devctl.h>
 #include <stdint.h>
#else /* Linux */
// #define IOC_MAGIC 'E'
 #ifndef __KERNEL__
  #include <sys/ioctl.h>
  #include <stdint.h>
 #endif
#endif

/* definitions for frame format    */
#define HiCOCAN_FORMAT_BASIC    0
#define HiCOCAN_FORMAT_EXTENDED 1

/* definition for remote frame     */
#define HiCOCAN_REMOTE_FRAME    1
#define HiCOCAN_NORMAL_FRAME    0

/* Acceptance filter mode. FILTERMODE_SINGLE is a 'normal' filtering mode.  In
 * FILTERMODE_DUAL - the two first databytes of a CAN telegram are also used
 * in filtering (for standard frame messages). See datasheet for SJA1000 for
 * more information on the dual mode. We recommend that you only use just the
 * single mode. */
#define HiCOCAN_FILTERMODE_DUAL 0
#define HiCOCAN_FILTERMODE_SINGLE       1

/* Add a filter to the list of basic (11bit) identifiers */
#define HiCOCAN_FW_FILTER_ADD_BASIC     2
/* Add a filter to the list of extended (29bit) identifiers */
#define HiCOCAN_FW_FILTER_ADD_EXT       3
/* Clear all IDs in the filter tables - basic and extended */
#define HiCOCAN_FW_FILTER_CLR           4 



/* definitions for the use of the default baud rates */
#define HiCOCAN_BAUD10K         1
#define HiCOCAN_BAUD20K         2
#define HiCOCAN_BAUD50K         5
#define HiCOCAN_BAUD100K        10
#define HiCOCAN_BAUD125K        12
#define HiCOCAN_BAUD250K        25
#define HiCOCAN_BAUD500K        50
#define HiCOCAN_BAUD800K        80
#define HiCOCAN_BAUD1M          100


/* Bit values in CAN state variable in the canState structure. Only few of
 * these have a meaning for applications */
#define C_RECBUFSTA  0x01
#define C_ERR_OVERR  0x02  /* Overrun flag */
#define C_TRABUFSTA  0x04
#define C_TRACMPL    0x08
#define C_RECSTA     0x10
#define C_TRASTA     0x20
#define C_ERR_PSV    0x40  /* CAN node in error passive state */
#define C_BUSOFF     0x80  /* CAN node in BusOff state */


// ioctl calls
#define IOC_MAGIC 'H'


/* Start listening CAN traffic. Messages that pass the acceptance masking are
 * saved into the drivers receive buffer. Transmit enabled. */
#define IOC_START               _IO     (IOC_MAGIC, 1)

/* Stop listening CAN traffic and disable transmit */
#define IOC_STOP                _IO     (IOC_MAGIC, 2)

/* Reset the board and the firmware and reset the transmit and receive
 * buffers. Note that this command affects both of the CAN nodes on the board
 * */
#define IOC_RESET_BOARD         _IO     (IOC_MAGIC, 3)

/* Reads the current CAN parameters from the board. Takes a pointer to a
 * sCanParameter structure where the parameters are to be written. */
#define IOC_GET_CAN_PARAM       _IOR    (IOC_MAGIC, 4, struct canParam)

/* Reads the current status information of the CAN node. Takes a pointer to a
 * canState structure where the status information is to be written */
#define IOC_GET_CAN_STATE       _IOR    (IOC_MAGIC, 5, struct canState)

/* Sets the timing registers btro and btr1 on the SJA1000 can controller */
#define IOC_SET_TIMING_REGS     _IOW    (IOC_MAGIC, 6, struct canParam)

/* Set  the CAN nodes baud rate (valid values are listed above HiCOCAN_BAUD*)
 * */
#define IOC_SET_BAUD            _IOW    (IOC_MAGIC, 7, struct canParam)

/* Set the acceptance mask and code */
#define IOC_SET_ACCEPTANCE      _IOW    (IOC_MAGIC, 8, struct canParam)

/* Set the timestamp counter on the HiCOCAN board */
#define IOC_SET_TIMESTAMP       _IOW    (IOC_MAGIC,     9, struct canTs)

/* Clear the overrun flag on the board */
#define IOC_CLEAR_OVERRUN       _IO     (IOC_MAGIC, 10)

/* Set the CAN node in passive mode. CAN traffic is listened and received it
 * is however not affected in any way */
#define IOC_PASSIVE         _IO     (IOC_MAGIC, 11)

/* Get the CAN line error status (Fault Tolerant CAN only) */
#define IOC_GET_LINE_ERR        _IOR    (IOC_MAGIC, 12, uint8_t)

/* Enable and disable the firmware message filtering */
#define IOC_EN_FILTER        _IO     (IOC_MAGIC, 13)
#define IOC_DIS_FILTER       _IO     (IOC_MAGIC, 14)

/* Following ioctl calls are not part of the normal API */
#define IOC_DIRECT_MODE_ON      _IO     (IOC_MAGIC,20)
#define IOC_DIRECT_MODE_OFF     _IO     (IOC_MAGIC, 21)
#define IOC_CFG_READ            _IOR    (IOC_MAGIC, 22, struct hicocan_cfg)

/* Get firmware version. Use this command only before configuring and starting the
 * CAN node ! */
#define IOC_FWID_READ           _IOR    (IOC_MAGIC, 23, uint8_t[21])
#define IOC_GET_BOARD_STATE     _IOR    (IOC_MAGIC, 24, uint8_t)
#define IOC_ABORT_TRANSMIT      _IO     (IOC_MAGIC, 25)
#define IOC_EN_AUTO_OVERR       _IO     (IOC_MAGIC, 26)
#define IOC_DIS_AUTO_OVERR      _IO     (IOC_MAGIC, 27)


/* CAN parameters. NOTE, that when using this structure with the above IOCTL
 * calls, only members related to the given IOCTL call are used (i.e.
 * IOC_SET_ACCEPTANCE only uses the accCode and accMask members and ignores
 * rest */
struct canParam {
    uint8_t btr0;               /* bus timing register 0                     */
    uint8_t btr1;               /* bus timing register 1                     */
    uint8_t baud;               /* CAN baudrate / 10                         */
    uint8_t accFm;              /* filter mode (see HiCOCAN_FILTERMODE_*)    */
    uint32_t accCode;           /* acceptance code                           */
    uint32_t accMask;           /* acceptance mask                           */
};

struct canState {
    uint8_t state;              /* state register FW                         */
    uint8_t rxErr;              /* receive error counter                     */
    uint8_t txErr;              /* transmit error counter                    */
    uint16_t recBuf;            /* Number of CAN messages in driver buffer   */
    uint16_t traQ;              /* Number of CAN messages in boards Tx Queue */
    uint16_t recQ;              /* Number of CAN messages in boards Rx Queue */
};

/* structure of a timestamp */
struct canTs {
    uint16_t day;
    uint8_t hour;
    uint8_t min;
    uint16_t sec;
    uint16_t ms;
    uint16_t us;
};

/* structure of a CAN messages */
struct canMsg {

    /* Frame format (normal or extended) */
    uint8_t ff;

    /* Remote transmission request */
    uint8_t rtr;

    /* Data length */
    uint8_t dlc;

    /* CAN Id (arbitration field */
    uint32_t id;

    uint8_t data[8];

    /* Time stamp (not used when sending) */
    struct canTs ts;
};

#endif /* _HICOCAN_H */
