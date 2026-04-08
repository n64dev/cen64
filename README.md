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

To build for Windows on Windows 10 (1809+) and Windows 11, do:

1. Install [MSYS2](https://www.msys2.org/).  After installation, a terminal with the UCRT64 environment will open.
* If this window is closed, it can be reopened by either accessing the start menu and clicking on `MSYS2/MSYS2 [env name]`, or by navigating to MSYS2's installation path (the default installation path is `C:\msys2`) and double clicking on the preferred environment (`ucrt64.exe` or `clang64.exe`).
2. Install a compiler. It is recommended to use either UCRT64 or CLANG64 environments. See [this page](https://www.msys2.org/docs/environments) for details and differences between MSYS2 environments.
* If you're using a UCRT64 terminal, type `pacman -S mingw-w64-ucrt-x86_64-gcc` and press enter.
* If you're using a CLANG64 terminal, type `pacman -S mingw-w64-clang-x86_64-clang` and press enter.
3. Install git, clone the CEN64 repo and move into the new directory:
* `pacman -S git`
* `git clone https://github.com/n64dev/cen64.git`
* `cd cen64`
  * The CEN64 repo will be in `[MSYS2 install dir]\home\[your username]\cen64`.
4. Install dependencies (choose either ucrt or clang, depending on which environment you have selected):
* Make dependencies: `mingw-w64-[ucrt/clang]-x86_64-{cmake,ninja}`
* Dependencies: `mingw-w64-[ucrt/clang]-x86_64-{iconv,openal}`
5. Create a build folder, move into it, then run cmake and ninja to configure and compile CEN64:
* `mkdir build && cd build`
* `cmake .. -DCMAKE_BUILD_TYPE=Release`
* `ninja` (For faster compilation, `ninja -j[number of threads]`)
  * A CEN64 executable should be created after compilation has finished.
6. Find all shared library dependencies, and copy each of them to the same folder cen64.exe is in. This step is needed so that CEN64 can be run outside of an MSYS2 environment.
* `ldd cen64.exe | grep [ucrt64|clang64]` (choose the environment you compiled cen64.exe for)
  * If `command not found` appears for either grep or ldd, `pacman -S grep ldd` 
* For each .dll listed, `cp [/path/to/dependency.dll] .`
7. Copy cen64.exe and each dll to a new folder of your choice. By default, the compiled CEN64 executable will be located in `C:\msys64\home\[username]\cen64\build`.
8. Open a command prompt window (Win+R, cmd.exe) and navigate to the new folder that contains CEN64.
9. Run CEN64.

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
