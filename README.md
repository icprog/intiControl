**INCOMPLETE**
This project is still a work in progress. The following describes both implemented and unimplemented features.

# intiControl
High power LED reef aquarium control project.

This firmware runs on custom hardware based on an Atmel ATMEGA32u2 controller. The controller is interfaced with a PCA9685 PWM IC, which is in turn connected to LED drivers.

This allows dimming control using 16bit resolution.

The lights can be set to any 3 modes;

1. Manual
2. Semi-Automatic
3. Full-Automatic

In all modes, the maximum intensity of all channels is user settable.

## Manual Mode
In this mode the lights are manually controlled by the user. The only interface to the lights is currently USB. An LCD and Rotary encoder form part of the hardware, but the user interface was just too cumbersome to use.

An advantage of this is that the controller can be accessed using any USB host (including remote *central* DIY controllers) for easier system integration. Control messages can be sent using simple python scripts sitting on the back of a CRON job, or fully fledged C++ application code. Making this manual mode very versatile. 

Details of expected messaging schemes can be found in messages.h.

## Semi-Automatic
In this mode the lights are automatically controlled. All required parameters are setup using the USB host software (still to be written).

The user preselects a sunrise and sunset times, and the controller ensures these times are adhered to every day.

## Fully-Automatic
In this mode the lights are automatically controlled as per Sime-Automatic mode, with the addition of location based improvements. 

This ensures that the derivation of sunset, sunrise and moonphases is based on given location. This way it is possible to setup your reef to mimic locations such as the barrier reef.

# USB
The LUFA USB stack is used, configured as a Generic HID device to ensure minimal driver issues. Your PC should already contain all required drivers.

## Control Application
At the moment there is no such application, but it is in the process of being written.
