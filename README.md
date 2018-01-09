<p align="center">
  <img src="/assets/logo.png" />
</p>

# Just give me a copy!
* Windows: https://www.cen64.com/uploads/stable/cen64-windows-x86_64.exe
* Linux: https://www.cen64.com/uploads/stable/cen64-debian9-x86_64
* Mac: It works, but unfortunately you have to build it yourself for now.

Buildbot: https://github-buildbot.cen64.com/builders

# About

Yes, _another_ Nintendo 64 emulator. This one, however, aims for _perfect_
emulation by emulating the hardware inside of the Nintendo 64 itself, down
to the register-transfer level (RTL). At the same time, I've tried to keep
things as optimized as possible in hopes that CEN64 will someday run ROMs at
full speed, even on modest systems.

# Why?

CEN64 is my pet project. It's something I pick up whenever I get bored. To me,
what Nintendo and SGI did with this console is nothing short of amazing. The
ingenuity and design of the hardware was well-ahead of it's time, and it is
an absolute blast to reverse-engineer and study. I started this project in
order to learn more about what _really_ went on at the hardware level back in
the (good old) days.

Thank you to every single one of you developers for filling my childhood
with excellent memories. I'd also like to thank the community on all their
hard work and effort spent reverse-engineering this little gem. Without
further ado... "Get N or get out"!

# Development

If you want to contribute, please do! Pull requests are most certainly
welcome. Feel free to add yourself to the CONTRIBUTORS file as well.

# Keyboard controls

* 3D stick:       arrow keys
* A button:       X
* B button:       C
* Z button:       Z
* Start button:   enter
* L/R buttons:    A/S
* C-pad:          TFGH
* D-pad:          IJKL

# Build requirements

* iconv
* OpenAL
* OpenGL

# FAQ

* How do I run cen64?
You will need a valid pifdata.bin file (NTSC or PAL), & a ROM in .z64 format (in big-endian format).

To run cen64 without multithreading (slower):
cen64 pifdata.bin ROM.z64

To run cen64 with multithreading (faster):
cen64 -multithread pifdata.bin ROM.z64

* How do I run 64DD games?
You will need a valid 64ddipl.bin file (NTSC JPN or USA), & a 64DD disk image file in .ndd format.

cen64 -ddipl 64ddipl.bin -ddrom DISK.ndd pifdata.bin

* How do I setup save files for games?
N64 has various types of save formats used in games...
You will need to specify the save type yourself, cen64 will create the file if it does not exist.

EEP4K:
cen64 -eep4k eep4k.bin pifdata.bin ROM.z64

EEP16K:
cen64 -eep16k eep16k.bin pifdata.bin ROM.z64

Flash:
cen64 -flash flash.bin pifdata.bin ROM.z64

SRAM:
cen64 -sram sram.bin pifdata.bin ROM.z64

* The game runs, but I get strange errors, like multiple ocarinas in Majoras Mask.
You can fix the issue by using the -flash option on the command line:
cen64 -flash flash.bin pifdata.bin majora.z64
