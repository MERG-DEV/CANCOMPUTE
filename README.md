# CANCOMPUTE
Event processing module. Just CBUS, no other hardware IO

This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material
    The licensor cannot revoke these freedoms as long as you follow the license terms.
    Attribution : You must give appropriate credit, provide a link to the license,
                   and indicate if changes were made. You may do so in any reasonable manner,
                   but not in any way that suggests the licensor endorses you or your use.
    NonCommercial : You may not use the material for commercial purposes. **(see note below)
    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                  your contributions under the same license as the original.
    No additional restrictions : You may not apply legal terms or technological measures that
                                  legally restrict others from doing anything the license permits.
   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

The actual CBUSlib files needed by the project are:
  * CBUSlib header files
    - callbacks.h
    - can18.h
    - cbus.h
    - cbuslib.h
    - devincs.h
    - EEPROM.h
    - events.h
    - FliM.h
    - MoreTypes.h
    - romops.h
    - StatusLeds.h
    - TickTime.h
  * CBUSlib source files
    - Bootloader.asm
    - c018.c
    - can18.c
    - cbus.c
    - events.c
    - FliM.c
    - romops.c
    - StatusLeds.c
    - ticktime.c
  * CBUSdefs header files
    - cbusdefs.h

You will also need to ensure you are using the correct linker script from this repo depending upon target processor type. 
