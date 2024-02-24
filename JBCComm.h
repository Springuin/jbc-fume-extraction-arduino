#ifndef _JBCCOMM_H_
#define _JBCCOMM_H_

#include <stdint.h>
#include "CP210x.h"
#include <usbhub.h>
class CP210xAsync : public CP210xAsyncOper {
  public:
    uint8_t OnInit(CP210x *cp210x);
};

class JBCComm {

public:
    const uint8_t DLE = 0x10;
    const uint8_t SOH = 0x01;
    const uint8_t STX = 0x02;
    const uint8_t ETX = 0x03;

    enum JBCCommand {
        M_HS = 0,
        M_EOT = 4,
        M_ACK = 6,
        M_NACK = 21, // 0x15
        M_SYN = 22, // 0x16
        M_R_DEVICEIDORIGINAL = 28, // 0x1C
        M_R_DISCOVER = 29, // 0x1D
        M_R_DEVICEID = 30, // 0x1E
        M_W_DEVICEID = 31, // 0x1F
        M_RESET = 32, // 0x20
        M_FIRMWARE = 33, // 0x21
        M_CLEARMEMFLASH = 34, // 0x22
        M_SENDMEMADDRESS = 35, // 0x23
        M_SENDMEMDATA = 36, // 0x24
        M_ENDPROGR = 37, // 0x25
        M_ENDUPD = 38, // 0x26
        M_CONTINUEUPD = 39, // 0x27
        M_CLEARING = 40, // 0x28
        M_INF_PORT = 48, // 0x30
        M_RESET_PORTTOOL = 49, // 0x31
        M_R_LEVELSTEMPS = 51, // 0x33
        M_W_LEVELSTEMPS = 52, // 0x34
        M_R_PROFILEMODE = 53, // 0x35
        M_W_PROFILEMODE = 54, // 0x36
        M_R_SLEEPDELAY = 64, // 0x40
        M_W_SLEEPDELAY = 65, // 0x41
        M_R_SLEEPTEMP = 66, // 0x42
        M_W_SLEEPTEMP = 67, // 0x43
        M_R_HIBERDELAY = 68, // 0x44
        M_W_HIBERDELAY = 69, // 0x45
        M_R_AJUSTTEMP = 70, // 0x46
        M_W_AJUSTTEMP = 71, // 0x47
        M_R_SELECTTEMP = 80, // 0x50
        M_W_SELECTTEMP = 81, // 0x51
        M_R_TIPTEMP = 82, // 0x52
        M_R_CURRENT = 83, // 0x53
        M_R_POWER = 84, // 0x54
        M_R_CONNECTTOOL = 85, // 0x55
        M_R_TOOLERROR = 86, // 0x56
        M_R_STATUSTOOL = 87, // 0x57
        M_W_SELECTTEMPVOLATILE = 88, // 0x58
        // There is an error here, enum does not match with code. But command 89 is about reading delay temperatures.
        //M_R_MOSTEMP = 89, // 0x59
        //M_R_DELAYTIME = 90, // 0x5A
        M_R_DELAYTIME = 89, // 0x5A
        M_R_REMOTEMODE = 96, // 0x60
        M_W_REMOTEMODE = 97, // 0x61
        M_R_CONTIMODE = 128, // 0x80
        M_W_CONTIMODE = 129, // 0x81
        M_I_CONTIMODE = 130, // 0x82
        M_R_ALARMMAXTEMP = 131, // 0x83
        M_W_ALARMMAXTEMP = 132, // 0x84
        M_R_ALARMMINTEMP = 133, // 0x85
        M_W_ALARMMINTEMP = 134, // 0x86
        M_R_ALARMTEMP = 135, // 0x87
        M_R_LOCK_PORT = 136, // 0x88
        M_W_LOCK_PORT = 137, // 0x89
        M_R_ASSISTANT_CONFIG = 138, // 0x8A
        M_W_ASSISTANT_CONFIG = 139, // 0x8B
        M_R_SOLDERING_RESULT = 140, // 0x8C
        M_READSTARTFILE = 144, // 0x90
        M_READFILEBLOCK = 145, // 0x91
        M_READENDOFFILE = 146, // 0x92
        M_WRITESTARTFILE = 147, // 0x93
        M_WRITEFILEBLOCK = 148, // 0x94
        M_WRITEENDOFFILE = 149, // 0x95
        M_R_FILESCOUNT = 150, // 0x96
        M_R_GETFILENAME = 151, // 0x97
        M_DELETEFILE = 152, // 0x98
        M_R_SELECTEDFILENAME = 154, // 0x9A
        M_W_SELECTEDFILENAME = 155, // 0x9B
        M_R_QST_ACTIVATE = 156, // 0x9C
        M_W_QST_ACTIVATE = 157, // 0x9D
        M_R_QST_STATUS = 158, // 0x9E
        M_W_QST_STATUS = 159, // 0x9F
        M_R_MAXTEMP = 162, // 0xA2
        M_W_MAXTEMP = 163, // 0xA3
        M_R_MINTEMP = 164, // 0xA4
        M_W_MINTEMP = 165, // 0xA5
        M_R_PINENABLED = 168, // 0xA8
        M_W_PINENABLED = 169, // 0xA9
        M_R_POWERLIM = 170, // 0xAA
        M_W_POWERLIM = 171, // 0xAB
        M_R_PIN = 172, // 0xAC
        M_W_PIN = 173, // 0xAD
        M_R_STATERROR = 174, // 0xAE
        M_R_TRAFOTEMP = 175, // 0xAF
        M_RESETSTATION = 176, // 0xB0
        M_R_DEVICENAME = 177, // 0xB1
        M_W_DEVICENAME = 178, // 0xB2
        M_W_TYPEOFGROUND = 185, // 0xB9
        M_R_TYPEOFGROUND = 186, // 0xBA
        M_R_USB_CONNECTSTATUS = 224, // 0xE0
        M_W_USB_CONNECTSTATUS = 225, // 0xE1
        M_R_ETH_TCPIPCONFIG = 231, // 0xE7
        M_W_ETH_TCPIPCONFIG = 232, // 0xE8
        M_R_ETH_CONNECTSTATUS = 233, // 0xE9
        M_W_ETH_CONNECTSTATUS = 234, // 0xEA
        M_R_RBT_CONNCONFIG = 240, // 0xF0
        M_W_RBT_CONNCONFIG = 241, // 0xF1
        M_R_RBT_CONNECTSTATUS = 242, // 0xF2
        M_W_RBT_CONNECTSTATUS = 243, // 0xF3
        M_R_PERIPHCOUNT = 249, // 0xF9
        M_R_PERIPHCONFIG = 250, // 0xFA
        M_W_PERIPHCONFIG = 251, // 0xFB
        M_R_PERIPHSTATUS = 252, // 0xFC
        M_W_PERIPHSTATUS = 253, // 0xFD
    };

