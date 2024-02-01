Recreation of Psion Organiser 2 using RP Pico
=============================================

This is a repository for the code of a recreation of the Psion Organsier 2. The recreation uses a RP Pico on a PCB that replaces the main
PCB of the organiser 2. This runs an emulator of the 6303 and organiser ROMs. The PCB is mated with a second PCB (the 'power' PSB) which has the slots 
that allow datapacks to be read (not written with the first version). It also holds a wifi module that should allow communication with a host over
wifi or maybe bluetooth.

The PCBs fit into the original organiser 2 case.

EPROM datapack programming
==========================

The original power PCB does not have a VPP supply as I didn't want to support programming of the EPROM datapacks. To do that would require the emulated code to run at a speed that exactly matched that of the original organiser, at least during the time that the programming pulses are being generated. Because of this I subsequently found that some datapacks have a regulator that takes the 21V VPP signal and turns it into a 5V signal. This is used, it appears, for the control of the page register on 128K EPROM datapacks. (At least the 27011 based packs). So the first pair of PCBs will not support the 128K EPROM datapacks. They will also not support any other datapacks that have a regulator on board, which includes flash datapacks.

Datapacks that Use Vpp
======================

There are datapacks that use Vpp such as 128K EPROM packs and flash datapacks. These will also not work with the re-creation.

Speed
=====

The processor emulation code now runs at about the same speed as the original Organiser. When accessing hardware the emulation speed can drop as the Pico has to emulate the hardware as well as the main processor.

PCB Versions
============

V1.0 PCBS
---------

The errata for these PCBs shows that there are many problems with these PCBs. It is possible to build them up and modify them to match the V1.1 PCBs as far as possible, but it's quite a job. mechanically these PCBs fit, but there are some mounting holes missing.

V1.1 PCBs
---------

These PCBs fix the errata of the V1.0 PCBs. They have more of the mounting holes and the pin headers conneting the two PCBs are better aligned.
Check the errata for problems with these PCBs.

Other Repositories
==================

PCB artwork is in other repositories. The 6303 emulatior is also in another repository.

Unsupported Hardware List
========================
128K EPROM datapacks. No reading or writing.
Flash datapacks. Not tested, but probably don't work for the same reason as the 128K EPROM datapacks.

ROM Versions
============

The emulator should be able to run any Organiser II ROM. There are two distinct hardware types, the XP and the LZ/LZ64. To run a ROM of either type, use the
embed_rom script with an appropriate ROM file selected within the script. The MODEL_AT_START needs to be set up in the file 'psion_recreate.h'. Rebuild the code and the ROm should run.


