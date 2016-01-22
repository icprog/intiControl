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
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>

#include "descriptors.h"
#include "Config/AppConfig.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

uint8_t * buffer;
USB_ClassInfo_HID_Device_t * interface;
bool * usbAttached;

//USB_ClassInfo_HID_Device_t Generic_HID_Interface;

void EVENT_USB_Device_Connect(void)
{
    *usbAttached = true;
}

void EVENT_USB_Device_Disconnect(void)
{
    *usbAttached = false;
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(interface);

    USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void)
{
    HID_Device_ProcessControlRequest(interface);
}

void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(interface);
}

uint8_t hack[4];
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    uint8_t * Data = (uint8_t*)ReportData;

    Data[0] = hack[0];
    Data[1] = hack[1];
    Data[2] = hack[2];
    Data[3] = hack[3];

    *ReportSize = GENERIC_REPORT_SIZE;

    return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    uint8_t* Data       = (uint8_t*)ReportData;
    hack[0] = Data[0];
    hack[1] = Data[1];
    hack[2] = Data[2];
    hack[3] = Data[3];
}


Usb::Usb()
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    m_interface.Config.InterfaceNumber          = INTERFACE_ID_GenericHID;
    m_interface.Config.ReportINEndpoint.Address = GENERIC_IN_EPADDR;
    m_interface.Config.ReportINEndpoint.Size    = GENERIC_EPSIZE;
    m_interface.Config.ReportINEndpoint.Banks   = 1;
    m_interface.Config.PrevReportINBuffer       = m_buffer;
    m_interface.Config.PrevReportINBufferSize   = sizeof(m_buffer);

    interface    = &m_interface;
    usbAttached  = &m_attached;
    buffer       = m_buffer;
}

bool Usb::tick()
{
    HID_Device_USBTask(interface);
    USB_USBTask();

    return usbAttached;
}

bool Usb::attached()
{
    return m_attached;
}