    enum ToolFutureMode{
        Hibernation = 72, // 0x48
        NoFutureMode = 78, // 0x4E
        Sleep = 83, // 0x53
    };

    enum ContinuousModeSpeed {
        OFF,
        T_10mS,
        T_20mS,
        T_50mS,
        T_100mS,
        T_200mS,
        T_500mS,
        T_1000mS,
    };

    typedef struct {
        bool extractor;
        bool hibernation;
        bool desold;
        bool sleep;
        bool stand;
        uint16_t timeToSleepHibern;
        ToolFutureMode toolFutureMode;
    } ToolStatus;
    // Contains all the data we collected about the first tool.
    ToolStatus toolStatus;
    JBCComm();
    void Init();
    void Process();
    void SendMessage(JBCCommand command, uint8_t * data, uint8_t length);
    
    using ToolStatusUpdatedEvent = void (*)(); 
    void RegisterToolstatusUpdatedCallback(ToolStatusUpdatedEvent callback);

private:
    enum DecodeState {
        WaitDLE,
        WaitSTX,
        Data,
        DataEscape,
    };
    struct FrameData {
        uint8_t source;
        uint8_t dest;
        // We can use the JBCCommand type here but then we need to compile with the short-enums compiler option.
        uint8_t command;
        uint8_t length;
        uint8_t payload[60];
    };
    enum CommState {
        Comm_Start,
        Comm_SynAck,
        Comm_AckAck,
        Comm_Protostart,
        Comm_Toolstatus,
        Comm_Delaytime,
        Comm_Response,
        Comm_Wait,
    };

    void UpdateToolStatus(uint8_t status);
    void PrintToolStatus();
    void UpdateToolDelaytime(uint16_t timeToSleepHibern, ToolFutureMode toolFutureMode);
    void RxPacket(uint8_t length);
    void DecodeRx();
    void tryRx();
    void SendBlock(uint8_t * data, uint8_t length);
    // Generic USB device
    USB Usb;
    // CP210x Init/Release callbacks. We hook into the init callback for setting the UART parameters
    CP210xAsync cp210xAsync;
    // CP210x object for communicating with the CP210x in the station
    CP210x cp210x;
    // Communication state
    CommState commState;
    // Last command we sent, used to determine next command to send
    CommState lastCommand;
    // Receive buffer for received frame (still encoded)
    uint8_t  rxbuf[64];
    // Fillcount for the rxbuf
    uint16_t rxCount;
    // Decoded version of the receive buffer
    uint8_t decodedFrame[64];
    // Transmitbuffer
    uint8_t txbuf[64];
    
    ToolStatusUpdatedEvent toolStatusUpdatedCallback;
    // Waiting flag and timer for the commState state machine
    bool commWaiting;
    uint32_t startWait;
    // Timer for the state-freeze detection
    uint32_t lastStateChange;
};

#endif