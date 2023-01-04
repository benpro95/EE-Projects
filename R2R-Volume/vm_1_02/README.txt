@(#)README.txt	1.21 16/12/19
Volu-Master(tm) firmware for LCDuino-1

GENERAL INFORMATION
-------------------
This package is the Volu-Master firmware source code for LCDuino-1,
an Arduino-compatible microcontroller platform designed for advanced
audio system status display, control and monitoring applications.

LCDuino-1 and Volu-Master development team:
Bryan Levin - Sercona Audio (http://www.sercona.net)
Ti Kan	    - AMB Laboratories (http://www.amb.org)

Volu-Master source code is released under GNU General Public License
version 3.  By using this source code you agree to its terms.

    http://www.gnu.org/licenses/gpl.html

Instructions for compilation and flashing the firmware on
the LCDuino-1 are found in the INSTALL.txt file.

Additional instructions and documentation for LCDuino-1 and
Volu-Master can be found at the official LCDuino-1, delta1
and delta2 websites:

    http://www.amb.org/audio/lcduino1/	Display I/O processor
    http://www.amb.org/audio/delta1/	Relay-based attenuator
    http://www.amb.org/audio/delta2/	Relay-based I/O selector

The alpha10 pre-amplifier is a reference implementation of this system:

    http://www.amb.org/audio/alpha10/

For user support and discussions, please visit the AMB DIY audio forum:

    http://www.amb.org/forum/

Printed circuit boards and related parts for the LCDuino-1 system,
as well as Atmel MCU chips pre-flashed with bootloader and
Volu-Master firmware are available at AMB audio shop:

    http://www.amb.org/shop/

RELEASE NOTES
-------------
Volu-Master firmware v1.02:
1. AMB now recommends using Arduino 1.6.13.  The Makefile-1.6 file has
   been updated and tested to support all 1.6.x versions up to 1.6.13.

Volu-Master firmware v1.01:
1. You may compile this version with Arduino version 1.0.x, 1.5.x or
   1.6.x.  If you use the Makefile method to compile your firmware
   (rather than the Arduino IDE), then use Makefile-1.0, Makefile-1.5
   or Makefile-1.6, respectively.  See the INSTALL.txt file for
   details.
2. Compiling with Arduino version 00xx is no longer supported.
3. The Arduino IDE may display this message:
   "Low memory available, stability problems may occur."
   You can ignore it.
4. On Windows, you may encounter the following error while compiling
   with Arduino 1.6.x due to an apparent linker bug:
   "collect2.exe: error: ld returned 5 exit status"
   The solution is to either use Arduino-1.5.8, or replace the
   arduino-1.6.x\hardware\tools\avr\avr\bin\ld.exe with the one
   from Arduino-1.0.6.

Volu-Master firmware v1.00
1. Initial release.
2. Must be compiled with Arduino 0022 or 0023, using the Arduino
   IDE or Makefile.

ACKNOWLEDGEMENTS
----------------
- Infrared remote control support code:
    + Based on Arduino IR Library by Ken Shirriff
      http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
    + Interrupt handling code based on NECIRrcv by Joe Knapp
      http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
    + Also influenced by this article by Mark Ivey:
      http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
- X10 support code based on work by BroHogan
  http://brohogan.blogspot.com/
- RTC chip driver and LCD driver initial code based on work by Tom Lackamp
  http://www.mindspring.com/~tom2000/Projects/AI-1_Remote/AI-1_Remote.html
- Big fonts based on work by dcb
  http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213319639

