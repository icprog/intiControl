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

#include "descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

static uint8_t usbBuffer[Message::m_length];
static USB_ClassInfo_HID_Device_t usbInterface;

static bool usbAttached;
static bool usbData;

/** Function to process the last received report from the host.
 *
 *  \param[in] DataArray  Pointer to a buffer where the last received report has been stored
 */
void ProcessGenericHIDReport(uint8_t* DataArray)
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

void EVENT_USB_Device_Connect(void)
{
    usbAttached = true;
}

void EVENT_USB_Device_Disconnect(void)
{
    usbAttached = false;
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_IN_EPADDR,  EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(GENERIC_OUT_EPADDR, EP_TYPE_INTERRUPT, GENERIC_EPSIZE, 1);

    USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&usbInterface);
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
    /* Handle HID Class specific requests */
    switch (USB_ControlRequest.bRequest)
    {
    case HID_REQ_GetReport:
        if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
        {
            uint8_t GenericData[GENERIC_REPORT_SIZE];
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
            uint8_t GenericData[GENERIC_REPORT_SIZE];

            Endpoint_ClearSETUP();

            /* Read the report data from the control endpoint */
            Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
            Endpoint_ClearIN();

            ProcessGenericHIDReport(GenericData);
        }

        break;
    }
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    return true;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    usbData = true;
}

void HID_Task(void)
{
    /* Device must be connected and configured for the task to run */
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
}

Usb::Usb()
    : m_buffer(usbBuffer),
      m_interface(&usbInterface),
      m_attached(&usbAttached),
      m_data(&usbData)

{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    usbInterface.Config.InterfaceNumber          = INTERFACE_ID_GenericHID;
    usbInterface.Config.ReportINEndpoint.Address = GENERIC_IN_EPADDR;
    usbInterface.Config.ReportINEndpoint.Size    = GENERIC_EPSIZE;
    usbInterface.Config.ReportINEndpoint.Banks   = 1;
    usbInterface.Config.PrevReportINBuffer       = m_buffer;
    usbInterface.Config.PrevReportINBufferSize   = sizeof(m_buffer);
}

bool Usb::tick()
{
    HID_Task();
    USB_USBTask();

    return m_data;
}

bool Usb::attached()
{
    return m_attached;
}
const Message * Usb::read()
{
    return (Message*)usbBuffer;
}
bool Usb::send(void * data, uint16_t len)
{
    bool ret = false;

    return ret;
}
