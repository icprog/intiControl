/*
 *
 * This file is part of intiLED.
 *
 * intiLED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intiLED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include <string.h>
#include <usb.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "descriptors.h"
#include "debug.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include <datetime.h>

#define TIMER_COUNT 0xD1

Debug dbg;

// Success indication globals (used to keep the success LED on so you can see it)
int16_t successIndicatorCounter = 0;
uint8_t successIndicatorFlag = false;

// Failure indication globals (used to keep the failure LED on so you can see it)
int16_t failureIndicatorCounter = 0;
uint8_t failureIndicatorFlag = false;

// Globals for bulk packet reception
int16_t bulkRxExpectedPackets = 0;
int16_t bulkRxCurrentPacket = 0;
uint8_t bulkRxModeFlag = false;
uint8_t bulkRxCommandNumber;

// Global for forming debug output strings using sprintf
char debugString[24];

// Event handler for the USB_ControlRequest event
void EVENT_USB_Device_ControlRequest(void)
{
    // Handle HID Class specific requests

    // Note: For all tests we expect to receive a 64 byte packet containing
    // the command in byte[0] and then the numbers 0-62 in bytes 1-63.
    uint8_t bufferPointer;
    uint8_t expectedData;
    uint8_t dataReceivedOk;

    int16_t bulkTxPacketCounter = 0;

    // This is called if the host is waiting for a report from the device.  This functionality is not used
    // in the generic HID framework but is kept for compatibility with LUFA (bulk sending and receiving is
    // an atomic operation dealt with when the initial command packet is received below).
    if (USB_ControlRequest.bRequest == HID_REQ_GetReport)
    {
        if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
        {
            uint8_t GenericData[GENERIC_REPORT_SIZE];
            //CreateGenericHIDReport(GenericData);

            Endpoint_ClearSETUP();

            sprintf(debugString, "Error: USB_ControlRequest.bRequest == HID_REQ_GetReport?");
            dbg.output(debugString);

            /* Write the report data to the control endpoint */
            //Endpoint_Write_Control_Stream_LE(&GenericData, sizeof(GenericData));
            Endpoint_ClearOUT();
        }
    }

    // Do we have a command request from the host?
    if (USB_ControlRequest.bRequest == HID_REQ_SetReport)
    {
        // Ensure this is the type of report we are interested in
        if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
        {
            // Note: the endpoints are from the perspective of the host: OUT is 'out from host' and IN is 'in to host'

            // Declare our send and receive buffers
            char hidReceiveBuffer[GENERIC_REPORT_SIZE];
            char hidSendBuffer[GENERIC_REPORT_SIZE];

            // Clear the SETUP endpoint
            Endpoint_ClearSETUP();

            // Read the report data from the control endpoint
            Endpoint_Read_Control_Stream_LE(&hidReceiveBuffer, sizeof(hidReceiveBuffer));
            Endpoint_ClearIN();

            // If we are not in Bulk Rx mode interpret the packet as a command
            if (bulkRxModeFlag == false)
            {
                // Process GenericHID command packet
                switch(hidReceiveBuffer[0])
                {
                    case 0x10:	// Debug information request from host
                        // Select the IN end-point
                        Endpoint_SelectEndpoint(GENERIC_IN_EPNUM);

                        // Send a response to the host
                        if (Endpoint_IsINReady())
                        {
                            // Copy any waiting debug text to the send data buffer
                            dbg.sendBuffer((char*)&hidSendBuffer[0]);

                            // Write the return packet data into the report
                            Endpoint_Write_Stream_LE(&hidSendBuffer, sizeof(hidSendBuffer), NULL);

                            // Finalize the stream transfer to send the last packet
                            Endpoint_ClearOUT();
                        }
                        break;

                    // Place application specific commands here:

                    case 0x80:  // Test 1 - Single packet write from host to device
                        sprintf(debugString, "** Received command 0x80 from host");
                        dbg.output(debugString);

                        // Test the received data
                        expectedData = 0;
                        dataReceivedOk = true;
                        for (bufferPointer = 1; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != expectedData)
                                dataReceivedOk = false;

                            expectedData++;
                        }

                        // Display the test result
                        if (dataReceivedOk == true)
                        {
                            successIndicatorFlag = true;
                            sprintf(debugString, "Successfully received 1 packet of data from command 0x80");
                            dbg.output(debugString);
                        }
                        else
                        {
                            failureIndicatorFlag = true;
                            sprintf(debugString, "Failed to receive 1 packet of data from command 0x80");
                            dbg.output(debugString);
                        }
                        break;

                    case 0x81:	// Test 2 - Single packet write from host, single packet reply from device
                        sprintf(debugString, "** Received command 0x81 from host");
                        dbg.output(debugString);

                        // Test the received data
                        expectedData = 0;
                        dataReceivedOk = true;
                        for (bufferPointer = 1; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != expectedData)
                                dataReceivedOk = false;

                            expectedData++;
                        }

                        // If we got the data correctly, send the response packet
                        if (dataReceivedOk == true)
                        {
                            sprintf(debugString, "Successfully received 1 packet of data from command 0x81");
                            dbg.output(debugString);

                            // Select the IN end-point
                            Endpoint_SelectEndpoint(GENERIC_IN_EPNUM);

                            // Send a response to the host
                            if (Endpoint_IsINReady())
                            {
                                // Create return packet data
                                expectedData = 0;
                                for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                                {
                                    hidSendBuffer[bufferPointer] = expectedData;
                                    expectedData++;
                                }

                                // Write the return packet data into the report
                                Endpoint_Write_Stream_LE(&hidSendBuffer, sizeof(hidSendBuffer), NULL);

                                // Finalize the stream transfer to send the last packet
                                Endpoint_ClearOUT();
                            }

                            sprintf(debugString, "Sent 1 packet of data to the host from command 0x81");
                            dbg.output(debugString);

                            // Show our success
                            successIndicatorFlag = true;
                        }
                        else
                        {
                            failureIndicatorFlag = true;

                            sprintf(debugString, "Failed to receive 1 packet of data from command 0x81");
                            dbg.output(debugString);
                        }
                        break;

                    case 0x82:	// Test 3 - Single packet write from host, 128 packets in reply from device
                        sprintf(debugString, "** Received command 0x82 from host");
                        dbg.output(debugString);

                        // Test the received data
                        expectedData = 0;
                        dataReceivedOk = true;
                        for (bufferPointer = 1; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != expectedData)
                                dataReceivedOk = false;

                            expectedData++;
                        }

                        // If the receive was OK, go into bulk sending mode
                        if (dataReceivedOk == true)
                        {
                            sprintf(debugString, "Successfully received 1 packet of data from command 0x82, now bulk sending");
                            dbg.output(debugString);

                            // Bulk send 128 reply packets to the host (128x64 bytes = 8Kbytes response with 64 byte packet size)

                            // Select the IN end-point
                            Endpoint_SelectEndpoint(GENERIC_IN_EPNUM);

                            for (bulkTxPacketCounter = 0; bulkTxPacketCounter < 128; bulkTxPacketCounter++)
                            {
                                // Wait for the host IN endpoint to be ready
                                while(!Endpoint_IsINReady());

                                // Create return packet data
                                for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                                {
                                    hidSendBuffer[bufferPointer] = bulkTxPacketCounter;
                                }

                                // Write the return packet data into the report
                                Endpoint_Write_Stream_LE(&hidSendBuffer, sizeof(hidSendBuffer), NULL);

                                // Finalize the stream transfer to send the last packet
                                Endpoint_ClearOUT();
                            }

                            sprintf(debugString, "Bulk sending complete (128 packets sent)");
                            dbg.output(debugString);

                            // Show our success
                            successIndicatorFlag = true;
                        }
                        else
                        {
                            failureIndicatorFlag = true;

                            sprintf(debugString, "Failed to receive 1 packet of data from command 0x82");
                            dbg.output(debugString);
                        }
                        break;

                    case 0x83:	// Test 4 - 128 packets written from host, 1 packet in reply from device
                        sprintf(debugString, "** Received command 0x83 from host");
                        dbg.output(debugString);

                        // There is only 63 bytes of data in the first packet due to the command byte
                        dataReceivedOk = true;

                        // we are expecting 0 (the packet number) in bytes 1 to 63 (byte 0 is the command)
                        for (bufferPointer = 1; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != 0)
                                dataReceivedOk = false;
                        }

                        // Display the test result
                        if (dataReceivedOk == true)
                        {
                            successIndicatorFlag = true;
                            sprintf(debugString, "Successfully received 1 packet of data from command 0x83");
                            dbg.output(debugString);
                        }
                        else
                        {
                            failureIndicatorFlag = true;
                            sprintf(debugString, "Failed to receive 1 packet of data from command 0x83");
                            dbg.output(debugString);
                        }

                        // Set up the bulk Rx globals
                        bulkRxExpectedPackets = 128;
                        bulkRxCurrentPacket = 1;
                        bulkRxModeFlag = true;
                        bulkRxCommandNumber = 0x83;

                        break;

                    case 0x84:	// Test 5 - 128 packets written from host, 128 packets in reply from device
                        sprintf(debugString, "** Received command 0x84 from host");
                        dbg.output(debugString);

                        // There is only 63 bytes of data in the first packet due to the command byte
                        dataReceivedOk = true;

                        // we are expecting 0 (the packet number) in bytes 1 to 63 (byte 0 is the command)
                        for (bufferPointer = 1; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != 0)
                                dataReceivedOk = false;
                        }

                        // Display the test result
                        if (dataReceivedOk == true)
                        {
                            successIndicatorFlag = true;
                            sprintf(debugString, "Successfully received 1 packet of data from command 0x84");
                            dbg.output(debugString);
                        }
                        else
                        {
                            failureIndicatorFlag = true;
                            sprintf(debugString, "Failed to receive 1 packet of data from command 0x84");
                            dbg.output(debugString);
                        }

                        // Set up the bulk Rx globals
                        bulkRxExpectedPackets = 128;
                        bulkRxCurrentPacket = 1;
                        bulkRxModeFlag = true;
                        bulkRxCommandNumber = 0x84;
                        break;

                    default:
                        // Unknown command received
                        sprintf(debugString, "Unknown command received (%x)", hidReceiveBuffer[0]);
                        dbg.output(debugString);
                        break;
                } // switch(hidReceiveBuffer[0])
            }
            else
            {
                // We are in bulk receive mode - packet is data
                switch(bulkRxCommandNumber)
                {
                    case 0x83: // Test 4 (Bulk Rx) - 128 packets written from host, 1 packet in reply from device
                        // Test the received data
                        expectedData = bulkRxCurrentPacket;
                        dataReceivedOk = true;
                        for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != expectedData)
                            {
                                dataReceivedOk = false;
                                sprintf(debugString, "Bulk Rx 0x83 expected %d, got %d (bufferPointer = %d)", expectedData, hidReceiveBuffer[bufferPointer], bufferPointer);
                                dbg.output(debugString);

                                // Quit the for() loop
                                break;
                            }
                        }

                        if (dataReceivedOk == true)
                        {
                            bulkRxCurrentPacket++;

                            // Is the bulk receive complete?
                            if (bulkRxCurrentPacket == bulkRxExpectedPackets)
                            {
                                // Leave bulk Rx mode
                                bulkRxModeFlag = false;
                                bulkRxCommandNumber = 0;

                                sprintf(debugString, "Bulk Rx to command 0x83 successful - %d packets received", bulkRxCurrentPacket);
                                dbg.output(debugString);

                                // Send 1 packet to the host in reply to the command

                                // Select the IN end-point
                                Endpoint_SelectEndpoint(GENERIC_IN_EPNUM);

                                // Send a response to the host
                                if (Endpoint_IsINReady())
                                {
                                    // Create return packet data
                                    for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                                    {
                                        hidSendBuffer[bufferPointer] = bufferPointer;
                                    }

                                    // Write the return packet data into the report
                                    Endpoint_Write_Stream_LE(&hidSendBuffer, sizeof(hidSendBuffer), NULL);

                                    // Finalize the stream transfer to send the last packet
                                    Endpoint_ClearOUT();
                                }

                                sprintf(debugString, "Sent 1 packet of data to the host from command 0x83");
                                dbg.output(debugString);

                                // Show our success
                                successIndicatorFlag = true;
                            }
                        }
                        else
                        {
                            // Received data was wrong, give up
                            failureIndicatorFlag = true;
                            sprintf(debugString, "Failed to bulk Rx data from command 0x83 (current packet = %d)", bulkRxCurrentPacket);
                            dbg.output(debugString);

                            // Leave bulk Rx mode
                            bulkRxModeFlag = false;
                            bulkRxCommandNumber = 0;
                        }
                        break;

                        case 0x84: // Test 5 (Bulk Rx) - 128 packets written from host, 128 packets in reply from device
                        // Test the received data
                        expectedData = bulkRxCurrentPacket;
                        dataReceivedOk = true;
                        for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                        {
                            if (hidReceiveBuffer[bufferPointer] != expectedData)
                            {
                                dataReceivedOk = false;
                                sprintf(debugString, "Bulk Rx 0x84 expected %d, got %d (bufferPointer = %d)", expectedData, hidReceiveBuffer[bufferPointer], bufferPointer);
                                dbg.output(debugString);

                                // Quit the for() loop
                                break;
                            }
                        }

                        if (dataReceivedOk == true)
                        {
                            bulkRxCurrentPacket++;

                            // Is the bulk receive complete?
                            if (bulkRxCurrentPacket == bulkRxExpectedPackets)
                            {
                                // Leave bulk Rx mode
                                bulkRxModeFlag = false;
                                bulkRxCommandNumber = 0;

                                sprintf(debugString, "Bulk Rx to command 0x84 successful - %d packets received", bulkRxCurrentPacket);
                                dbg.output(debugString);

                                // Send 128 packets to the host in reply to the command

                                // Select the IN end-point
                                Endpoint_SelectEndpoint(GENERIC_IN_EPNUM);

                                for (bulkTxPacketCounter = 0; bulkTxPacketCounter < 128; bulkTxPacketCounter++)
                                {
                                    // Wait for the host IN endpoint to be ready
                                    while(!Endpoint_IsINReady());

                                    // Create return packet data
                                    for (bufferPointer = 0; bufferPointer < GENERIC_REPORT_SIZE; bufferPointer++)
                                    {
                                        hidSendBuffer[bufferPointer] = bulkTxPacketCounter;
                                    }

                                    // Write the return packet data into the report
                                    Endpoint_Write_Stream_LE(&hidSendBuffer, sizeof(hidSendBuffer), NULL);

                                    // Finalize the stream transfer to send the last packet
                                    Endpoint_ClearOUT();
                                }

                                sprintf(debugString, "Bulk sending complete (128 packets sent) 0x84");
                                dbg.output(debugString);

                                // Show our success
                                successIndicatorFlag = true;
                            }
                        }
                        else
                        {
                            // Received data was wrong, give up
                            failureIndicatorFlag = true;
                            sprintf(debugString, "Failed to bulk Rx data from command 0x84 (current packet = %d)", bulkRxCurrentPacket);
                            dbg.output(debugString);

                            // Leave bulk Rx mode
                            bulkRxModeFlag = false;
                            bulkRxCommandNumber = 0;
                        }
                        break;

                    default:
                        // Unknown bulk receive command!
                        sprintf(debugString, "Unknown bulk receive command!");
                        dbg.output(debugString);
                        break;
                }
            }
        }
    }
}


// Event handler for the USB_ConfigurationChanged event
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    // Setup HID Report Endpoints
    ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPNUM, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
        ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPNUM, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
}

ISR(TIMER0_OVF_vect)
{
    USB_USBTask();

    TCNT0 = TIMER_COUNT;
};

Usb::Usb()
{
    // setup a local ISR
    TCCR0A  = 0x00;           // normal count up timer operation
    TCCR0B  = 0x05;           // div-1024 = 7812.5Hz clock
    TCNT0   = TIMER_COUNT;    // Interrupt every 10ms
    TIMSK0 |= (1 << TOIE0);   // enable interrupt

    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    GlobalInterruptEnable();
}
