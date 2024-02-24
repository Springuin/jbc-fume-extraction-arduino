/* Copyright (C) 2015 Henrik Larsson
This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Henrik Larsson
Email: laselase (a) gmail.com
*/
#if !defined(__CP210x_H__)
#define __CP210x_H__

#include "Usb.h"

#define CP210x_MAX_ENDPOINTS              3

#define REQTYPE_HOST_TO_INTERFACE   (uint8_t) 0x41
#define REQTYPE_INTERFACE_TO_HOST   (uint8_t) 0xc1

// Commands
#define CP210X_SET_LINE_CTL         (uint8_t) 0x03
#define CP210x_GET_LINE_CTL         (uint8_t) 0x04
#define CP210X_SET_FLOW             (uint8_t) 0x13
#define CP210X_IFC_ENABLE           (uint8_t) 0x00
#define CP210X_SET_BAUDRATE         (uint8_t) 0x1E
#define CP210X_GET_BAUDRATE         (uint8_t) 0x1D
#define CP210X_SET_CHARS            (uint8_t) 0x19

#define CP210X_PARITY_NONE          (uint8_t) 0x00
#define CP210X_PARITY_ODD           (uint8_t) 0x01
#define CP210X_PARITY_EVEN          (uint8_t) 0x02
#define CP210X_PARITY_MARK          (uint8_t) 0x03
#define CP210X_PARITY_SPACE         (uint8_t) 0x04

#define CP210X_STOP_BITS_1          (uint8_t) 0x01
#define CP210X_STOP_BITS_15         (uint8_t) 0x03
#define CP210X_STOP_BITS_2          (uint8_t) 0x02

#define CP210X_FLOW_CONTROL_OFF       0x00
#define CP210X_FLOW_CONTROL_RTS_CTS   0x01
#define CP210X_FLOW_CONTROL_DSR_DTR   0x02
#define CP210X_FLOW_CONTROL_XON_XOFF  0x03


class CP210x;

class CP210xAsyncOper {
public:

        virtual uint8_t OnInit(CP210x *pftdi) {
                return 0;
        };

        virtual uint8_t OnRelease(CP210x *pftdi) {
                return 0;
        };
};


class CP210x : public USBDeviceConfig, public UsbConfigXtracter {
        static const uint8_t epDataInIndex; // DataIn endpoint index
        static const uint8_t epDataOutIndex; // DataOUT endpoint index
        static const uint8_t epInterruptInIndex; // InterruptIN  endpoint index

        CP210xAsyncOper *pAsync;
        USB *pUsb;
        uint8_t bAddress;
        uint8_t bConfNum; // configuration number
        uint8_t bNumIface; // number of interfaces in the configuration
        uint8_t bNumEP; // total number of EP in the configuration
        uint32_t qNextPollTime; // next poll time
        bool bPollEnable; // poll enable flag

        EpInfo epInfo[CP210x_MAX_ENDPOINTS];

        void PrintEndpointDescriptor(const USB_ENDPOINT_DESCRIPTOR* ep_ptr);

public:
        CP210x(USB *pusb, CP210xAsyncOper *pasync);

        uint8_t GetLineCTL(uint8_t data[]);

        uint8_t IFCEnable();
        uint8_t SetBaudRate(uint32_t baud);
        uint8_t GetBaudRate(uint8_t data[]);
        uint8_t SetDataBits(uint8_t dataBits);
        uint8_t SetStopBits(uint8_t stopBits);
        uint8_t SetParity(uint8_t parity);
        uint8_t SetModemControl(uint16_t control);
        uint8_t SetFlowControl(uint8_t flowControl);
     
        // Methods for recieving and sending data
        uint8_t RcvData(uint16_t *bytes_rcvd, uint8_t *dataptr);
        uint8_t SndData(uint16_t nbytes, uint8_t *dataptr);

        // USBDeviceConfig implementation
        uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);
        uint8_t Release();
        uint8_t Poll();

        virtual uint8_t GetAddress() {
                return bAddress;
        };

        // UsbConfigXtracter implementation
        void EndpointXtract(uint8_t conf, uint8_t iface, uint8_t alt, uint8_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);

        virtual bool VIDPIDOK(uint16_t vid, uint16_t pid) {
                // Accepts all PID/VID.
                return true;
        }

};

#endif // __CP210x_H__
