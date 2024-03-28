#include "JBCComm.h"
#include "CP210x.h"
#include <usbhub.h>

#include "pgmstrings.h"
#include <SPI.h>

// Set to 0 for no debugging, 1 for regular, 2 for trace.
#define JBC_DEBUGPRINT 0

uint8_t CP210xAsync::OnInit(CP210x *cp210x)
{
    uint8_t rcode = 0;
    rcode = cp210x->IFCEnable();
    if (rcode)
    {
        return rcode;
    }

    rcode = cp210x->SetBaudRate(500000);
    if (rcode)
    {
        return rcode;
    }

    rcode = cp210x->SetDataBits(8);
    if (rcode)
    {
        return rcode;
    }

    rcode = cp210x->SetStopBits(CP210X_STOP_BITS_1);
    if (rcode)
    {
        return rcode;
    }

    rcode = cp210x->SetParity(CP210X_PARITY_EVEN);
    if (rcode)
    {
        return rcode;
    }

    rcode = cp210x->SetFlowControl(CP210X_FLOW_CONTROL_OFF);
    if (rcode)
    {
        return rcode;
    }

    return rcode;
}

#define JBC_POLl_LOOP_WAIT_TIME 250

JBCComm::JBCComm() :
    commState(CommState::Comm_Start),
    lastCommand(CommState::Comm_Start),
    commWaiting(false),
    cp210x(CP210x(&Usb, &cp210xAsync))
    {

}

void JBCComm::RegisterToolstatusUpdatedCallback(ToolStatusUpdatedEvent callback) {
    toolStatusUpdatedCallback = callback;
}

void JBCComm::Init() {
    if (Usb.Init() == -1) {
        Serial.println("USB did not start.");
    }
    // Initialize processing timeout
    commState = Comm_Start;
    lastStateChange = millis();
    commWaiting = false;
}


void JBCComm::SendBlock(uint8_t * data, uint8_t length) {
    uint8_t rcode;
    rcode = cp210x.SndData(length, data);

    if (rcode) {
       ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
    }

    delay(50);
}

#define ADDCRC(c) crc = crc ^ (c);
#define ADDBUFF(c) { if ((c)==DLE) {txbuf[fill++] = DLE;ADDCRC(DLE);} txbuf[fill++] = (c); ADDCRC(c);}

void JBCComm::SendMessage(JBCCommand command, uint8_t * data, uint8_t length) {
    uint8_t fill = 0;
    uint8_t crc = 0;    
    txbuf[0] = DLE;
    txbuf[1] = STX;
    ADDCRC(STX);
    fill = 2;
    // Origin
    ADDBUFF(0x01)
    // Destination
    ADDBUFF(0x00)
    ADDBUFF(command)
    ADDBUFF(length)
    for (uint8_t i = 0; i < length; i++) {
        ADDBUFF(data[i])
    }
    ADDCRC(0)
    ADDCRC(ETX)
    uint8_t msgcrc = crc;
    ADDBUFF(crc);
    txbuf[fill++] = DLE;
    txbuf[fill++] = ETX;
    SendBlock(txbuf, fill);
}



// Try to receive data from USB and put it in rxbuf. rxCount contains the number of bytes received.
// Returns the returncode from RcvData. 0 if success.
uint8_t JBCComm::tryRx() {
    memset(rxbuf, 0, sizeof(rxbuf));

    rxCount = 64;
    uint8_t rcode = cp210x.RcvData(&rxCount, rxbuf);    
    #if JBC_DEBUGPRINT
    Serial.print("tryRx rxCount: " );
    Serial.print(rxCount, DEC);
    Serial.print(" rcode: ");
    Serial.println(rcode, HEX);
    #endif
}

