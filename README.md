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

* 3D stick:       arrow keys (hold shift to "walk")
* A button:       X
* B button:       C
* Z button:       Z
* Start button:   enter
* L/R buttons:    A/S
* C-pad:          TFGH
* D-pad:          IJKL

# Build requirements

* CMake
* iconv
* OpenAL
* OpenGL

To build on Fedora 29, do: `sudo dnf install cmake make mesa-libGL-devel openal-soft-devel`

To build for Windows on Fedora 29, do: `sudo dnf install cmake make mingw64-{gcc,iconv,openal}`

To build for Windows on Windows XP..10, do:
1. Install MSYS 1.0.11: https://sourceforge.net/projects/mingw/files/MSYS/Base/msys-core/msys-1.0.11/MSYS-1.0.11.exe/download
* Say yes to post install
* Say no to mingw is installed
* Press enter a few times to finish the install
2. Extract contents of "mingw64" directory to "MSYS\mingw" directory: https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z
3. Copy "MSYS\mingw\bin\mingw32-make.exe" to "MSYS\mingw\bin\make.exe"
4. Extract OpenAL "bin, libs, include" directories to "MSYS\mingw\x86_64-w64-mingw32\": https://kcat.strangesoft.net/openal-binaries/openal-soft-1.19.1-bin.zip
5. Extract iconv to "MSYS\home\yourname\libiconv" directory: https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz
6. Run MSYS & type "cd libiconv" Enter, followed by "./configure --disable-shared" Enter, followed by "make install" Enter
7. Extract CMake "bin, doc, man, share" directories to "MSYS\": https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-win64-x64.zip
8. Extract CEN64 source to "MSYS\home\yourname\cen64" directory: https://github.com/n64dev/cen64
9. Run MSYS & type "cd cen64" Enter, followed by "cmake-gui" Enter
10. Add the "MSYS/home/yourname/cen64" directory to "Browse Source..." & "Browse Build...", then Generate MSYS compatible files:
* Add OpenAL "MSYS/mingw/x86_64-w64-mingw32/include/AL" as "OPENAL/OPENAL_INCLUDE_DIR" & "MSYS/mingw/x86_64-w64-mingw32/lib/libOpenAL32.dll.a" as "OPENAL/OPENAL_LIBRARY" (if they are not found)
* Make sure to use libiconv static lib (Not DLL) "MSYS/local/lib/libiconv.a" as "ICONV/ICONV_LIBRARIES"
* Select "SSE4.1" or "AVX" as "Ungrouped Entries/CEN64_ARCH_SUPPORT" & make sure VR4300_BUSY_WAIT_DETECTION is ticked
11. Click "Configure" then click "Generate" to get the makefiles, then quit cmake-gui, & type "make" Enter

# Usage

* How do I run cen64?<br />
You will need a valid pifdata.bin file (NTSC or PAL), & a ROM in .z64 format (in big-endian format).<br />

To run cen64 without multithreading (slower):<br />
cen64 pifdata.bin ROM.z64<br />
<br />
To run cen64 with multithreading (faster):<br />
cen64 -multithread pifdata.bin ROM.z64<br />

* How do I run 64DD games?<br />
You will need a valid 64ddipl.bin file (NTSC JPN or USA), & a 64DD disk image file in .ndd format.<br />

cen64 -ddipl 64ddipl.bin -ddrom DISK.ndd pifdata.bin<br />

* How do I setup save files for games?<br />
N64 has various types of save formats used in games...<br />
You will need to specify the save type yourself, cen64 will create the file if it does not exist.<br />

EEP4K:<br />
cen64 -eep4k eep4k.bin pifdata.bin ROM.z64<br />
<br />
EEP16K:<br />
cen64 -eep16k eep16k.bin pifdata.bin ROM.z64<br />
<br />
FLASH:<br />
cen64 -flash flash.bin pifdata.bin ROM.z64<br />
<br />
SRAM:<br />
cen64 -sram sram.bin pifdata.bin ROM.z64<br />

* The game runs, but I get strange errors, like multiple ocarinas in Majora's Mask.<br />

You can fix the issue by using the -flash option on the command line:<br />
cen64 -flash flash.bin pifdata.bin majora.z64
