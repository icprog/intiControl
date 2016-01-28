/*
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

#include <usb.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#define TIMER_COUNT 0x4E

/** Function to process the last received report from the host.
 *
 *  \param[in] DataArray  Pointer to a buffer where the last received report has been stored
 */
void ProcessGenericHIDReport(const uint8_t* DataArray)
{
    /*
        This is where you need to process reports sent from the host to the device. This
        function is called each time the host has sent a new report. DataArray is an array
        holding the report sent from the host.
    */
}

/** Function to create the next report to send back to the host at the next reporting interval.
 *
 *  \param[out] DataArray  Pointer to a buffer where the next report data should be stored
 */
void CreateGenericHIDReport(uint8_t* DataArray)
{
    /*
        This is where you need to create reports to be sent to the host from the device. This
        function is called each time the host is ready to accept a new report. DataArray is
        an array to hold the report to the host.
    */
}
#if 0
/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
    /* Indicate USB enumerating */
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management task.
 */
void EVENT_USB_Device_Disconnect(void)
{
    /* Indicate USB not ready */

}
#endif
/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the generic HID device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    /* Setup HID Report Endpoints */
    ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);

    /* Indicate endpoint configuration success or failure */

}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
    uint8_t GenericData[GENERIC_REPORT_SIZE];

    /* Handle HID Class specific requests */
    switch (USB_ControlRequest.bRequest)
    {
    case HID_REQ_GetReport:
        if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
        {
            CreateGenericHIDReport(GenericData);

            Endpoint_ClearSETUP();

            /* Write the report data to the control endpoint */
            Endpoint_Write_Control_Stream_LE(&GenericData, sizeof(GenericData));
            Endpoint_ClearOUT();
        }

        break;
    case HID_REQ_SetReport:
        if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
        {
            Endpoint_ClearSETUP();

            /* Read the report data from the control endpoint */
            Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
            Endpoint_ClearIN();

            ProcessGenericHIDReport(GenericData);
        }

        break;
    }
}

void HID_Task(void)
{
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);

    /* Check to see if a packet has been sent from the host */
    if (Endpoint_IsOUTReceived())
    {
        /* Check to see if the packet contains data */
        if (Endpoint_IsReadWriteAllowed())
        {
            /* Create a temporary buffer to hold the read in report from the host */
            uint8_t GenericData[GENERIC_REPORT_SIZE];

            /* Read Generic Report Data */
            Endpoint_Read_Stream_LE(&GenericData, sizeof(GenericData), NULL);

            /* Process Generic Report Data */
            ProcessGenericHIDReport(GenericData);
        }

        /* Finalize the stream transfer to send the last packet */
        Endpoint_ClearOUT();
    }

#if 0
    Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

    /* Check to see if the host is ready to accept another packet */
    if (Endpoint_IsINReady())
    {
        /* Create a temporary buffer to hold the report to send to the host */
        uint8_t GenericData[GENERIC_REPORT_SIZE];

        /* Create Generic Report Data */
        CreateGenericHIDReport(GenericData);

        /* Write Generic Report Data */
        Endpoint_Write_Stream_LE(&GenericData, sizeof(GenericData), NULL);

        /* Finalize the stream transfer to send the last packet */
        Endpoint_ClearIN();
    }
#endif
}

ISR(TIMER0_OVF_vect)
{
    TCNT0 = TIMER_COUNT;

    USB_USBTask();
    //HID_Task();
}

Usb::Usb()
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    // setup a local ISR
    TCCR0A = 0x00;           // normal count up timer operation
    TCCR0B = 0x05;           // div-1024 = 7812.5Hz clock
    TCNT0  = TIMER_COUNT;    // Interrupt every 10ms
    TIMSK0 = 0x01;           // enable interrupt

    /* Hardware Initialization */
    USB_Init();

    GlobalInterruptEnable();
}

bool Usb::tick()
{
    if (USB_DeviceState != DEVICE_STATE_Configured)
    {
        // check for data
    }

    return false;
}

bool Usb::attached()
{
    return (USB_DeviceState != DEVICE_STATE_Configured);
}
const Message * Usb::read()
{
    Message * ret = 0;

    if (attached())
    {
        Endpoint_SelectEndpoint(GENERIC_OUT_EPADDR);

        /* Check to see if a packet has been sent from the host */
        if (Endpoint_IsOUTReceived())
        {
            /* Check to see if the packet contains data */
            if (Endpoint_IsReadWriteAllowed())
            {
                /* Create a temporary buffer to hold the read in report from the host */
                uint8_t GenericData[GENERIC_EPSIZE];

                /* Read Generic Report Data */
                Endpoint_Read_Stream_LE(&GenericData, sizeof(GenericData), NULL);

                /* Process Generic Report Data */
                ProcessGenericHIDReport(GenericData);

                ret = (Message*)GenericData;
            }

            /* Finalize the stream transfer to send the last packet */
            Endpoint_ClearOUT();
        }
    }

    return ret;
}
bool Usb::send(const Message & data)
{
    bool ret = false;

    if (attached())
    {
        Endpoint_SelectEndpoint(GENERIC_IN_EPADDR);

        if (Endpoint_IsINReady())
        {
            ret = (Endpoint_Write_Stream_LE(&data, data, NULL)
                   != ENDPOINT_RWSTREAM_NoError);

            // Finalize the stream transfer to send the last packet
            Endpoint_ClearIN();
        }
    }

    return ret;
}