void JBCComm::PrintToolStatus() {
#if JBC_DEBUGPRINT
    Serial.print("Tool status: Sleep");
    Serial.print(toolStatus.sleep ? " yes" : " no ");
    Serial.print("Hiber");
    Serial.print(toolStatus.hibernation ? " yes " : " no ");
    Serial.print("extractor");
    Serial.print(toolStatus.extractor ? " yes " : " no ");
    Serial.print("desold");
    Serial.print(toolStatus.desold ? " yes " : " no ");
    Serial.print("stand");
    Serial.print(toolStatus.stand ? " yes " : " no ");
    Serial.print("timeToSleep ");
    Serial.print(toolStatus.timeToSleepHibern);
    Serial.print(" futuremode ");
    Serial.print(toolStatus.toolFutureMode);
    Serial.println("");
#endif
}
void JBCComm::UpdateToolStatus(uint8_t status) {
    // I'm not sure if I agree with the naming of the, but this is how things are named in the JBC Library.
    // In my case, the desold field tells me if the tool is in the holder or not. And it has nothing to do with sleep times, they are always 0.
    // An alternative may be to use message 48 ReadInfoPort, which also provides information about the tool type.
    this->toolStatus.sleep =  ((status & 0x01) == 0x01);
    this->toolStatus.hibernation = ((status & 0x02) == 0x02);
    this->toolStatus.extractor = ((status & 0x04) == 0x04);
    this->toolStatus.desold = ((status & 0x08) == 0x08);
    if (this->toolStatus.timeToSleepHibern > 0 && this->toolStatus.toolFutureMode == ToolFutureMode::Sleep){
        // Not yet sleeping
        this->toolStatus.sleep = false;
        this->toolStatus.stand = true;
    } else {
        this->toolStatus.stand = false;
    }
    PrintToolStatus();
    if (this->toolStatusUpdatedCallback != NULL) {
        toolStatusUpdatedCallback();
    }
}

void JBCComm::UpdateToolDelaytime(uint16_t timeToSleepHibern, ToolFutureMode toolFutureMode) {
    this->toolStatus.toolFutureMode=toolFutureMode;
    this->toolStatus.timeToSleepHibern = timeToSleepHibern;
    if (this->toolStatus.timeToSleepHibern > 0 && this->toolStatus.toolFutureMode == ToolFutureMode::Sleep){
        // Not yet sleeping
        this->toolStatus.sleep = false;
        this->toolStatus.stand = true;
    } else {
        this->toolStatus.stand = false;
    }
    PrintToolStatus();
    if (toolStatusUpdatedCallback != NULL) {
        toolStatusUpdatedCallback();
    }
}

// Most useful message info is in ReceiveFrame01_SOLD.cs for incoming and SendFrame01_SOLD.cs for outgoing frames
// PhysicalChannelUsb can be modified to print sent and received data.
void JBCComm::RxPacket(uint8_t length) {
    // there is a packet in dedodedFrame, with lenght number of bytes
#if JBC_DEBUGPRINT
    Serial.print("Frame:");
    for (int i = 0; i < length; i++) {
            Serial.print(decodedFrame[i], HEX);
            Serial.print(" ");
    }
    Serial.println(" ");
#endif
    uint8_t crc = 0;
    crc = crc ^ STX;
    crc = crc ^ ETX;
    for (int i = 0; i < length; i++) {
        crc = crc ^ decodedFrame[i];
    }

    if (crc != 0) {
        // CRC mismatch
#if JBC_DEBUGPRINT
        Serial.println("CRC mismatch");
#endif
        return;
    }
    FrameData * data = (FrameData *)decodedFrame;
#if JBC_DEBUGPRINT
    Serial.print("Command: ");
    Serial.println(data->command, HEX);
#endif
    switch (data->command) {
    case M_R_STATUSTOOL:
        if (data->length == 1) {
            UpdateToolStatus(data->payload[0]);
        }
        break;
    case M_R_DELAYTIME:
        if (data->length == 3) {
            uint16_t time = data->payload[1] << 8 | data->payload[0];
            UpdateToolDelaytime(time, (ToolFutureMode)data->payload[2]);
        }
        break;
    }

}

void JBCComm::DecodeRx() {
    if (rxCount == 0) {
        return;
    }

    if (rxbuf[0] == M_NACK) {
        // Got a NACK. Restart handshake.
        commState = Comm_Start;
        return;
    }
#if JBC_DEBUGPRINT > 1
    Serial.print("RX:");
    for (int i = 0; i < rxCount; i++) {
            Serial.print(rxbuf[i], HEX);
            Serial.print(" ");
    }
    Serial.println(" ");
#endif
    DecodeState ds = WaitDLE;
    uint8_t decodepos = 0;
    for (int i = 0; i < rxCount; i++) {
        switch(ds){
            // First DLE
            case WaitDLE:
                if (rxbuf[i] == DLE) {
                    ds = WaitSTX;
                }
                break;
            // STX after the first DLE
            case WaitSTX:
                if (rxbuf[i] == STX) {
                    ds = Data; 
                    decodepos = 0;
                } else if (rxbuf[i] == DLE) {
                    // stay
                } else {
                    ds = WaitDLE;
                }
                break;
            // Regular data that could contain a DLE-DLE sequence
            case Data:
                if (rxbuf[i] == DLE) {
                    ds = DataEscape;
                } else {
                    decodedFrame[decodepos++] = rxbuf[i];
                }
                break;
            // Got a DLE in regular data, see what is next and act on that
            case DataEscape:
                if (rxbuf[i] == DLE) {
                    // Regular case: DLE-DLE that should be decoded to DLE
                    ds = Data;
                    decodedFrame[decodepos++] = rxbuf[i];
                } else if (rxbuf[i] == ETX) {
                    // End of frame, check and process frame. Reset decodepos.
                    ds = WaitDLE;
                    RxPacket(decodepos);
                } else if (rxbuf[i] == STX) {
                    // Unexpected DLE-STX. Restart packet.
                    decodepos = 0;
                    ds = Data;
                } else {
                    // DLE-anything else is an error, restart at the beginning.
                    ds = WaitDLE;
                }            
                break;
        }
    }


}


