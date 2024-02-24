# JBC CD-2BQF Fume extraction switch

# Disclaimer
Use this at your own risk. This is an engineering example, not a detailed instruction, not a product. This is not associated to or endorsed by JBC in any way.

# History
I had a Weller soldering station with Weller WFE-P fume extractor that extracts the solder fumes at the tip of the soldering Iron.
It has Stop-and-Go stand that controls a Weller KH-E device that switches the WEF-P on or off depending on wether the iron is in the stand or in use.

I bought a JBC CD-2BQF to replace my Weller soldering station. It surprised me that JBC does offer a tip extraction accesory but not a fume extraction unit to go with that. Therefore the I decided to keep the Weller Fume extraction setup.

The next question was, how do I get the JBC station to control the fume extraction? Weller's Stop-and-Go stand contains an LED that lights up when the tool is not in the stand. The stands sends that via a light conducting fiber to the KH-E and the KH-E switches the extractor on when it sees the light.

The JBC has an RJ-12 port that talks to a fume extractor like the FAE1. First tought was that it would be a simple on/off switch. But upon investigation, this was an RS-232 port that talked to the fume extractor. With no access to the fume extractor, no help from JBC and no documentation available about the protocol, figuring out how to respond to the soldering station would be difficult.

## Collecting information
JBC has a software utility to update firmware of their stations. This utility is a .NET tool, written in C#. After installing, it puts a number of DLLs in C:\Program Files\JBC\JBC Stations Updater. JBC_Connect.dll is the interesting one. It contains a complete API for talking to JBC soldering equipment. You can use that to write your own C# application to play with all data the station has. Also, being a C# project, it is very easy to decompile, for example using JetBrains dotPeek. Once you do that it spills its guts.

Another thing I did was open the JBC soldering station. Next to the USB port is the CP210x USB-to-Serial converter, that has two traces to an opto isolator of some kind. I used the old Weller to add wires to that so I could monitor the communication between the PC and the station. This way I could be sure that no information was missed.

# The protocol
The CP210x that handles the USB communication is a standard USB to Serial converter. The protocol runs at 500000 baud, 8 bits, **even parity** 1 stopbit.

In the base state, the station sends NAK (0x15) bytes. The PC then sends a SYN (0x16). Station responds with ACK (0x06). PC sends ACK, station responds with SOH (0x01) and the PC acknowledges this with an ACK.

From this moment on we are in the protocol state, unless it times out. Then the station returns to sending NAKs.

In the protocol state messages look like this:

```
<DLE> <STX> <source> <destination> <command> <length> [length * data bytes] <crc> <DLE> <ETX>
```

If the data contains a DLE byte, it is escaped with DLE.
The checksum is an XOR of all bytes of the data, not including the DLEs.

For example, a Get Device UID message looks like this:

```
10 02 01 00 B9 00 B9 10 03

Decoded:
    10 DLE
    02 STX
    01 From: PC
    00 To: Station
    B9 Get device UID command
    00 0 bytes payload
    B9 CRC (02 ^ 01 ^ 00 ^ B9 ^ 00 ^ 03)
    10 DLE
    03 ETX
```

The station then responds with a message in similar format that contains the response.

# This code
This code contains the basics for talking to the station over the USB interface. To run the code, use an Arduino (e.g. Arduino Uno) with the USB Host Shield 2.0.

Install the library for the USB Host Shield, compile the sketch and run it.

The soldering station connects to the port of the USB host shield.

When you pick the soldering iron up from the stand, the builtin LED of the Arduino will light up and when you put it back, the LED will be off. Adapt this to your own needs. In my case, the LED will be used to start the KH-E or maybe I will add a relay to completely replace the KH-E.

A small delay was added to this process because sometimes the station reports the iron not in the holder when it is just sitting in the holder. Also there is a delay that keeps the fume extraction on for a little longer after you put the soldering iron back to ensure that it catches the last fumes as well.