/** Returns 0 if success, anything else is an error and requires reinitalization. */
uint8_t JBCComm::Process() {
    uint8_t rxresult = 0;
    uint8_t error = 0;
    Usb.Task();
    uint8_t taskState = Usb.getUsbTaskState();
    if ( taskState != USB_STATE_RUNNING ) {
        if (taskState == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
            Serial.println("No device found");
            return 1;
        }
        #if JBC_DEBUGPRINT
        Serial.print("taskState: ");
        Serial.println(taskState, HEX);
        #endif
        return 0;
    }
    CommState prevCommState = commState;
    // handle sync an data readout
    switch(commState) { 
        // Handshake states
        case Comm_Start:
            rxresult = tryRx();
            if ((rxCount > 0) && rxbuf[0] == M_NACK) {
                txbuf[0] = M_SYN;
                SendBlock(txbuf, 1);
                commState = Comm_SynAck;
            }
            break;
        case Comm_SynAck:
            rxresult = tryRx();
            if ((rxCount > 0) && rxbuf[0] == M_ACK) {
                txbuf[0] = M_ACK;
                SendBlock(txbuf, 1);
                commState = Comm_AckAck;
            }
            break;
        case Comm_AckAck:
            rxresult = tryRx();
            if ((rxCount > 0) && rxbuf[0] == SOH) {
                txbuf[0] = M_ACK;
                SendBlock(txbuf, 1);
                // Entering protocol state
                commState = Comm_Protostart;
            }
            break;
        case Comm_Protostart:
            // Pick next command based on previous
            switch (lastCommand) {
                // Regular loop:
                case Comm_Toolstatus:   commState = Comm_Delaytime; break;
                case Comm_Delaytime:    commState = Comm_Wait; break;
                case Comm_Wait:         commState = Comm_Toolstatus; break;
                // Start/failure cases:
                case Comm_Start:        commState = Comm_Toolstatus; break;
                default:                commState = Comm_Toolstatus; break;
            }
            lastCommand = commState;
            break;
        case Comm_Toolstatus: {
            // Parameter: port
            uint8_t tooldat[] = {1};
            SendMessage(M_R_STATUSTOOL,tooldat, sizeof(tooldat));
            commState = Comm_Response;
            break;
        }
        case Comm_Delaytime: {
            // Parameter: port
            uint8_t tooldat[] = {1};
            SendMessage(M_R_DELAYTIME,tooldat, sizeof(tooldat));
            commState = Comm_Response;
            break;
        }       
        case Comm_Response:
            // Get packet(s) and decode if we receive something.
            rxresult = tryRx();
            if (rxCount > 0) {
                DecodeRx();
            } else {
                // No response, station off?
                error = 1;
            }
            
            commState = Comm_Protostart;
            break;
        case Comm_Wait:
            if (commWaiting) {
                if (millis() - startWait > JBC_POLl_LOOP_WAIT_TIME) {
                    // done waiting
                    commState = Comm_Protostart;
                    commWaiting = false;
                }
            } else {
                commWaiting = true;
                startWait = millis();
            }
            break;
    
    }

    // Statemachine timeout. None of the steps should take long, so if we hit the timeout, we missed something.
    if (millis() - lastStateChange > 1000) {
#if JBC_DEBUGPRINT
        Serial.println("Response timeout, resetting state");
#endif
        commState = Comm_Start;
        lastStateChange = millis();
    }
    if (commState != prevCommState) {
        lastStateChange = millis();
#if JBC_DEBUGPRINT
        Serial.print("commState: ");
        Serial.print(prevCommState, DEC);
        Serial.print(" -> ");
        Serial.print(commState, DEC);
        Serial.print(" rxresult: ");
        Serial.print(rxresult);
        Serial.print(" rxcount: ");
        Serial.println(rxCount);
#endif
    }    
    return error;
}